#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning "discarding return value of function with 'nodiscard' attribute"
//  Because they warn on their own std::vector implementation, a warning that is discouraged by the standard...
#pragma warning(disable : 4834)
#endif


#include <fmt/core.h>
#include <internal_use_only/config.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <functional>
#include <string_view>
#include <utility>
#include <vector>

#include "clap_cpp/clap.hpp"
#include "clap_cpp/clap/command.hpp"

// Define the command-line options and commands
namespace config {
using clap::Opt::NeedValue;


// Define the command-line options
namespace option {
  constexpr clap::Option help{ "-h", clap::Opt::Flag, "--help" };
  constexpr clap::Option echo{ "--echo", clap::Opt::Flag };


  constexpr clap::OptionArray opt_array =
    clap::def_options(clap::Option("--version", clap::Opt::Flag, "-V"), help, echo);
}// namespace option

// Define the commands for the command-line parser
namespace commands {
  constexpr clap::Command run{ "run", true };
}

/**
 * @brief Singleton class that holds the Command-line argument parser (CLAP) for the example.
 *
 * Parses the command-line arguments and stores the results in a singleton config object.
 *
 * @note The singleton is initialized at compile-time and parsed at run-time.
 *
 */
class Config
{
public:
  /**
   * @brief Get the singleton config object.
   *
   * @return Config*
   */
  static auto get() -> decltype(auto)
  {
    static constinit auto config = clap::init_clap(option::opt_array, clap::def_cmds(commands::run));
    return &config;
  }
};

}// namespace config


using cfg = config::Config;


int main(int argc, char *argv[])
{
  std::vector<std::string_view> args{ argv + 1, argv + argc };
  if (auto errors = config::Config::get()->Parse(args)) [[unlikely]] {
    fmt::println("{} argument(s) failed to validate", errors);
    return -1;
  };

  if (cfg::get()->FlagSet(config::option::help)) [[unlikely]] {
    cfg::get()->PrintShortHelp();
    return 0;
  }

  if (cfg::get()->FlagSet(config::option::echo)) { cfg::get()->PrintArgs(); }

  if (cfg::get()->FlagSet("--version")) [[unlikely]] {
    fmt::println("v{}", clap_cpp::cmake::project_version);
    return 0;
  }

  return 0;
}
