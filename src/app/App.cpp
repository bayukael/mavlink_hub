#include "app/App.h"

#include "agent_registry/AgentRegistry.h"
#include "agent_registry/AgentRegistryUserAccess.h"
#include "app/app_service/AppService.h"
#include "app/lib_loader/LibLoader.h"
#include "app/shutdown_controller/ShutdownController.h"
#include "app/startup/ArgsParser.h"
#include "app/types/AppConfig.h"
#include "app/types/LibList.h"
#include "app/ui_handler/CliUiHandler.h"
#include "common/json_utils/JsonUtils.h"
#include "common/types/OperationResult.h"
#include "manager/Manager.h"

#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUser.h>
#include <fstream>
#include <memory>
#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  using TransportRegistry = pendarlab::lib::comm::byte_transport::Registry;
  using TransportRegistryUserAccess = pendarlab::lib::comm::byte_transport::RegistryUserAccess;

  struct App::AppImpl {
    AppImpl(int argc, char** argv);
    int argc;
    char** argv;
    AgentRegistry agent_registry;
    std::unique_ptr<AgentRegistryUserAccess> agent_registry_user;
    TransportRegistry transport_registry;
    std::unique_ptr<TransportRegistryUserAccess> transport_registry_user;
    LibLoader lib_loader;
    OperationResult app_result;
    ShutdownController shutdown_controller;
    Manager manager;
    AppService app_service;
    CliUiHandler cli_handler;
    const startup::Intent default_intent;

    std::optional<AppConfig> processStartupIntent(const startup::Intent& intent);
    OperationResult configure(const AppConfig& config);
  };

  App::AppImpl::AppImpl(int c, char** v) :
      argc(c),
      argv(v),
      agent_registry_user(std::move(agent_registry.createUser())),
      transport_registry_user(std::move(transport_registry.createUser())),
      lib_loader(agent_registry, transport_registry),
      manager(*agent_registry_user, *transport_registry_user),
      app_service(manager, lib_loader),
      cli_handler(app_service)
  {
  }

  std::optional<AppConfig> App::AppImpl::processStartupIntent(const startup::Intent& intent)
  {
    AppConfig cfg;
    if (intent.config_file_path != default_intent.config_file_path) {
      std::ifstream cfg_stream(intent.config_file_path);
      if (!cfg_stream.is_open()) {
        return std::nullopt;
      }

      std::optional<AppConfig> processed_cfg = json_utils::fstreamToAppConfig(cfg_stream);
      if (!processed_cfg.has_value()) {
        return std::nullopt;
      }
      cfg = processed_cfg.value();
    }
    return cfg;
  }

  OperationResult App::AppImpl::configure(const AppConfig& config)
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

  App::App(int argc, char** argv) : d(std::make_unique<AppImpl>(argc, argv))
  {
  }

  App::~App()
  {
  }

  App::App(App&&) noexcept = default;
  App& App::operator=(App&&) noexcept = default;

  OperationResult App::run()
  {
    OperationResult app_result;

    std::optional<startup::Intent> intent = startup::parseArgs(d->argc, d->argv, d->default_intent);
    if (!intent.has_value()) {
      app_result.success = false;
      app_result.messages.push_back("[App]: Error in parsing arguments");
      return app_result;
    }

    std::optional<AppConfig> app_config_opt = d->processStartupIntent(intent.value());
    if (!app_config_opt.has_value()) {
      app_result.success = false;
      app_result.messages.push_back("[App]: Failed to parse the given config");
      return app_result;
    }
    AppConfig& app_config = app_config_opt.value();

    OperationResult configure_result = d->configure(app_config);
    // Get AppConfig contained in StartupIntent
    // Iterate over files contained in agent_libs_dir_path in AppConfig, load libs using AppService
    // Do the same for transport_libs_dir_path
    // Remark: Now, agent_registry and transport_registry have contained all the default libs via LibLoader through AppService

    // Is there a UserPlan in StartupIntent?
    // --> If yes, pass the UserPlan into Manager via AppService

    d->cli_handler.start();
    // HttpRequestHandler also?

    d->shutdown_controller.waitForShutdownSignal();

    d->cli_handler.stop();
    // HttpRequestHandler also?

    app_result.merge(d->cli_handler.getResult());
    // HttpRequestHandler also?

    return app_result;
  }
} // namespace pendarlab::app::mavlink_hub
