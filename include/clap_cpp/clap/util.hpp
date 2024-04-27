#pragma once

#include <spdlog/spdlog.h>

#include <algorithm>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

/**
 * @brief Utility functions for the Command-Line Argument Parser.
 */
namespace clap::util {

/**
 * @brief Check if a value is equal to any of the given values.
 *
 * @tparam T The type of the value.
 * @tparam Args Parameter pack of types that are implicitly convertible to `T`.
 * @param value The value to compare.
 * @param args The values to compare to.
 * @return true If the value is equal to any of the given values.
 */
template<typename T, typename... Args>
[[nodiscard]] inline constexpr auto equals_any(const T &value, Args... args) -> bool
{
  return ((value == args) || ...);
}

/**
 * @brief Type trait to check if a type is implicitly convertible to `std::string_view`.
 *
 * @tparam T
 */
template<typename T> struct [[nodiscard]] is_convertible_to_string_view
{
  static constexpr bool value = std::is_convertible_v<T, std::string_view>;
};

/**
 * @brief Check if all types in a parameter pack are implicitly convertible to `std::string_view`.
 *
 * @tparam Args Parameter pack of types.
 * @return true If all types in the parameter pack are implicitly convertible to `std::string_view`.
 */
template<typename... Args> [[nodiscard]] inline constexpr auto all_convertible_to_string_view() -> bool
{
  return (is_convertible_to_string_view<Args>::value && ...);
}

/**
 * @brief Check if an option or any of its aliases are set
 *
 * @note Checks if a vector of `std::string_view`s contains the name of an option.
 *
 * @tparam Options Parameter pack of types that are implicitly convertible to `std::string_view`.
 * @param args The vector of `std::string_view`s.
 * @param option_names The names of the options.
 * @return true If the option is defined, meaning the vector contains any of the option names.
 */
template<typename... Options>
[[nodiscard]] auto has_option(const std::vector<std::string_view> &args, Options... option_names) -> bool
{
  static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");

  // Cannot use std::ranges because apple clang still does not support it...
  return std::any_of(
    args.cbegin(), args.cend(), [&](const std::string_view &arg) { return util::equals_any(arg, option_names...); });
}

/**
 * @brief Check if an option or any of its aliases are set and return the argument if it is.
 *
 * @tparam Options Parameter pack of types that are implicitly convertible to `std::string_view`.
 * @param args The vector of `std::string_view`s to search.
 * @param option_names The names of the options.
 * @return std::optional<std::string_view> The argument of the option if it is defined, `std::nullopt` otherwise.
 */
template<typename... Options>
[[nodiscard]] auto has_option_arg(const std::vector<std::string_view> &args, Options... option_names)
  -> std::optional<std::string_view>
{
  static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");


  for (auto it = args.cbegin(), end = args.cend(); it != end; ++it) {
    if (util::equals_any(*it, option_names...)) {
      if (it + 1 != end) {
        return *(it + 1);
      } else {
        spdlog::error("Option {} was specified but no argument was given", *it);
      }
    }
  }

  return std::nullopt;
}


}// namespace clap::util