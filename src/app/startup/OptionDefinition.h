#pragma once

#include <string>

namespace pendarlab::app::mavlink_hub::startup
{
  struct OptionDefinition {
    std::string identifier;
    int num_of_args;
    std::string msg_info;

    OptionDefinition(const std::string& id, int nargs, const std::string& info) : identifier(id), num_of_args(nargs), msg_info(info) {}
  };
} // namespace pendarlab::app::mavlink_hub::startup
