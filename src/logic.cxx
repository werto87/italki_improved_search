#include "logic.hxx"

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

boost::asio::awaitable<void>
tryToEnrollIntoCourse (boost::asio::io_context &ioContext, std::string const &email, std::string const &password, std::string const &courseToEnroll)
{
  try
    {
      ssl::context ctx (ssl::context::tlsv12_client);
      ctx.set_verify_mode (ssl::context::verify_none); // do not use this in production it turns verification off
      tcp::resolver resolver (ioContext);
      beast::ssl_stream<beast::tcp_stream> stream (ioContext, ctx);
      auto host = "community.nimbuscloud.at";
      auto port = "443";
      int version = 11;
      auto const results = co_await resolver.async_resolve (host, port, boost::asio::use_awaitable);
      co_await beast::get_lowest_layer (stream).async_connect (results, boost::asio::use_awaitable);
      co_await stream.async_handshake (ssl::stream_base::client, boost::asio::use_awaitable);
      http::request<http::string_body> authenticate{ http::verb::post, "/api/v1/authenticate", version };
      authenticate.set (http::field::host, host);
      authenticate.set (http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      authenticate.set (http::field::content_type, "application/x-www-form-urlencoded; charset=UTF-8");
      authenticate.body () = "account-email=" + email + "&account-password=" + password + "&init-system=default";
      authenticate.prepare_payload ();
      co_await http::async_write (stream, authenticate, boost::asio::use_awaitable);
      beast::flat_buffer buffer;
      http::response<http::string_body> res;
      co_await http::async_read (stream, buffer, res, boost::asio::use_awaitable);
      if (res.result_int () == 400)
        {
          std::cout << "authenticate response code 400" << std::endl;
          std::cout << res.body () << std::endl;
          abort ();
        }
      std::vector<std::string> splitMesssage{};
      boost::algorithm::split (splitMesssage, res.find (http::field::set_cookie)->value (), boost::is_any_of ("; "));
      auto const &cookie = splitMesssage.at (0);
      http::request<http::string_body> userBaseData{ http::verb::post, "/api/v1/user/base-data", version };
      userBaseData.set (http::field::host, host);
      userBaseData.set (http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      userBaseData.set (http::field::accept, "application/json, text/javascript, */*; q=0.01");
      userBaseData.set (http::field::cookie, cookie);
      userBaseData.set (http::field::content_type, "application/x-www-form-urlencoded; charset=UTF-8");
      co_await http::async_write (stream, userBaseData, boost::asio::use_awaitable);
      http::response<http::string_body> userBaseDataResponse;
      co_await http::async_read (stream, buffer, userBaseDataResponse, boost::asio::use_awaitable);
      boost::json::error_code userBaseDataResponseJsonError{};
      auto userBaseDataResponseJson = boost::json::value_to<boost::json::object> (confu_json::read_json (userBaseDataResponse.body (), userBaseDataResponseJsonError));
      if (userBaseDataResponseJsonError)
        {
          std::cout << "userBaseDataResponseJsonError: " << userBaseDataResponseJsonError.what () << std::endl;
          abort ();
        }
      auto const customerNumber = userBaseDataResponseJson["customer"].as_object ()["customernr"].as_string ();
      http::request<http::string_body> preCheckinCourses{ http::verb::post, "/api/v1/checkin/pre-checkin-courses", version };
      preCheckinCourses.set (http::field::host, host);
      preCheckinCourses.set (http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      preCheckinCourses.set (http::field::accept, "application/json, text/javascript, */*; q=0.01");
      preCheckinCourses.set (http::field::cookie, cookie);
      preCheckinCourses.set (http::field::content_type, "application/x-www-form-urlencoded; charset=UTF-8");
      preCheckinCourses.body () = "selectedCustomer=" + std::string{ customerNumber };
      preCheckinCourses.prepare_payload ();
      co_await http::async_write (stream, preCheckinCourses, boost::asio::use_awaitable);
      http::response<http::string_body> preCheckinCoursesResponse;
      co_await http::async_read (stream, buffer, preCheckinCoursesResponse, boost::asio::use_awaitable);
      boost::json::error_code preCheckinCoursesResponseJsonError{};
      auto json = boost::json::value_to<boost::json::object> (confu_json::read_json (preCheckinCoursesResponse.body (), preCheckinCoursesResponseJsonError));
      if (preCheckinCoursesResponseJsonError)
        {
          std::cout << "preCheckinCoursesResponseJsonError: " << preCheckinCoursesResponseJsonError.what () << std::endl;
          abort ();
        }
      for (auto day : json["content"].as_object ()["locations"].as_array ()[0].as_object ()["days"].as_array ())
        {
          for (auto course : day.as_object ()["courses"].as_array ())
            {
              if (course.as_object ()["name"].as_string () == courseToEnroll and not course.as_object ()["visitExists"].as_bool ())
                {
                  std::cout << "Queue for course:" << std::endl;
                  std::cout << "name: " << course.as_object ()["name"].as_string () << std::endl;
                  std::cout << "start_date: " << course.as_object ()["start_date"].as_string () << std::endl;
                  std::cout << "start_time: " << course.as_object ()["start_time"].as_string () << std::endl;
                  std::cout << "end_time: " << course.as_object ()["end_time"].as_string () << std::endl;
                  http::request<http::string_body> togglePreCheckin{ http::verb::post, "/api/v1/checkin/toggle-pre-checkin", version };
                  togglePreCheckin.set (http::field::host, host);
                  togglePreCheckin.set (http::field::user_agent, BOOST_BEAST_VERSION_STRING);
                  togglePreCheckin.set (http::field::accept, "application/json, text/javascript, */*; q=0.01");
                  togglePreCheckin.set (http::field::cookie, cookie);
                  togglePreCheckin.set (http::field::content_type, "application/x-www-form-urlencoded; charset=UTF-8");
                  togglePreCheckin.body () = "event=" + std::string{ course.as_object ()["event"].as_string () } + "&selectedCustomer=" + std::string{ customerNumber };
                  togglePreCheckin.prepare_payload ();
                  co_await http::async_write (stream, togglePreCheckin, boost::asio::use_awaitable);
                  http::response<http::string_body> togglePreCheckinResponse;
                  co_await http::async_read (stream, buffer, togglePreCheckinResponse, boost::asio::use_awaitable);
                }
            }
        }
    }
  catch (std::exception const &e)
    {
      std::cerr << "Error: " << e.what () << std::endl;
    }
}