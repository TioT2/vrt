#ifndef __vrt_memory_h_
#define __vrt_memory_h_

#include "vrt_def.h"

namespace vrt
{
  template <typename object_type, UINT32 BLOCK_SIZE = 16>
    class deferred_block_allocator
    {
      /* stored object */
      struct storage
      {
        object_type Data;
        storage *Prev = nullptr, *Next = nullptr;
      }; /* object */

      /* block of stored objects */
      struct block
      {
        storage Data[BLOCK_SIZE] {}; // data of block
        block *Next = nullptr;      // next block

        block( block *Next ) : Next(Next)
        {

        } /* block */
      }; /* block */

      block *FirstBlock = nullptr;                                                  // first block
      storage *FirstFree = nullptr, *FirstToDelete = nullptr, *FirstUsed = nullptr; // object stocks

      /* New block adding function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      VOID AddBlock( VOID )
      {
        block *Block = new block(FirstBlock);

        for (UINT32 i = 0; i < BLOCK_SIZE - 1; i++)
        {
          Block->Data[i].Next = Block->Data + i + 1;
          Block->Data[i + 1].Prev = Block->Data + i;
        }

        if (FirstUsed != nullptr)
        {
          Block->Data[BLOCK_SIZE - 1].Next = FirstFree;
          FirstFree->Prev = Block->Data + BLOCK_SIZE - 1;
        }

        FirstBlock = Block;
        FirstFree = Block->Data;
      } /* AddBlock */

    public:
      struct iterator
      {
        friend class deferred_block_allocator<object_type>;

        storage *Storage = nullptr;

        iterator( storage *stg ) : Storage(stg)
        {
        } /* iterator */
      public:

        iterator( const iterator &Other ) : Storage(Other.Storage)
        {

        } /* iterator */

        iterator & operator++( VOID )
        {
          Storage = Storage->Next;

          return *this;
        } /* operator++ */

        iterator & operator--( VOID )
        {
          Storage = Storage->Prev;

          return *this;
        } /* operator-- */

        object_type & operator*( VOID )
        {
          return Storage->Data;
        } /* operator* */

        object_type * operator->( VOID )
        {
          return &Storage->Data;
        } /* operator-> */

        BOOL operator==( const iterator &Other )
        {
          return Storage == Other.Storage;
        } /* operator== */

        BOOL operator!=( const iterator &Other )
        {
          return Storage != Other.Storage;
        } /* operator!= */
      }; /* iterator */

      iterator Begin( VOID )
      {
        return iterator(FirstUsed);
      } /* Begin */

      static constexpr iterator End( VOID )
      {
        return iterator(nullptr);
      } /* End */

      VOID Clear(  VOID )
      {
        block *Block = FirstBlock;

        // Delete all
        FlushFree();
        for (storage *stg = FirstUsed; stg != nullptr; stg = stg->Next)
          stg->Data.~object_type();

        while (Block != nullptr)
        {
          block *toDelete = Block;

          Block = Block->Next;

          /* REAL SHIT BELOW */
          delete reinterpret_cast<BYTE *>(toDelete);
        }

        FirstBlock = nullptr;
        FirstToDelete = nullptr;
        FirstUsed = nullptr;
        FirstFree = nullptr;
      } /* Clear */

      /* Destructor. */
      ~deferred_block_allocator( VOID )
      {
        Clear();
      } /* ~deferred_block_allocator */

      template <typename ...argument_types>
        object_type * Alloc( argument_types&& ...Arguments )
        {
          if (FirstFree == nullptr)
            AddBlock();

          storage *Memory = FirstFree;

          FirstFree = Memory->Next;
          if (FirstFree != nullptr)
          {
            FirstFree->Prev = nullptr;
            Memory->Next = nullptr;
          }

          // placememnt new
          new (&Memory->Data) object_type(std::forward<argument_types>(Arguments)...);

          Memory->Next = FirstUsed;
          if (FirstUsed != nullptr)
            FirstUsed->Prev = Memory;
          FirstUsed = Memory;

          return &Memory->Data;
        } /* Alloc */

      VOID Free( object_type *Data )
      {
        storage *stg = reinterpret_cast<storage *>(Data);

        if (stg == FirstUsed)
        {
          FirstUsed = stg->Next;
          if (FirstUsed != nullptr)
            FirstUsed->Prev = nullptr;
          stg->Next = nullptr;
        }
        else
        {
          stg->Prev->Next = stg->Next;
          stg->Prev = nullptr;

          if (stg->Next != nullptr)
            stg->Next->Prev = stg->Prev;
        }

        if (FirstToDelete != nullptr)
        {
          FirstToDelete->Prev = stg;
          stg->Next = FirstToDelete;
        }

        FirstToDelete = stg;
      } /* Free */

      VOID FlushFree( VOID )
      {
        if (FirstToDelete == nullptr)
          return;

        storage *last = nullptr;
        for (storage *first = FirstToDelete; first != nullptr; first = first->Next)
        {
          OnFree(first->Data);

          if (first->Next == nullptr)
            last = first;
        }

        last->Next = FirstFree;
        if (FirstFree != nullptr)
          FirstFree->Prev = last;
        FirstFree = FirstToDelete;
        FirstToDelete = nullptr;
      } /* FlushFree */

      virtual VOID OnFree( object_type &Object )
      {
        Object.~object_type();
      } /* OnFree */
    }; /* deferred_block_allocator */

  template <typename type>
    class resource;

  template <typename index_type>
    class manager_base
    {
      std::map<index_type, resource<index_type> *> DataMap;

      friend class resource<index_type>;

      VOID Suicide( resource<index_type> *Resource )
      {
        Delete(Resource);
        DataMap.erase(*Resource->_Index);
      } /* Suicide */
    public:

      BOOL Manage( const index_type &Index, resource<index_type> *Resource )
      {
        typename std::map<index_type, resource<index_type> *>::iterator Iter = DataMap.find(Index);

        if (Iter == DataMap.end())
        {
          Resource->_Manager = this;
          Iter = DataMap.insert({Index, Resource}).first;
          Resource->_Index = &Iter->first;

          return TRUE;
        }

        return FALSE;
      } /* Manage */

      /* Function that clears resource data of manager */
      virtual VOID Delete( resource<index_type> *Resource )
      {

      } /* Delete */
    }; /* manager_base */

  template <typename index_type>
    class resource
    {
      friend class manager_base<index_type>;
  
      manager_base<index_type> *_Manager = nullptr; // pointer to manager that manages this resource
      const index_type *_Index = nullptr;           // index of resource in manager pointer
      UINT32 _UseCount = 1;                         // count of usages of this resource
  
    public:
      UINT32 GetUseCount( VOID ) const
      {
        return _UseCount;
      } /* GetUseCount */
    
      VOID Grab( VOID )
      {
        _UseCount++;
      } /* Grab */
    
      VOID Release( VOID )
      {
        _UseCount--;
  
        if (_UseCount < 1)
          _Manager->Suicide(this);
      } /* Release */
  
      virtual ~resource( VOID )
      {

      } /* ~resource_base */
    }; /* resource */

  template <typename resource_type>
    class ptr
    {
      resource_type *Ptr = nullptr;
    public:

      inline ptr( ptr &Other )
      {
        Ptr = Other.Ptr;
        Ptr->Grab();
      } /* ptr */

      inline ptr( resource_type *Resource )
      {
        Ptr = Resource;
      } /* ptr */

      inline operator resource_type *( VOID )
      {
        return Ptr;
      } /* operator resource_type * */

      inline resource_type * operator->( VOID )
      {
        return Ptr;
      } /* operator-> */

      ~ptr( VOID )
      {
        Ptr->Release();
      } /* ~ptr */
    }; /* ptr */

  template <typename resource_type, typename index_type = UINT32, UINT32 ALLOCATOR_BLOCK_SIZE = 16>
    class manager : manager_base<index_type>, public deferred_block_allocator<resource_type, ALLOCATOR_BLOCK_SIZE>
    {
      VOID Delete( resource<index_type> *Resource ) override
      {
        resource_type *Res = static_cast<resource_type *>(Resource);
        deferred_block_allocator<resource_type, ALLOCATOR_BLOCK_SIZE>::Free(Res);
      } /* Delete */

    protected:
      template <typename ...argument_types>
        resource_type * CreateResource( const index_type &Index, argument_types ...Arguments )
        {
          resource_type *Storage = deferred_block_allocator<resource_type>::Alloc(std::forward<argument_types>(Arguments)...);
          manager_base<index_type>::Manage(Index, Storage);
          return Storage;
        } /* CreateResource */
    }; /* manager */

  template <typename resource_type>
    class manager<resource_type, UINT32, 16> : manager_base<UINT32>, public deferred_block_allocator<resource_type, 16>
    {
      VOID Delete( resource<UINT32> *Resource ) override
      {
        resource_type *Res = static_cast<resource_type *>(Resource);
        deferred_block_allocator<resource_type, 16>::Free(Res);
      } /* Delete */
    protected:
      UINT32 UniqueResourceIndex = 0;

      template <typename ...argument_types>
        resource_type * CreateResource( argument_types ...Arguments )
        {
          resource_type *Storage = deferred_block_allocator<resource_type>::Alloc(std::forward<argument_types>(Arguments)...);

          manager_base<UINT32>::Manage(UniqueResourceIndex, Storage);
          UniqueResourceIndex++;
          return Storage;
        } /* CreateResource */
    }; /* manager */
} /* namespace vrt */

#endif // !defined __vrt_memory_h_