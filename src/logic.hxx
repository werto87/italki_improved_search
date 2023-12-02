#ifndef BE98F3C9_250B_4579_8C5A_61582E694F6C
#define BE98F3C9_250B_4579_8C5A_61582E694F6C

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
#include <chrono>
#include <confu_json/util.hxx>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

boost::asio::awaitable<void> tryToEnrollIntoCourse (boost::asio::io_context &ioContext, std::string const &email, std::string const &password, std::string const &courseToEnroll);

#endif /* BE98F3C9_250B_4579_8C5A_61582E694F6C */