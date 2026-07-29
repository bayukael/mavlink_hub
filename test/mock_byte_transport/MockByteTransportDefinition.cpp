#include "mock_byte_transport/MockByteTransportDefinition.h"
#include "mock_byte_transport/MockByteTransport.h"

#include <memory>

namespace pendarlab::app::mavlink_hub::test
{
  using Config = pendarlab::lib::comm::byte_transport::Config;
  using ConfigParseResult = pendarlab::lib::comm::byte_transport::ConfigParseResult;
  using Transport = pendarlab::lib::comm::byte_transport::Transport;

  ConfigParseResult MockByteTransportDefinition::parseConfig(const std::unordered_map<std::string, std::string>& cfg) const
  {
    ConfigParseResult result;
    result.config = Config();
    return result;
  }

  std::unique_ptr<Transport> MockByteTransportDefinition::create(const Config& cfg) const
  {
    return std::make_unique<MockByteTransport>();
  }
} // namespace pendarlab::app::mavlink_hub::test