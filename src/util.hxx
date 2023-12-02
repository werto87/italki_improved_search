#ifndef EBD66723_6B6F_4460_A3DE_00AEB1E6D6B1
#define EBD66723_6B6F_4460_A3DE_00AEB1E6D6B1

#include <boost/json.hpp>
#include <exception>
#include <iostream>
boost::json::value read_json (std::stringstream &is, boost::json::error_code &ec);

boost::json::value read_json (std::string const &jsonAsString, boost::json::error_code &ec);

#ifdef LOG_CO_SPAWN_PRINT_EXCEPTIONS
void inline printExceptionHelper (std::exception_ptr eptr)
{
  try
    {
      if (eptr)
        {
          std::rethrow_exception (eptr);
        }
    }
  catch (std::exception const &e)
    {
      std::cout << "co_spawn exception: '" << e.what () << "'" << std::endl;
    }
}
#else
void inline printExceptionHelper (std::exception_ptr) {}
#endif

template <class... Fs> struct overloaded : Fs...
{
  using Fs::operator()...;
};

template <class... Fs> overloaded (Fs...) -> overloaded<Fs...>;

auto const printException1 = [] (std::exception_ptr eptr) { printExceptionHelper (eptr); };

auto const printException2 = [] (std::exception_ptr eptr, auto) { printExceptionHelper (eptr); };

auto const printException = overloaded{ printException1, printException2 };

#endif /* EBD66723_6B6F_4460_A3DE_00AEB1E6D6B1 */