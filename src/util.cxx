//
// Created by walde on 12/2/23.
//
#include "util.hxx"
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <sstream>
#include <string_view>
boost::json::value
read_json (std::stringstream &is, boost::json::error_code &ec)
{
  boost::json::stream_parser p;
  std::string line;
  while (std::getline (is, line))
    {
      p.write (line, ec);
      if (ec) return nullptr;
    }
  p.finish (ec);
  if (ec) return nullptr;
  return p.release ();
}

boost::json::value
read_json (std::string const &jsonAsString, boost::json::error_code &ec)
{
  std::stringstream is{};
  is << jsonAsString;
  return read_json (is, ec);
}