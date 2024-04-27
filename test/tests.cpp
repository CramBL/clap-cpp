// NOLINTBEGIN

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <clap_cpp/clap.hpp>
#include <fmt/core.h>


#include <regex>
#include <string_view>
#include <utility>

using Catch::Matchers::ContainsSubstring;

using clap::Opt::Flag;
using clap::Opt::NeedValue;

constinit auto static_clap = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", Flag)));


TEST_CASE("Test basic CLAP")
{
  char argv0[] = "example_bin";
  char argv1[] = "--version";

  char *argv[] = { argv0, argv1 };
  int argc = 2;

  std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };

  SECTION("Dynamically initialized - Show version")
  {
    auto clap = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", Flag)));
    fmt::print("Options are:\n");
    clap.PrintOptions();

    CHECK(clap.Parse(arg_vec) == 0);
    fmt::print("Arguments are:\n");
    clap.PrintArgs();
  }

  SECTION("Static initialized - show version")
  {
    fmt::print("Parsing arguments with constinit Clap\n");
    CHECK(static_clap.Parse(arg_vec) == 0);
    fmt::print("Arguments are:\n");
    static_clap.PrintArgs();
  }

  SECTION("Alias version cmd - Show version")
  {
    auto clap_alias_version = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", Flag, "-V")));

    CHECK(clap_alias_version.Parse(arg_vec) == 0);

    fmt::print("Arguments are:\n");
    clap_alias_version.PrintArgs();

    CHECK(clap_alias_version.FlagSet("--version"));
    CHECK(clap_alias_version.FlagSet("-V"));
  }
}

TEST_CASE("Test CLAP with options and values")
{
  char argv0[] = "example_bin";
  char arg_version[] = "--version";
  char arg_save_as[] = "--save-as";
  char arg_save_as_val[] = "saved.txt";

  SECTION("test save as option value")
  {
    char *argv[] = { argv0, arg_save_as, arg_save_as_val };
    int argc = 3;

    std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };

    auto clap = clap::Clap<2, 0>(
      clap::OptionArray<2>(clap::Option("--version", Flag, "-V"), clap::Option("--save-as", NeedValue, "-s")));


    CHECK(clap.Parse(arg_vec) == 0);
    fmt::print("Got args:\n");
    clap.PrintArgs();

    CHECK(clap.OptionValue("--save-as").value() == arg_save_as_val);
    CHECK(clap.OptionValue("-s").value() == arg_save_as_val);
    CHECK(clap.FlagSet("--version") == false);
    CHECK(clap.FlagSet("-V") == false);
  }

  SECTION("Argument validation catches errors")
  {
    constexpr auto version_option = clap::Option("--version", Flag, "-V");
    constexpr auto save_as_option = clap::Option("--save-as", NeedValue, "-s");
    constexpr auto opt_arr = clap::OptionArray<2>(version_option, save_as_option);
    auto clap = clap::Clap<2, 0>(opt_arr);


    SECTION("Missing option value - end of args")
    {
      char *argv[] = { argv0, arg_save_as };
      int argc = 2;
      std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };


      fmt::print("Got args:\n");
      fmt::print("Should fail as --save-as doesn't have a value provided\n");
      CHECK(clap.Parse(arg_vec) != 0);
    }

    SECTION("Missing option value - next option instead of value")
    {
      char *argv[] = { argv0, arg_save_as, arg_version };
      int argc = 3;
      std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };

      fmt::print("Got args:\n");
      fmt::print(
        "Should fail as --save-as doesn't have a value provided, instead it's followed by the --version option\n");
      CHECK(clap.Parse(arg_vec) != 0);
    }
  }
}

TEST_CASE("Command struct")
{
  // Command with no aliases
  constexpr clap::Command cmd0{ "my-cmd", false };
  CHECK(cmd0.name_ == "my-cmd");
  CHECK(cmd0.is_flag_ == false);

  // with alias
  constexpr clap::Command cmd1{ "my-cmd1", false };
  CHECK(cmd1.name_ == "my-cmd1");
  CHECK(cmd1.is_flag_ == false);

  // With multiple aliases
  constexpr clap::Command cmd2{ "my-cmd2", true };
  CHECK(cmd2.name_ == "my-cmd2");
  CHECK(cmd2.is_flag_ == true);

  // They can fit in same cmd array
  constexpr std::array<clap::Command, 3> cmd_arr = { cmd0, cmd1, cmd2 };
  REQUIRE(cmd_arr.at(0).name_ == cmd0.name_);
  CHECK(cmd0.is_flag_ == false);

  REQUIRE(cmd_arr.at(2).name_ == "my-cmd2");
  REQUIRE(cmd_arr.at(2).is_flag_ == true);

  constexpr clap::CommandArray<3> my_cmd_arr{ cmd0, cmd1, cmd2 };
  constexpr auto arr_sz = my_cmd_arr.size();// Circumvent CPP check warning: [knownConditionTrueFalse]
  REQUIRE(arr_sz == 3);
  CHECK(my_cmd_arr.find("my-cmd2").has_value());
  CHECK(my_cmd_arr.find("my-cmd1").value().name_ == "my-cmd1");
  CHECK(my_cmd_arr.find("my-cmd1").value().is_flag_ == false);
}

TEST_CASE("Option struct")
{
  constexpr clap::Option opt{ "--my-option", Flag };
  constexpr clap::Option opt_w_alias("--my-option", Flag, "--my-alias");

  constexpr bool opt_has_alias = opt.has_alias();
  REQUIRE(opt_has_alias == false);

  constexpr bool opt_w_alias_has_alias = opt_w_alias.has_alias();
  REQUIRE(opt_w_alias_has_alias == true);

  constexpr clap::OptionArray<2> opt_arr{ opt, opt_w_alias };

  constexpr auto arr_sz = opt_arr.size();
  CHECK(arr_sz == 2);

  CHECK(opt_arr.find("--my-option").has_value() == true);
  CHECK(opt_arr.find("--my-alias").has_value() == true);

  auto found_opt = opt_arr.find("--my-alias");
  REQUIRE(found_opt.has_value() == true);
  CHECK(found_opt.value().name_ == "--my-option");
}

// NOLINTEND