#include "app/App.h"

#include "agent_registry/AgentRegistry.h"
#include "agent_registry/AgentRegistryUserAccess.h"
#include "app/app_service/AppService.h"
#include "app/lib_loader/LibLoader.h"
#include "app/shutdown_controller/ShutdownController.h"
#include "app/signal_handler/SignalHandler.h"
#include "app/startup/AppConfigurator.h"
#include "app/startup/ArgsParser.h"
#include "app/ui_handler/CliUiHandler.h"
#include "manager/Manager.h"

#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUser.h>
#include <csignal>
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
    ShutdownController shutdown_controller;
    Manager manager;
    AppService app_service;
    CliUiHandler cli_handler;
    std::unique_ptr<SignalHandler> signal_handler;
  };

  App::AppImpl::AppImpl(int c, char** v) :
      argc(c),
      argv(v),
      agent_registry_user(std::move(agent_registry.createUser())),
      transport_registry_user(std::move(transport_registry.createUser())),
      lib_loader(agent_registry, transport_registry),
      manager(*agent_registry_user, *transport_registry_user),
      app_service(manager, lib_loader),
      cli_handler(app_service, shutdown_controller),
      signal_handler(std::make_unique<SignalHandler>(shutdown_controller, std::vector<int>{ SIGINT, SIGTERM }))
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

  int App::run()
  {
    startup::ParseResult parse_result = startup::parseArgs(d->argc, d->argv);
    if (parse_result.status == startup::ParseResult::Status::ShowHelp) {
      return parse_result.exit_code;
    }
    if (parse_result.status == startup::ParseResult::Status::FatalError) {
      return parse_result.exit_code;
    }

    startup::applyConfig(parse_result.config.value(), d->lib_loader, d->app_service);

    // Take ownership of SIGINT/SIGTERM BEFORE the UI thread starts: this blocks
    // those signals in this thread (inherited by the UI thread) and runs a
    // dedicated sigwait() thread that calls requestShutdown(), so a signal
    // triggers App::run()'s normal teardown instead of FTXUI's re-raise-and-die
    // path. See SignalHandler docs.
    d->signal_handler->start();

    d->cli_handler.start();
    // HttpRequestHandler also?

    d->shutdown_controller.waitForShutdownSignal();

    d->cli_handler.stop();
    // HttpRequestHandler also?

    d->signal_handler->stop();

    return 0;
  }
} // namespace pendarlab::app::mavlink_hub
