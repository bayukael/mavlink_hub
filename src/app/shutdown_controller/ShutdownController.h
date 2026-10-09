#pragma once

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  /// Coordinates a cooperative shutdown between threads.
  ///
  /// A producer signals shutdown with requestShutdown; the main thread blocks in
  /// waitForShutdownSignal until that signal arrives. The internal flag is latched: multiple
  /// requests are safe, and waiting after a signal returns immediately.
  class ShutdownController
  {
  public:
    ShutdownController();
    ~ShutdownController();
    ShutdownController(ShutdownController&&) noexcept;
    ShutdownController& operator=(ShutdownController&&) noexcept;

    /// Set the shutdown flag and wake any thread waiting in waitForShutdownSignal.
    void requestShutdown();

    /// Block until requestShutdown is called. Returns immediately if already requested.
    void waitForShutdownSignal();

  private:
    struct ShutdownControllerImpl;
    std::unique_ptr<ShutdownControllerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
