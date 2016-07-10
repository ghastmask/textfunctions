#include "text_function.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace txtfn;

TEST_CASE("text function void return") {
  int i = 0;
  auto f = create_text_function([&] (int b, std::string) { i = b; });
  std::string res;
  f->call(std::vector<std::string>{"3", "go"}, res);
  REQUIRE(res.empty());
  REQUIRE(i == 3);
  REQUIRE("void" == f->return_type());
  REQUIRE("int" == f->arg_types()[0]);
  REQUIRE("std::string" == f->arg_types()[1]);
  REQUIRE("void \x1b[32mname\x1b[0m [int] [std::string]" == f->help("name"));
}

TEST_CASE("text function non void return") {
  auto f = create_text_function([&] (int b, const std::string& str) { 
    return std::to_string(b) + str;;
  });

  std::string res;
  f->call(std::vector<std::string>{"3", "go"}, res);
  REQUIRE(res == "3go");
  REQUIRE("std::string" == f->return_type());
  REQUIRE("int" == f->arg_types()[0]);
  REQUIRE("std::string" == f->arg_types()[1]);
  REQUIRE("std::string \x1b[32mname\x1b[0m [int] [std::string]" == f->help("name"));
}

TEST_CASE("text function wrong args") {
  auto f = create_text_function([&] (int b, const std::string& str) { 
    return std::to_string(b) + str;;
  });

  std::string res;
  try {
    f->call(std::vector<std::string>{"go", "go"}, res);
    FAIL("didnt throw");
  } catch(const std::exception& e) {
    REQUIRE("Unable to convert arg: 'go' to int" == std::string(e.what()));
  }

  try {
    f->call(std::vector<std::string>{"1", "go", "3"}, res);
    FAIL("didnt throw");
  } catch(const std::exception& e) {
    REQUIRE("Wrong number of args: 3 != 2" == std::string(e.what()));
  }
}

TEST_CASE("text function library no dupes") {
  TextFunctionLibrary lib;

  auto fn = create_text_function([&] () { });
  auto fn2 = create_text_function([&] () { });
  lib.add(std::move(fn), TextFunctionHelp("fn"));
  try {
    lib.add(std::move(fn2), TextFunctionHelp("fn"));
    FAIL("didnt throw");
  } catch (const std::exception& e) {
    REQUIRE("fn already registered." == std::string(e.what()));
  }
}

TEST_CASE("text function library") {
  TextFunctionLibrary lib;

  auto multiply = create_text_function([&] (int a, int b) { return a * b; });
  lib.add(std::move(multiply),
      TextFunctionHelp("multiply")
        .arg("multiplicand", "it is multiplied")
        .arg("multiplier", "it multiplies")
        .description("Multiplies two numbers"));

  auto concat = create_text_function([&] (std::string a, std::string b) { return a + b; });
  lib.add(std::move(concat),
      TextFunctionHelp("concat")
        .arg("str1", "a string")
        .arg("str2", "another string")
        .description("Concatenates two strings"));

  SECTION("builtin help") {
    std::string res;
    lib.call("help", std::vector<std::string>{}, res);

    const std::string expected =
      "std::string \x1b[32mconcat\x1b[0m [std::string] [std::string] -- Concatenates two strings\n"
      "std::string \x1b[32mhelp\x1b[0m -- Returns list of functions\n"
      "std::string \x1b[32mhelp\x1b[0m [std::string] -- Returns detailed help for matching function\n"
      "int \x1b[32mmultiply\x1b[0m [int] [int] -- Multiplies two numbers\n"
      "std::string \x1b[32msearch\x1b[0m [std::string] -- Returns list of functions matching regex\n";
    REQUIRE(expected == res);
    lib.call("help", std::vector<std::string>{"func"}, res);
    REQUIRE("func not found" == res);
  }

  SECTION("builtin detailed help") {
    std::string res;
    lib.call("help", std::vector<std::string>{"multiply"}, res);

    const std::string expected =
      "int \x1b[32mmultiply\x1b[0m [int] [int]\n"
      "  Description: Multiplies two numbers\n"
      "  Arguments:\n"
      "    multiplicand: it is multiplied\n"
      "    multiplier: it multiplies\n";
    REQUIRE(expected == res);
  }

  SECTION("builtin search") {
    std::string res;
    lib.call("search", std::vector<std::string>{"two"}, res);
    const std::string expected =
      "std::string \x1b[32mconcat\x1b[0m [std::string] [std::string]\n"
      "  Description: Concatenates two strings\n"
      "  Arguments:\n"
      "    str1: a string\n"
      "    str2: another string\n"
      "\n"
      "int \x1b[32mmultiply\x1b[0m [int] [int]\n"
      "  Description: Multiplies two numbers\n"
      "  Arguments:\n"
      "    multiplicand: it is multiplied\n"
      "    multiplier: it multiplies\n"
      "\n";
    REQUIRE(expected == res);
  }

  SECTION("no function") {
    std::string res;
    REQUIRE_FALSE(lib.call("badfunc", std::vector<std::string>{}, res));
  }
}
