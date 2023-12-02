#include "src/logic.hxx"
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
#include "src/util.hxx"

// TODO make a systemd service for this and run it

int
main (int argc, char **argv)
{
  Corrade::Utility::Arguments args;
  // clang-format off
  args
    .addNamedArgument("email").setHelp("email", "email address")
    .addNamedArgument("password").setHelp("password", "password")
    .addNamedArgument("courseToEnroll").setHelp("courseToEnroll", "course to enroll use \"your course for spaces\"")
    .addOption("minutesToWait","60").setHelp("minutesToWait", "minutes to wait until retry to enroll")
    .parse(argc, argv);
  // clang-format on
  auto email = args.value ("email");
  auto password = args.value ("password");
  auto courseToEnroll = args.value ("courseToEnroll");
  auto minutesToWait = std::stoul (args.value ("minutesToWait"));
  boost::asio::io_context ioc;
  for (;;)
    {
      boost::asio::co_spawn (ioc, tryToEnrollIntoCourse (ioc, email, password, courseToEnroll), printException);
      boost::asio::system_timer timer{ ioc };
      timer.expires_after (std::chrono::minutes{ minutesToWait });
      boost::asio::co_spawn (ioc, timer.async_wait (boost::asio::use_awaitable), printException);
      ioc.run ();
      ioc.stop ();
      ioc.reset ();
    }
  return 0;
}