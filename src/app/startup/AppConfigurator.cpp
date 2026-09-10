#include "app/startup/AppConfigurator.h"

#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"
#include "app/types/UserCommandType.h"
#include "common/json_utils/JsonUtils.h"

#include <fstream>
#include <optional>

namespace pendarlab::app::mavlink_hub::startup
{
  OperationResult applyConfig(const AppConfig& config, ILibLoader& lib_loader, IAppService& app_service)
  {
    OperationResult result;
    for (const auto& [name, lib_info] : config.agent_lib_list) {
      OperationResult load_result = lib_loader.loadAgentLib(lib_info);
      result.merge(load_result);
    }

    for (const auto& [name, lib_info] : config.transport_lib_list) {
      OperationResult load_result = lib_loader.loadTransportLib(lib_info);
      result.merge(load_result);
    }

    if (!config.path_to_extra_lib_list.empty()) {
      std::ifstream extra_libs_stream(config.path_to_extra_lib_list);
      if (!extra_libs_stream.is_open()) {
        OperationResult extra_libs_find_file;
        extra_libs_find_file.success = false;
        extra_libs_find_file.messages.push_back("[App]: path_to_extra_lib_list is set to [" + config.path_to_extra_lib_list +
                                                "], but it cannot be found.");
        result.merge(extra_libs_find_file);
        return result;
      }

      // We use fstreamToAppConfig just to get the library list part.
      std::optional<AppConfig> extra_libs_opt = json_utils::fstreamToAppConfig(extra_libs_stream);
      if (!extra_libs_opt.has_value()) {
        OperationResult extra_libs_parsing;
        extra_libs_parsing.success = false;
        extra_libs_parsing.messages.push_back("[App]: path_to_extra_lib_list is set to [" + config.path_to_extra_lib_list +
                                              "], but it cannot be parsed properly.");
        result.merge(extra_libs_parsing);
        return result;
      }
      AppConfig& extra_libs = extra_libs_opt.value();
      for (const auto& [name, lib_info] : extra_libs.agent_lib_list) {
        OperationResult load_result = lib_loader.loadAgentLib(lib_info);
        result.merge(load_result);
      }

      for (const auto& [name, lib_info] : extra_libs.transport_lib_list) {
        OperationResult load_result = lib_loader.loadTransportLib(lib_info);
        result.merge(load_result);
      }
    }

    // Process path_to_startup_user_plan
    if (!config.path_to_startup_user_plan.empty()) {
      // We can pass all of these to AppService.
      UserCommand cmd;
      cmd.cmd_type = UserCommandType::LOAD_PLAN_FROM_PATH;
      cmd.payload = config.path_to_startup_user_plan;
      CommandResult load_plan_cmd_result = app_service.executeCommand(cmd);
      OperationResult load_plan_op_result;
      load_plan_op_result.success = load_plan_cmd_result.success;
      load_plan_op_result.messages = load_plan_cmd_result.message;
      result.merge(load_plan_op_result);

      if (config.apply_user_plan_on_startup && load_plan_cmd_result.success) {
        cmd.cmd_type = UserCommandType::APPLY_CURRENT_PLAN;
        cmd.payload = ""; // The command does not need a payload.
        CommandResult apply_plan_cmd_result = app_service.executeCommand(cmd);
        OperationResult apply_plan_op_result;
        apply_plan_op_result.success = apply_plan_cmd_result.success;
        apply_plan_op_result.messages = apply_plan_cmd_result.message;
        result.merge(apply_plan_op_result);
      }
    }

    return result;
  }
} // namespace pendarlab::app::mavlink_hub::startup
