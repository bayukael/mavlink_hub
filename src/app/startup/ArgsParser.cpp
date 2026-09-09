#include "app/startup/ArgsParser.h"

#include "app/types/AppConfig.h"
#include "common/json_utils/JsonUtils.h"

#include <cli11/CLI11.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>

namespace pendarlab::app::mavlink_hub::startup
{
  std::optional<AppConfig> parseArgs(int argc, char** argv)
  {
    std::string config_file_path;
    CLI::App app{
      "mavlink_hub is a tool that connects endpoints that uses Mavlink to agents (not AI agent!) so that agents can send and receive "
      "mavlink messages via endpoints."
    };

    argv = app.ensure_utf8(argv);
    app.add_option("-c,--config", config_file_path, "Specifies which config file to use. Config file should be written in JSON");

    try {
      app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
      return std::nullopt;
    }

    AppConfig app_cfg;
    if(!config_file_path.empty()){
      std::error_code ec;
      std::string canonical_config_file_path = std::filesystem::canonical(config_file_path, ec);
      if(ec){
        return std::nullopt;
      }

      std::ifstream cfg_stream(canonical_config_file_path);
      if (!cfg_stream.is_open()) {
        return std::nullopt;
      }

      std::optional<AppConfig> processed_cfg = json_utils::fstreamToAppConfig(cfg_stream);
      if (!processed_cfg.has_value()) {
        return std::nullopt;
      }
      app_cfg = processed_cfg.value();
    }
    return app_cfg;
  }

} // namespace pendarlab::app::mavlink_hub::startup
