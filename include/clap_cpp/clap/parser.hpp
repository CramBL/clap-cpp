#pragma once

#include "clap_cpp/clap/command.hpp"
#include "clap_cpp/clap/option.hpp"
#include "clap_cpp/clap/util.hpp"

#include <fmt/color.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <optional>
#include <string_view>
#include <utility>


namespace clap {


/**
 * @brief The main Command-Line Argument Parser (CLAP) class.
 *
 * @tparam N_opts The number of defined options.
 * @tparam N_cmds The number of defined commands.
 */
template<size_t N_opts, size_t N_cmds> class Clap
{
  // User-defined options and commands
  std::optional<clap::OptionArray<N_opts>> options_;
  std::optional<clap::CommandArray<N_cmds>> commands_;

  // Options/command set from the command-line (generated from parsing the command-line arguments)
  std::optional<std::vector<std::pair<clap::Option, std::optional<std::string_view>>>> set_options_;
  std::optional<clap::Command> set_cmd_;// only single command allowed (TODO: support subcommands)
  bool is_clap_parsed = false;


public:
  /**
   * @brief Construct a new Clap object with the given options and commands.
   *
   * @param opts_arr A `clap::OptionArray` with the defined options.
   * @param cmds_arr A `clap::CommandArray` with the defined commands.
   */
  [[nodiscard]] constexpr explicit Clap(clap::OptionArray<N_opts> opts_arr,
    clap::CommandArray<N_cmds> cmds_arr) noexcept
    : options_{ opts_arr }, commands_{ cmds_arr }, set_options_{ std::nullopt }, set_cmd_{ std::nullopt }
  {}

  /**
   * @brief A default constructor that does not define any options or commands.
   */
  [[nodiscard]] constexpr explicit Clap(std::optional<clap::OptionArray<N_opts>> opts_arr = std::nullopt,
    std::optional<clap::CommandArray<N_cmds>> cmds_arr = std::nullopt) noexcept
    : options_{ opts_arr }, commands_{ cmds_arr }, set_options_{ std::nullopt }, set_cmd_{ std::nullopt }
  {}

  /**
   * @brief Returns the number of defined options.
   *
   * @return std::size_t The number of defined options.
   */
  [[nodiscard]] constexpr std::size_t option_count() const
  {
    if constexpr (N_opts == 0) {
      return 0;
    } else {
      return options_.value().size();
    }
  }

  /**
   * @brief Returns the number of defined commands.
   *
   * @return std::size_t The number of defined commands.
   */
  [[nodiscard]] constexpr std::size_t command_count() const
  {
    if constexpr (N_cmds == 0) {
      return 0;
    } else {
      return commands_.value().size();
    }
  }

  /**
   * @brief Prints the names of the defined options to stdout.
   */
  void PrintOptions() const
  {
    if constexpr (N_opts == 0) {
      fmt::print("No options defined\n");
    } else {
      options_.value().print();
    }
  }

  /**
   * @brief Prints the names of the defined commands to stdout.
   */
  void PrintCommands() const
  {
    if constexpr (N_cmds == 0) {
      fmt::print("No commands defined\n");
    } else {
      commands_.value().print();
    }
  }

  /**
   * @brief Prints a short help text to stdout.
   *
   * @note This is usually invoked with an option equivelant to "-h" or "help" in most command-line apps.
   */
  void PrintShortHelp() const
  {
    spdlog::warn("TODO: Add description fields to Options and Commands");
    spdlog::warn("TODO: PrintAbout");
    spdlog::warn("TODO: PrintUsage");

    // Commands section
    fmt::print(fmt::emphasis::bold | fmt::emphasis::underline | fg(fmt::color::light_cyan), "Commands:\n");
    for (const clap::Command &cmd : this->commands_.value().cmds_) {
      fmt::print(fg(fmt::color::white), "  {}\n", cmd.name_);
    }
    fmt::print("\n");

    // Options section
    fmt::print(fmt::emphasis::bold | fmt::emphasis::underline | fg(fmt::color::light_yellow), "Options:\n");
    for (const clap::Option &opt : this->options_.value().opts_) {
      fmt::print(fg(fmt::color::white), "  {}\n", opt.name_);
    }
    fmt::print("\n");
  }

  // TODO: Long help

  /**
   * @brief Parse the command-line arguments.
   *
   * @param args The command-line arguments to parse.
   * @return size_t The number of arguments that failed validation (check that it's 0 to not run over errors)
   */
  [[nodiscard]] auto Parse(const std::vector<std::string_view> &args) noexcept -> size_t
  {
    if (this->is_clap_parsed) {
      return 1;
    } else {
      this->is_clap_parsed = true;
    }

    return this->parse_args(args);
  }


  /**
   * @brief Print the arguments that were set.
   *
   * @note This is useful for debugging.
   */
  void PrintArgs() const
  {
    if (this->set_cmd_.has_value()) {
      fmt::println("Set command: {}", this->set_cmd_.value().name_);
    } else {
      fmt::println("No command set");
    }
    if (this->set_options_.has_value()) {
      fmt::println("{} options set:\n", this->set_options_.value().size());
      for (const auto &opt : this->set_options_.value()) { fmt::print("\t{}\n", opt.first.name_); }
    } else {
      fmt::println("No options set");
    }
  }

  /**
   * @brief Lookup if a flag is set by supplying a partial or fully equivelant `Option` struct, and return if it is set.
   *
   * @param opt_inst A reference to the `Option` struct to lookup.
   * @return true If the flag is set.
   */
  [[nodiscard]] constexpr auto FlagSet(const clap::Option &opt_inst) const -> bool { return FlagSet(opt_inst.name_); }

  /**
   * @brief Lookup if a flag is set by name, return if it's set or not.
   *
   * @param flag_name The name of the flag.
   * @return true If the flag is set.
   */
  [[nodiscard]] constexpr auto FlagSet(std::string_view flag_name) const -> bool
  {
    if constexpr (N_opts == 0) {
      return false;
    } else if (!this->set_options_.has_value()) {
      return false;
    } else {
      auto res = std::find_if(
        this->set_options_.value().begin(), this->set_options_.value().end(), [flag_name](const auto &opt) {
          return opt.first.name_ == flag_name
                 || (opt.first.has_alias()
                     && std::any_of(opt.first.aliases_.value().begin(),
                       opt.first.aliases_.value().end(),
                       [flag_name](const auto &name) { return name.has_value() && name.value() == flag_name; }));
        });

      return static_cast<bool>(res != this->set_options_.value().end());
    }
  }


  /**
   * @brief Lookup option by supplying a partial or fully equivelant `Option` struct, and if it is set, get the value.
   *
   * @param opt_inst A reference to the `Option` struct to lookup.
   * @return `std::optional<std::string_view>` The value of the option if it is set, `std::nullopt` otherwise.
   */
  [[nodiscard]] auto OptionValue(const clap::Option &opt_inst) const -> std::optional<std::string_view>
  {
    return OptionValue(opt_inst.name_);// :)
  }

  /**
   * @brief Lookup option by name (or alias) and if it is set, get the value.
   *
   * @param opt_name The name of the option.
   * @return `std::optional<std::string_view>` The value of the option if it is set, `std::nullopt` otherwise.
   */
  [[nodiscard]] auto OptionValue(std::string_view opt_name) const -> std::optional<std::string_view>
  {
    if constexpr (N_opts == 0) {
      // If instantiated with 0 options
      return std::nullopt;
    } else if (!this->set_options_.has_value()) {
      // If no set options
      return std::nullopt;
    } else {
      // If set options, look for match

      auto res =
        std::find_if(this->set_options_.value().begin(), this->set_options_.value().end(), [opt_name](const auto &opt) {
          return opt.first.name_ == opt_name
                 || (opt.first.has_alias()
                     && std::any_of(opt.first.aliases_.value().begin(),
                       opt.first.aliases_.value().end(),
                       [opt_name](const auto &name) { return name.has_value() && name.value() == opt_name; }));
        });

      if (res != this->set_options_.value().end()) {
        if (!(*res).second.has_value()) {
          spdlog::warn("No value found for option: {}", (*res).first.name_);
          return std::nullopt;
        } else {
          return (*res).second.value();
        }
      } else {
        return std::nullopt;
      }
    }
  }

  /**
   * @brief Lookup command by supplying a partial or fully equivelant `Command` struct, and return if it is set.
   *
   * @param cmd_inst A reference to the `Command` struct to lookup.
   * @return true If the command is set.
   */
  [[nodiscard]] constexpr auto CmdSet(const clap::Command &cmd_inst) const -> bool { return CmdSet(cmd_inst.name_); }

  /**
   * @brief Lookup command by name and return if it is set.
   *
   * @param cmd_name The name of the command.
   * @return true If the command is set.
   */
  [[nodiscard]] constexpr auto CmdSet(std::string_view cmd_name) const -> bool
  {
    if constexpr (N_cmds == 0) {
      return false;
    } else {
      return (this->set_cmd_.has_value() && this->set_cmd_.value().name_ == cmd_name);
    }
  }

  /**
   * @brief Returns if the command-line arguments have been parsed.
   *
   * @note This can be used to make sure that the command-line arguments are parsed before accessing the set
   * options/commands.
   *
   * @return true If the command-line arguments have been parsed.
   */
  [[nodiscard]] auto isClapParsed() const noexcept -> bool { return this->is_clap_parsed; }

  // Private helpers
private:
  // Store the option and its value in the set_options_ vector
  void store_option_value(clap::Option opt, std::optional<std::string_view> opt_val)
  {
    if (!this->set_options_.has_value()) {
      this->set_options_ =
        std::vector<std::pair<clap::Option, std::optional<std::string_view>>>{ std::make_pair(opt, opt_val) };
    } else {
      this->set_options_.value().emplace_back(std::make_pair(opt, opt_val));
    }
  }

  // Find a command by name and move it to the set_cmd_ member variable to indicate that it is has been set from the
  // command-line.
  [[nodiscard]] auto find_command(std::string_view cmd) -> size_t
  {
    if (auto found_cmd = this->commands_.value().find(cmd)) {
      if (!this->set_cmd_.has_value()) {
        this->set_cmd_ = std::move(found_cmd);
      } else [[unlikely]] {
        spdlog::error(
          "Attempted setting command: '{}' when a command was already set: '{}'. Only one command is allowed",
          cmd,
          this->set_cmd_.value().name_);
        return 1;
      }
    }
    return 0;
  }


  // Parse the command-line arguments
  [[nodiscard]] auto parse_args(const std::vector<std::string_view> &args) noexcept -> size_t
  {
    size_t errors = 0;

    for (auto it = args.cbegin(), end = args.cend(); it != end; ++it) {
      if ((*it)[0] == '-') {
        // Find in option array
        if constexpr (N_opts == 0) {
          ++errors;
          spdlog::error("Got option '{}' but no options have been defined", *it);
        } else {
          if (std::optional<clap::Option> found_opt = this->options_.value().find(*it)) {
            // Check if it is not a flag, then the next argument should be the attached value
            std::optional<std::string_view> opt_value = std::nullopt;
            if (found_opt.value().opt_type_ == Opt::NeedValue) {
              // Then check for the value in the arguments
              if ((it + 1 == end) || (*(it + 1)).starts_with("-")) [[unlikely]] {
                ++errors;
                spdlog::error("Option {} passed with missing value", *it);
              } else {
                opt_value = *(it + 1);
                // Increment as we already validated the option value and we don't want to parse it as an option in
                // the next iteration
                ++it;
              }
            }
            this->store_option_value(found_opt.value(), opt_value);
          } else [[unlikely]] {
            // Provided option not found
            ++errors;
            spdlog::error("Unknown option '{}'", *it);
          }
        }

      } else {
        // Find in command array
        if constexpr (N_cmds == 0) {
          ++errors;
          spdlog::error("Got command '{}' but no commands have been defined", *it);
        } else {
          errors += this->find_command(*it);
        }
      }
    }

    return errors;
  }
};


}// namespace clap