# textfunctions

C++ Typed Functions Callable with Text

## Example

``c++
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

# Compatibility

I have only tested this with GCC 6.1.0

# License

See LICENSE. catch.hpp is imported for convenience and is not licensed here. See the Test framework section.

# Test framework

Uses Catch: https://github.com/philsquared/Catch
