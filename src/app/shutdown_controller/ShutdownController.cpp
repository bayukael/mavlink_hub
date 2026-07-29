#include "app/shutdown_controller/ShutdownController.h"

#include <condition_variable>
#include <mutex>

namespace pendarlab::app::mavlink_hub
{
  struct ShutdownController::ShutdownControllerImpl {
    std::mutex mtx;
    std::condition_variable cv;
    bool shutdown_requested = false;
  };

  ShutdownController::ShutdownController() : d(std::make_unique<ShutdownControllerImpl>())
  {
  }
  ShutdownController::~ShutdownController()
  {
  }
  ShutdownController::ShutdownController(ShutdownController&&) noexcept = default;
  ShutdownController& ShutdownController::operator=(ShutdownController&&) noexcept = default;

  void ShutdownController::requestShutdown()
  {
    {
      std::lock_guard lock(d->mtx);
      d->shutdown_requested = true;
    }
    d->cv.notify_one();
  }

  void ShutdownController::waitForShutdownSignal()
  {
    std::unique_lock lock(d->mtx);
    d->cv.wait(lock, [this] { return d->shutdown_requested; });
  }

} // namespace pendarlab::app::mavlink_hub