#include "text_function_detail.h"
#include <experimental/tuple>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#include <type_traits>

namespace txtfn {
template <class Last>
void type(int& i, std::vector<std::string>& args) {
  args.emplace_back(detail::demangle<Last>());
  ++i;
}

template <class First, class Second, class ...Rest>
void type(int& i, std::vector<std::string>& args) {
  args.emplace_back(detail::demangle<First>());
  ++i;
  type<Second, Rest...>(i, args);
}

// handle empty case
template <class...Args>
struct GetType {
  static void get(std::vector<std::string>& args) {
    int i = 0;
    type<Args...>(i, args);
  }
};

template <>
struct GetType<> {
  static void get(std::vector<std::string>&) {
  }
};

class TextFunctionBase {
 public:
  virtual ~TextFunctionBase() { }
  virtual void call(const std::vector<std::string>& args, std::string& out) const = 0;
  virtual const std::string& return_type() const = 0;
  virtual const std::vector<std::string>& arg_types() const = 0;
  virtual const std::string& help(const std::string& name) const =0;
};

template<class F, class Sig>
class TextFunction;

template<class F, class Ret, class... Args>
class TextFunction<F, Ret (Args...)> : public TextFunctionBase {
public:
  TextFunction(F&& f)
  : callable_(f) {
  }

  void call(const std::vector<std::string>& args, std::string& out) const override {
    call_impl(args, out, std::is_void<Ret>());
  }

private:
  const std::string& return_type() const override {
    return detail::demangle<Ret>();
  }

  const std::vector<std::string>& arg_types() const override {
    static std::vector<std::string> args = [] {
      std::vector<std::string> args;
      GetType<Args...>::get(args);
      return args;
    }();
    return args;
  }

  const std::string& help(const std::string& name) const {
    static std::string help = [&] {
      constexpr const char* GREEN = "\x1b[32m";
      constexpr const char* RESET = "\x1b[0m";
      std::ostringstream ss;
      ss << return_type() << " " << GREEN << name << RESET;
      auto it = arg_types().begin();
      auto end = arg_types().end();
      if (it != end) {
        ss << " [" << *it++ << "]";
      }

      for (; it != end; ++it) {
        ss << " [" << *it << "]";
      }
      return ss.str();
    }();
    return help;
  }

  template <class T>
  std::decay_t<T> get(size_t& i, const std::vector<std::string>& arg) const {
    std::decay_t<T> t;
    std::stringstream a(arg[i]);
    if (!(a >> t)) {
      throw std::invalid_argument("Unable to convert arg: '" + arg[i] + "' to " + detail::demangle<T>());
    }
    ++i;
    return t;
  }

  void validate_args(const std::vector<std::string>& args) const {
    if (args.size() != sizeof...(Args)) {
      throw std::runtime_error("Wrong number of args: " + std::to_string(args.size())
          + " != " + std::to_string(sizeof...(Args)));
    }
  }

  void call_impl(const std::vector<std::string>& args, std::string&, std::true_type) const {
    validate_args(args);
    size_t index = 0;
    (void)index; // Remove unused warning when zero arg function
    std::experimental::apply(callable_, std::tuple<Args...>{get<Args>(index, args)...});
  }

  void call_impl(const std::vector<std::string>& args, std::string& out, std::false_type) const{
    validate_args(args);
    std::stringstream conv;
    size_t index = 0;
    (void)index; // Remove unused warning when zero arg function
    conv << std::experimental::apply(callable_, std::tuple<Args...>{get<Args>(index, args)...});
    if(!conv) {
      throw std::runtime_error("Failed to convert return type " + detail::demangle<Ret>() + " to string");
    }
    out = conv.str();
  }

  F callable_;
};

// Handle lambdas
template <typename S, typename F>
class TextFunction : public TextFunction<F, decltype(&F::operator())> { };

class TextFunctionHelp {
 public:
  TextFunctionHelp(const std::string& name) : name_(name) { }
  TextFunctionHelp& arg(const std::string& name, const std::string& description) {
    args_.emplace_back(Arg{name, description});
    return *this;
  }
  TextFunctionHelp& description(const std::string& description) {
    description_ = description;
    return *this;
  }
  const std::string& name() const { return name_; }
  const std::string& description() const { return description_; }
  struct Arg {
    std::string name;
    std::string description;
  };
  const std::vector<Arg>& args() const { return args_; }

 private:
  std::string name_;
  std::string description_{"No description"};
  std::vector<Arg> args_;
};

struct Lookup {
  const std::string& name;
  size_t num_args;
};

template <class F>
std::unique_ptr<TextFunctionBase> create_text_function(F&& f) {
  return std::make_unique<TextFunction<F, detail::get_signature<F>>>(std::forward<F>(f));
}

class TextFunctionLibrary {
 public:
  TextFunctionLibrary() {
    register_help();
    register_search();
  }

  void register_search() {
    auto search = create_text_function([&] (const std::string& str) {
      std::regex r(str);
      std::ostringstream ss;
      for (auto& fn : funcs_) {
        auto data = detailed_help(fn.first.name());
        if (std::regex_search(data, r)) {
          ss << data << "\n";
        }
      }
      if (ss.str().empty()) {
        ss << "Nothing found for pattern: " << str;
      }
      return ss.str();
    });
    add(std::move(search),
     TextFunctionHelp("search")
       .arg("search_regex", "Regex is matched against names, descriptions, and arguments")
       .description("Returns list of functions matching regex"));
  }

  std::string detailed_help(const std::string& name) {
    std::ostringstream ss;
    for (auto& fn : funcs_) {
      if (fn.first.name() == name) {
        ss << fn.second->help(fn.first.name()) << "\n  Description: ";
        ss << fn.first.description() << "\n  Arguments:\n";
        for (auto& it : fn.first.args()) {
          ss << "    " << it.name << ": " << it.description << "\n";
        }
      }
    }
    if (ss.str().empty()) {
      ss << name << " not found";
    }
    return ss.str();
  }

  void register_help() {
    auto help = create_text_function([&] {
      std::ostringstream ss;
      for (auto& fn : funcs_) {
        ss << fn.second->help(fn.first.name()) << " -- " << fn.first.description() << "\n";
      }
      return ss.str();
    });
    add(std::move(help),
     TextFunctionHelp("help")
       .description("Returns list of functions"));

    auto detailed_help = create_text_function([&] (const std::string& name) {
        return this->detailed_help(name);
    });
    add(std::move(detailed_help),
     TextFunctionHelp("help")
       .arg("func_name", "function name to retrieve detailed help for")
       .description("Returns detailed help for matching function"));
  }

  void add(std::unique_ptr<TextFunctionBase> fn, TextFunctionHelp help) {
    if (help.args().size() != fn->arg_types().size()) {
      throw std::runtime_error("Help has different number of arguments: "
          + std::to_string(help.args().size()) + " != " + std::to_string(fn->arg_types().size()));
    }
    if (funcs_.find(help) != funcs_.end()) {
      throw std::runtime_error(help.name() + " already registered.");
    }
    funcs_[help] = std::move(fn);
  }

  void add(const std::string& name, std::unique_ptr<TextFunctionBase> fn) {
    TextFunctionHelp h(name);
    for (auto& arg : fn->arg_types()) {
      h.arg(arg, arg);
    }
    h.description(fn->return_type());

    if (funcs_.find(h) != funcs_.end()) {
      throw std::runtime_error(name + " already registered.");
    }
    funcs_[h] = std::move(fn);
  }

  // Return false if function can not be found
  bool call(const std::string& name, const std::vector<std::string>& args, std::string& out) {
    const Lookup l{name, args.size()};
    auto it = funcs_.find(l);
    if (it == funcs_.end()) {
      return false;
    }
    it->second->call(args, out);
    return true;
  }
 private:
  std::map<TextFunctionHelp, std::unique_ptr<TextFunctionBase>, std::less<>> funcs_;
};

inline bool operator<(const TextFunctionHelp& lhs, const Lookup& rhs) {
  const auto lhs_size = lhs.args().size();
  return std::tie(lhs.name(), lhs_size) < std::tie(rhs.name, rhs.num_args);
}

inline bool operator<(const TextFunctionHelp& lhs, const TextFunctionHelp& rhs) {
  const auto lhs_size = lhs.args().size();
  const auto rhs_size = rhs.args().size();
  return std::tie(lhs.name(), lhs_size) < std::tie(rhs.name(), rhs_size);
}

inline bool operator<(const Lookup& lhs, const TextFunctionHelp& rhs) {
  const auto rhs_size = rhs.args().size();
  return std::tie(lhs.name, lhs.num_args) < std::tie(rhs.name(), rhs_size);
}

}
