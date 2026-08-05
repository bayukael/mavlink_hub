#include "mock_byte_transport/MockByteTransportDefinition.h"

#include "mock_byte_transport/MockByteTransport.h"

#include <memory>
#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub::test
{
  using Config = pendarlab::lib::comm::byte_transport::Config;
  using ConfigParseResult = pendarlab::lib::comm::byte_transport::ConfigParseResult;
  using Transport = pendarlab::lib::comm::byte_transport::Transport;

  ConfigParseResult MockByteTransportDefinition::parseConfig(const std::unordered_map<std::string, std::string>& cfg) const
  {
    ConfigParseResult result;
    std::string required_entry("a_required_string");
    auto it = cfg.find(required_entry);
    if (it == cfg.end()) {
      result.config = std::nullopt;
      result.messages.push_back("[MockByteTransport] : a required entry is missing: " + required_entry);
    } else {
      result.config = Config();
    }

    return result;
  }

  std::unique_ptr<Transport> MockByteTransportDefinition::create(const Config& cfg) const
  {
    return std::make_unique<MockByteTransport>();
  }
} // namespace pendarlab::app::mavlink_hub::test