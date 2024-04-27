#pragma once

#include "clap/command.hpp"
#include "clap/option.hpp"
#include "clap/parser.hpp"

// Command-Line Argument Parsing (CLAP) utility
namespace clap {

/**
 * @brief Helper function to define options for the Command-Line Argument Parser.
 *
 * @tparam Opts A parameter pack of `clap::Option` objects.
 * @param opts The options.
 * @return `clap::OptionArray` with the defined options.
 *
 * @note This function is only needed to avoid having to specify the template parameters.
 */
template<class... Opts> constexpr auto def_options(Opts... opts) noexcept -> decltype(auto)
{
  return clap::OptionArray<sizeof...(opts)>{ opts... };
}

/**
 * @brief Helper function to define commands for the Command-Line Argument Parser.
 *
 * @tparam Cs A parameter pack of `clap::Command` objects.
 * @param cmds The commands.
 * @return `clap::CommandArray` with the defined commands.
 *
 * @note This function is only needed to avoid having to specify the template parameters.
 */
template<class... Cs> constexpr auto def_cmds(Cs... cmds) noexcept -> decltype(auto)
{
  return clap::CommandArray<sizeof...(cmds)>{ cmds... };
}

/**
 * @brief Helper function to initialize a `clap::Clap` object.
 *
 * @tparam N The number of options.
 * @tparam M The number of commands.
 * @param options_arr A `clap::OptionArray` with the defined options.
 * @param cmd_arr A `clap::CommandArray` with the defined commands.
 * @return `clap::Clap<N, M>` with the defined options and commands.
 *
 * @note This function is only needed to avoid having to specify the template parameters.
 */
template<size_t N, size_t M>
[[nodiscard]] constexpr auto init_clap(clap::OptionArray<N> options_arr, clap::CommandArray<M> cmd_arr) noexcept
  -> clap::Clap<N, M>
{
  return clap::Clap<N, M>{ options_arr, cmd_arr };
}

}// namespace clap
