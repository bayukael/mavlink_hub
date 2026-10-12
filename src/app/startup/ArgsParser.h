#pragma once

#include "app/types/AppConfig.h"

#include <optional>

namespace pendarlab::app::mavlink_hub::startup
{
  /// Outcome of parsing command-line arguments.
  struct ParseResult {
    enum class Status {
      Ok,        ///< Arguments parsed successfully; config (if any) is set.
      ShowHelp,  ///< Help/version was requested; the caller should exit with exit_code.
      FatalError ///< Parsing failed; the caller should exit with exit_code.
    };
    Status status = Status::Ok;
    int exit_code = 0;               ///< Exit code to use for ShowHelp/FatalError.
    std::optional<AppConfig> config; ///< Parsed config (set only on Ok with a --config file).
  };

  /// Parse command-line arguments into an AppConfig.
  ///
  /// Recognizes a --config/--c option pointing at a JSON config file. If no config path is
  /// given, returns an empty AppConfig. Help output and hard errors are reported through the
  /// returned ParseResult rather than thrown.
  /// @return the parse outcome; inspect Status before using config.
  ParseResult parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
