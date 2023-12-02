#include "src/hello.hxx"
#include <iostream>
#include <boost/predef.h>
int
main ()
{

#if DEBUG
    std::cout << "DEBUG" << std::endl;
#else
    std::cout << "RELEASE" << std::endl;
#endif
#if BOOST_COMP_GNUC
    std::cout << "BOOST_COMP_GNUC" << std::endl;
#endif
#if BOOST_COMP_CLANG
    std::cout << "BOOST_COMP_CLANG" << std::endl;
#endif
#if BOOST_LIB_C_GNU
    std::cout << "BOOST_LIB_C_GNU" << std::endl;
#endif
#if BOOST_LIB_STD_CXX
    std::cout << "BOOST_LIB_STD_CXX" << std::endl;
#endif
#if BOOST_LIB_STD_GNU
    std::cout << "BOOST_LIB_STD_GNU" << std::endl;
#endif
  return 0;
}