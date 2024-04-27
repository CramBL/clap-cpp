#pragma once

#include <cstdint>
#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>


namespace clap {

/**
 * @brief The maximum number of aliases an option can have.
 *
 * @warning This is a hard limit, i.e. the program will not compile if you try to define more aliases.
 *
 * @note More than 3 aliases is just too much.
 */
static inline constexpr size_t MAX_ALIAS_COUNT = 3;

/**
 * @brief Enum class for defining the type of an option.
 *
 * @note `Flag` is for options that toggle a boolean value.
 * @note `NeedValue` is for options that require a value.
 */
enum class [[nodiscard]] Opt : uint8_t{ Flag, NeedValue };


/**
 * @brief Struct for defining an command-line option.
 *
 * @example `constexpr auto my_opt = clap::Option{ "help", clap::Opt::Flag, "h", "help", "H" }`
 */
struct [[nodiscard]] Option
{
  using T_opt_sv = std::optional<std::string_view>;
  using T_alias_array = std::array<T_opt_sv, clap::MAX_ALIAS_COUNT>;
  using T_alias_array_opt = std::optional<T_alias_array>;

  std::string_view name_;
  Opt opt_type_;
  // Array should be optional, will then be std::nullopt instead of empty array if there's no aliases
  T_alias_array_opt aliases_;


  /**
   * @brief Construct a new Option object.
   *
   * @tparam T_Name A type that is implicitly convertible to `std::string_view`.
   *
   * @param name The name of the option.
   *
   * @tparam T_Alias A type that is implicitly convertible to `std::string_view`.
   *
   * @param aliases The aliases of the option.
   * @param opt_type The type `Opt` of the option
   *
   * @note The first alias is the "main" alias, i.e. the one that will be used in the help text.   *
   */
  template<std::convertible_to<std::string_view> T_Name, std::convertible_to<T_opt_sv>... T_Alias>
  [[nodiscard]] constexpr explicit Option(T_Name name, Opt opt_type, T_Alias... aliases) noexcept
    : name_{ name }, opt_type_{ opt_type }, aliases_{ T_alias_array{ T_opt_sv{ aliases }... } }
  {
    // Just to improve compiler error
    static_assert(
      sizeof...(aliases) <= clap::MAX_ALIAS_COUNT, "Too many aliases provided in initialization of struct `Option`");
  }

  /**
   * @brief Returns whether the option has any defined aliases.
   */
  [[nodiscard]] constexpr bool has_alias() const
  {
    return aliases_.has_value() && aliases_.value().front().has_value();
  }
};


/**
 * @brief Struct for defining an array of `clap::Option`s.
 *
 * @tparam N_opts The number of options.
 */
template<size_t N_opts> struct OptionArray
{
  using T_opt = clap::Option;

  std::array<T_opt, N_opts> opts_;

  /**
   * @brief Construct a new Option Array object.
   *
   * @tparam T A parameter pack of `clap::Option` objects.
   * @param opts The options.
   */
  template<class... T> [[nodiscard]] constexpr explicit OptionArray(T... opts) noexcept : opts_{ opts... } {}

  /**
   * @brief Returns the number of defined options.
   */
  [[nodiscard]] constexpr auto size() const noexcept { return opts_.size(); }

  /**
   * @brief Prints the names of the defined options.
   */
  void print() const noexcept
  {
    for (const T_opt &opt : opts_) { fmt::print("{}\n", opt.name_); }
  }

  /**
   * @brief Searches for an option with the given name.
   *
   * @param opt_name The name of the option.
   * @return std::optional<T_opt> The option if found, `std::nullopt` otherwise.
   *
   * @note The search is case-sensitive.
   */
  [[nodiscard]] auto find(std::string_view opt_name) const noexcept -> std::optional<T_opt>
  {
    auto match_option_name = [&](const T_opt &opt) {
      return opt.name_ == opt_name
             || (opt.has_alias()
                 && std::any_of(opt.aliases_.value().begin(), opt.aliases_.value().end(), [&](const auto &name) {
                      return name.has_value() && name.value() == opt_name;
                    }));
    };


    if (auto iter = std::find_if(opts_.begin(), opts_.end(), match_option_name); iter != opts_.end()) [[likely]] {
      return *iter;
    } else [[unlikely]] {
      return std::nullopt;
    }
  }
};


}// namespace clap