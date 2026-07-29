#pragma once

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  class ShutdownController
  {
  public:
    ShutdownController();
    ~ShutdownController();
    ShutdownController(ShutdownController&&) noexcept;
    ShutdownController& operator=(ShutdownController&&) noexcept;
    void requestShutdown();
    void waitForShutdownSignal();

  private:
    struct ShutdownControllerImpl;
    std::unique_ptr<ShutdownControllerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
