#include "app/ui_handler/CliCommandDescriptor.h"

#include <utility>

namespace pendarlab::app::mavlink_hub
{
  CliCommandDescriptor::CliCommandDescriptor(const AppServiceCommandDescriptor& descriptor, std::string name)
      : type(descriptor.type),
        name(name.empty() ? std::string(descriptor.name) : std::move(name)),
        requires_payload(descriptor.requires_payload),
        payload_hint(std::string(descriptor.payload_hint)),
        description()
  {
  }

} // namespace pendarlab::app::mavlink_hub
