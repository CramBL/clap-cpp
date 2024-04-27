#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>


namespace clap {


/**
 * @brief Struct for defining a command for the Command-Line Argument Parser.
 *
 * @note A command is a positional argument, i.e. it is not prefixed with a dash.
 *
 * @example `constexpr auto my_cmd = clap::Command{ "help", false }`
 */
struct [[nodiscard]] Command
{
  std::string_view name_;
  bool is_flag_;

  /**
   * @brief Construct a new Command object.
   *
   * @tparam T_Name A type that is implicitly convertible to `std::string_view`.
   * @tparam T_Alias A type that is implicitly convertible to `std::optional<std::string_view>`.
   *
   * @param name The name of the command.
   * @param is_flag Whether the command is a flag or not. A flag does not require a trailing value.
   */
  template<std::convertible_to<std::string_view> T_Name,
    std::convertible_to<std::optional<std::string_view>>... T_Alias>
  [[nodiscard]] constexpr explicit Command(T_Name name, bool is_flag) noexcept : name_{ name }, is_flag_{ is_flag }
  {}
};

/**
 * @brief Struct for defining an array of commands for the Command-Line Argument Parser.
 *
 * @tparam N_cmds The number of commands.
 *
 * @note This is a wrapper around `std::array<clap::Command, N_cmds>`.
 */
template<size_t N_cmds> struct CommandArray
{
  using T_cmd = clap::Command;

  std::array<T_cmd, N_cmds> cmds_;

  /**
   * @brief Construct a new Command Array object.
   *
   * @tparam T A parameter pack of `clap::Command` objects.
   *
   * @param cmds The commands.
   *
   * @note This constructor is only needed to avoid having to specify the template parameters.
   */
  template<class... T> [[nodiscard]] constexpr explicit CommandArray(T... cmds) noexcept : cmds_{ cmds... } {}

  /**
   * @brief Returns the number of defined commands.
   */
  [[nodiscard]] constexpr auto size() const noexcept { return cmds_.size(); }

  /**
   * @brief Prints the names of the defined commands to stdout.
   */
  void print() const noexcept
  {
    for (const T_cmd &cmd : cmds_) { fmt::print("{}\n", cmd.name_); }
  }

  /**
   * @brief Searches for a command with the given name.
   *
   * @param cmd_name The name of the command.
   * @return std::optional<T_cmd> The command if found, `std::nullopt` otherwise.
   *
   * @note The search is case-sensitive.
   */
  [[nodiscard]] auto find(std::string_view cmd_name) const noexcept -> std::optional<T_cmd>
  {
    auto match_command_name = [&](const T_cmd &cmd) { return cmd.name_ == cmd_name; };


    if (auto iter = std::find_if(cmds_.begin(), cmds_.end(), match_command_name); iter != cmds_.end()) {
      return *iter;
    } else {
      return std::nullopt;
    }
  }
};


}// namespace clap
