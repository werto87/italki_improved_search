#include "logic.hxx"
#include "util.hxx"
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// clang-format off
auto const sessionLengthTimeInMinutesMapping = std::map<int64_t, std::chrono::minutes>{
                                                                                        { 1, std::chrono::minutes{ 15 } },
                                                                                        { 2, std::chrono::minutes{ 30 } },
                                                                                        { 3, std::chrono::minutes{ 45 } },
                                                                                        { 4, std::chrono::minutes{ 60 } },
                                                                                        { 5, std::chrono::minutes{ 75 } },
                                                                                        { 6, std::chrono::minutes{ 90 } }
                                                                                      };
// clang-format on
boost::asio::awaitable<std::vector<Teacher> >
getTeacherWithPrice (boost::asio::io_context &ioContext, std::function<bool (boost::json::value const &data)> const &filter)
{
  auto teachers = std::vector<Teacher>{};
  boost::json::value result = co_await teacherRequest (ioContext, 1);
  try
    {
      auto count = boost::numeric_cast<uint64_t> (result.at_pointer ("/statistics/count").as_int64 ()) / 20;
      for (uint64_t i = 1; i <= count; ++i)
        {
          boost::json::value const &teachersForPage = co_await teacherRequest (ioContext, i);
          for (auto const &data : teachersForPage.at_pointer ("/data").as_array ())
            {
              if (std::ranges::find_if (teachers, [id = data.at_pointer ("/user_info/user_id").as_int64 ()] (Teacher const &teacher) { return teacher.id == id; }) == teachers.end ())
                {
                  if (filter)
                    {
                      auto pricePerMinute = std::vector<double>{};
                      for (auto const &course : data.at_pointer ("/pro_course_detail").as_array ())
                        {
                          for (auto const &priceList : course.at_pointer ("/price_list").as_array ())
                            {
                              auto price = boost::numeric_cast<double> (priceList.at_pointer ("/session_price").as_int64 ());
                              auto timeInMinutes = sessionLengthTimeInMinutesMapping.at (priceList.at_pointer ("/session_length").as_int64 ());
                              pricePerMinute.emplace_back (price / boost::numeric_cast<double> (timeInMinutes.count ()));
                            }
                        }
                      teachers.emplace_back (Teacher{ .id = data.at_pointer ("/user_info/user_id").as_int64 (), .pricePerMinuteInDollarCent = *std::ranges::min_element (pricePerMinute) });
                    }
                }
            }
        }
    }
  catch (std::exception const &ex)
    {
      std::cout << "std::exception const &ex : " << ex.what () << std::endl;
    }
  catch (...)
    {
      std::cout << "..." << std::endl;
    }
  co_return teachers;
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