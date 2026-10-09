#pragma once

#include "app/app_service/IAppService.h"
#include "common/types/OperationResult.h"

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  /// Full-screen FTXUI-based view that renders the CliUiController.
  ///
  /// Owns a CliUiController and runs a UI thread that blocks on the FTXUI event loop. It
  /// binds the controller's state to the rendered component tree and wires up the redraw
  /// callback so completed commands refresh the UI automatically.
  class CliUiHandler
  {
  public:
    /// @param appsrv the service backing the UI's controller. Must outlive this handler.
    CliUiHandler(IAppService& appsrv);
    ~CliUiHandler();
    CliUiHandler(CliUiHandler&&) noexcept;
    CliUiHandler& operator=(CliUiHandler&&) noexcept;

    /// Launch the UI on a background thread. No-op if already running.
    void start();

    /// Tear down the UI, waiting for any in-flight command to finish first. No-op if not
    /// running. Called automatically by the destructor.
    void stop();

    /// @return the handler's result after a UI session (currently always a default/empty
    ///         OperationResult).
    OperationResult getResult();

  private:
    struct CliUiHandlerImpl;
    std::unique_ptr<CliUiHandlerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
