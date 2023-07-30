#include "vrt.h"

std::string vrt::unit_register_table::MainId;

int main( int argc, char **argv )
{
  if constexpr (vrt::IS_DEBUG)
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

  vrt::system System;

  System.Run();
  return 0;
} /* main */