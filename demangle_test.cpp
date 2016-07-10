#include "text_function_detail.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace txtfn::detail;

namespace test {
  struct TestDemangle{};
}
TEST_CASE("demangle") {
  REQUIRE(demangle<std::string>() == "std::string");
  REQUIRE(demangle<std::vector<std::string>>() == "std::vector<std::string, std::allocator<std::string > >");
  REQUIRE(demangle<int>() == "int");
  REQUIRE(demangle<test::TestDemangle>() == "test::TestDemangle");
}
