#include "app/startup/ArgsParser.h"

#include "common/json_utils/JsonUtils.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  std::unordered_map<std::string, std::string> startup::parseArgs(int argc, char** argv)
  {
    /*
    REWORK THIS:
    - parseArgs should only parse into unordered_map<string, string>
    - make another function called executeArgs that receives the output of parseArgs.
    - executeArgs returns optional<StartupIntent>
    */
    // StartupIntent result;
    // if (argc > 1) {
    //   // Loop through each argument (starting from the first argument after the program name)
    //   for (int i = 1; i < argc; ++i) {
    //     /*
    //     For now, it is just a simple iterating through the arguments and immediately executing them.
    //     Next, we might want to separate between parsing and execution.
    //     */

    //     std::string arg = argv[i];
    //     // Check if the argument matches "-h"
    //     if (arg == "-h") {
    //       // show help
    //       return std::nullopt;
    //     }
    //     // Check if the argument matches "-f"
    //     else if (arg == "-f") {
    //       // Check if there is another argument after "-f"
    //       if (i + 1 < argc) {
    //         std::error_code ec;
    //         std::filesystem::path startup_config_path = std::filesystem::canonical(argv[i + 1], ec);
    //         if (ec) { // wrong filepath
    //           return std::nullopt;
    //         }
    //         std::ifstream startup_config_stream(startup_config_path.c_str());
    //         std::optional<StartupIntent> parsed_intent = json_utils::fstreamToStartupIntent(startup_config_stream);
    //         if (!parsed_intent.has_value()) { // intent cannot be parsed successfully
    //           return std::nullopt;
    //         }
    //         result = parsed_intent.value();
    //         ++i; // Skip the next argument since it's the value for "-f"
    //       } else {
    //         return std::nullopt;
    //         break;
    //       }
    //     } else {
    //       return std::nullopt;
    //       // unknown option
    //     }
    //   }
    // } else { // No arguments = use default value
    //   // Default value
    // }
    // return result;
    return std::unordered_map<std::string, std::string>();
  }

} // namespace pendarlab::app::mavlink_hub
