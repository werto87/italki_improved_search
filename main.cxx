#include "src/logic.hxx"
#include "src/util.hxx"
#include <Corrade/Utility/Arguments.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/json/src.hpp>
#include <chrono>
#include <confu_json/util.hxx>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int
main (int argc, char **argv)
{
  auto minutesToWait = std::chrono::minutes{ 10 };
  boost::asio::io_context ioc;
  for (;;)
    {
      boost::asio::co_spawn (ioc, getCheapestTeacher (ioc), printException);
      boost::asio::system_timer timer{ ioc };
      timer.expires_after (std::chrono::minutes{ minutesToWait });
      boost::asio::co_spawn (ioc, timer.async_wait (boost::asio::use_awaitable), printException);
      ioc.run ();
      ioc.stop ();
      ioc.reset ();
    }
  return 0;
}
