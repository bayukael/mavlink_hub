#include "app/App.h"

#include "agent_registry/AgentRegistry.h"
#include "agent_registry/AgentRegistryUserAccess.h"
#include "app/app_service/AppService.h"
#include "app/lib_loader/LibLoader.h"
#include "app/shutdown_controller/ShutdownController.h"
#include "app/startup/StartupArgsParser.h"
#include "app/ui_handler/CliUiHandler.h"
#include "common/types/OperationResult.h"
#include "manager/Manager.h"

#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUser.h>
#include <memory>

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

    // startup::parseArgs(argc, argv) -> StartupIntent (StartupIntent struct)
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
