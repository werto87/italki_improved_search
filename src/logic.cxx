#include "logic.hxx"
#include "util.hxx"
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <fstream>
#include <iostream>
namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

boost::asio::awaitable<void>
getCheapestTeacher (boost::asio::io_context &ioContext)
{
  boost::json::value result = co_await teacherRequest (ioContext, 1);
  auto teachers = std::vector<boost::json::value>{};
  for (uint64_t i = 1; i <= result.at_pointer ("/statistics/count").as_uint64 () / 20; ++i)
    {
      //      TODO extract just the teachers and put them in one array
      teachers.push_back (co_await teacherRequest (ioContext, i));
    }
  auto test = 42;
}

boost::asio::awaitable<boost::json::value>
teacherRequest (boost::asio::io_context &ioContext, uint64_t page)
{
  try
    {
      ssl::context ctx (ssl::context::tlsv12_client);
      ctx.set_verify_mode (ssl::context::verify_none); // do not use this in production it turns verification off
      tcp::resolver resolver (ioContext);
      beast::ssl_stream<beast::tcp_stream> stream (ioContext, ctx);
      auto host = "api.italki.com";
      auto port = "443";
      int version = 11;
      if (!SSL_set_tlsext_host_name (stream.native_handle (), host))
        {
          boost::system::error_code ec{ static_cast<int> (::ERR_get_error ()), boost::asio::error::get_ssl_category () };
          throw boost::system::system_error{ ec };
        }
      auto const results = co_await resolver.async_resolve (host, port, boost::asio::use_awaitable);
      co_await beast::get_lowest_layer (stream).async_connect (results, boost::asio::use_awaitable);
      co_await stream.async_handshake (ssl::stream_base::client, boost::asio::use_awaitable);
      http::request<http::string_body> teacherReq{ http::verb::post, "/api/v2/teachers", version };
      teacherReq.set (http::field::host, host);
      teacherReq.set (http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      teacherReq.set (http::field::accept, "application/json, text/plain, */*");
      teacherReq.set (http::field::accept_language, "en-US,en;q=0.5");
      teacherReq.set (http::field::accept_encoding, "gzip");
      teacherReq.set (http::field::content_type, "application/json");
      teacherReq.set (http::field::x_device_accept, "10");
      teacherReq.set (http::field::content_length, "139");
      teacherReq.set (http::field::origin, "https://www.italki.com");
      teacherReq.set (http::field::connection, "keep-alive");
      teacherReq.set (http::field::referer, "https://www.italki.com/");
      teacherReq.set (http::field::sec_fetch_dest, "empty");
      teacherReq.set (http::field::sec_fetch_mode, "cors");
      teacherReq.set (http::field::sec_fetch_site, "same-site");
      teacherReq.set (http::field::te, "trailers");
      auto teacherRequestBody = std::stringstream{};
      // clang-format off
      teacherRequestBody<<R"({"teach_language":{"language":"japanese"},"page_size":20,"user_timezone":"Asia/Tokyo","page":)";
      teacherRequestBody << page;
      teacherRequestBody<<"}";
      // clang-format on
      teacherReq.body () = teacherRequestBody.str ();
      teacherReq.prepare_payload ();
      co_await http::async_write (stream, teacherReq, boost::asio::use_awaitable);
      beast::flat_buffer buffer;
      http::response<http::string_body> res;
      co_await http::async_read (stream, buffer, res, boost::asio::use_awaitable);
      std::stringstream ss{};
      ss << res.body ();
      boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
      inbuf.push (boost::iostreams::gzip_decompressor ());
      inbuf.push (ss);
      std::stringstream decomp{};
      boost::iostreams::copy (inbuf, decomp);
      auto ec = boost::json::error_code{};
      auto result = boost::json::parse (decomp.str (), ec);
      if (ec)
        {
          abort ();
        }
      co_return result;
    }
  catch (std::exception const &e)
    {
      std::cerr << "Error: " << e.what () << std::endl;
      co_return std::string{};
    }
}