#include "src/logic.hxx"
#include <catch2/catch.hpp>

TEST_CASE ("hello", "[hello]") { REQUIRE (helloWorld () == "Hello World!"); }