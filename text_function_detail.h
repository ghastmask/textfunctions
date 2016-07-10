#pragma once

#include <regex>
#include <stdexcept>
#include <string>
#include <cxxabi.h>

namespace txtfn {
namespace detail {

template <typename T> struct remove_class { };

template <typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...)> { using type = R(A...); };

template <typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const> { using type = R(A...); };

template <typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };

template <typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

template <typename T>
struct get_signature_impl { using type = typename remove_class<
    decltype(&std::remove_reference<T>::type::operator())>::type; };

template <typename R, typename... A>
struct get_signature_impl<R(A...)> { using type = R(A...); };

template <typename R, typename... A>
struct get_signature_impl<R(&)(A...)> { using type = R(A...); };

template <typename R, typename... A>
struct get_signature_impl<R(*)(A...)> { using type = R(A...); };

template <typename T> using get_signature = typename get_signature_impl<T>::type;

template <class T>
const std::string& demangle_i() {
  const static std::string name = [] {
    int status = 0;
    const char* name = typeid(T).name();
    char* good_name = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (status != 0 || good_name == nullptr) {
      if (good_name != nullptr) {
        free(good_name);
      }
      throw std::runtime_error(std::string("Unable to demangle: ") + name);
    }
    const std::string str_name(good_name);
    free(good_name);
    return str_name;
  }();
  return name;
}
template <class T>
const std::string& demangle() {
  const static std::string name = [] {
    const auto& demangled = demangle_i<T>();
    const auto& str = demangle_i<std::string>();
    // Cleans up std::string output
    return std::regex_replace(demangled, std::regex(str), "std::string");
  }();
  return name;
}

}
}
