#pragma once

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  /// Application entry point that wires together and runs the mavlink_hub.
  ///
  /// Owns all core components (registries, lib loader, manager, app service, CLI UI) via a
  /// PIMPL handle. Construction parses nothing; startup arguments are consumed by run().
  class App
  {
  public:
    /// @param argc argument count from main.
    /// @param argv argument vector from main.
    App(int argc, char** argv);
    ~App();
    App(App&&) noexcept;
    App& operator=(App&&) noexcept;

    /// Parse startup arguments, apply the startup config, run the CLI UI, and block until a
    /// shutdown signal is received.
    /// @return 0 on clean shutdown, non-zero if startup failed (e.g. bad arguments or config).
    int run();

  private:
    struct AppImpl;
    std::unique_ptr<AppImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub