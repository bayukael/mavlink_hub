#pragma once

#include "common/types/OperationResult.h"

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  class App
  {
  public:
    App(int argc, char** argv);
    ~App();
    App(App&&) noexcept;
    App& operator=(App&&) noexcept;

    OperationResult run();

  private:
    struct AppImpl;
    std::unique_ptr<AppImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub