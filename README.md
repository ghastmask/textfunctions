# textfunctions

C++ Typed Functions Callable with Text

## Example

```c++
auto multiply = create_text_function([&] (int a, int b) { return a * b; });
std::string res;
std::vector<std::string> args{2,5};
multiply->call(args, res);
assert("10" == res);
TextFunctionLibrary lib;
lib.add(std::move(multiply),
    TextFunctionHelp("multiply")
      .arg("multiplicand", "it is multiplied")
      .arg("multiplier", "it multiplies")
      .description("Multiplies two numbers"));
res.clear();
lib.call("multiply", args, res);
assert("10" == res);
```

String arguments are automatically converted to their correct type to call the function. There are also
builtin functions inside of TextFunctionLibrary for getting help and searching for functions. Some uses cases would be
a game input console (~ in many FPS games) or a basic text RPC mechanism.

Examples from the test file:

Sample help output:

```
std::string concat [std::string] [std::string] -- Concatenates two strings
std::string help -- Returns list of functions
std::string help [std::string] -- Returns detailed help for matching function
int multiply [int] [int] -- Multiplies two numbers
std::string search [std::string] -- Returns list of functions matching regex
```

Sample detailed help output for the multiply function:

```
int multiply [int] [int]
  Description: Multiplies two numbers
  Arguments:
      multiplicand: it is multiplied
      multiplier: it multiplies
```

Searching for "two":

```
std::string concat [std::string] [std::string]
  Description: Concatenates two strings
  Arguments:
    str1: a string
    str2: another string

int multiply [int] [int]
  Description: Multiplies two numbers
  Arguments:
    multiplicand: it is multiplied
    multiplier: it multiplies
```

# Compatibility

I have only tested this with GCC 6.1.0

# License

See LICENSE. catch.hpp is imported for convenience and is not licensed here. See the Test framework section.

# Test framework

Uses Catch: https://github.com/philsquared/Catch
