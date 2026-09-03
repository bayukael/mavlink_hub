#include "app/startup/ArgsParser.h"

#include "common/json_utils/JsonUtils.h"

#include <cli11/CLI11.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>

namespace pendarlab::app::mavlink_hub
{
  std::optional<StartupIntent> startup::parseArgs(int argc, char** argv)
  {
    StartupIntent intent;
    CLI::App app{
      "mavlink_hub is a tool that connects endpoints that uses Mavlink to agents (not AI agent!) so that agents can send and receive "
      "mavlink messages via endpoints."
    };

    argv = app.ensure_utf8(argv);
    app.add_option("-c,--config", intent.config_file_path, "Specifies which config file to use. Config file should be written in JSON");
    try {
      app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
      return std::nullopt;
    }
    return intent;
  }

} // namespace pendarlab::app::mavlink_hub
