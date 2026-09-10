#pragma once

#include <string>

namespace pendarlab::app::mavlink_hub
{
  struct LibInfo{
    std::string name;
    std::string path;
    std::string sym;
  };
} // namespace pendarlab::app::mavlink_hub