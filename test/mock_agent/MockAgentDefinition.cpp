#include "mock_agent/MockAgentDefinition.h"

#include "mock_agent/MockAgent.h"

#include <memory>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub::test
{
  using AgentConfigParseResult = pendarlab::sdk::mavlink_hub::AgentConfigParseResult;
  using Agent = pendarlab::sdk::mavlink_hub::Agent;
  using AgentConfig = pendarlab::sdk::mavlink_hub::AgentConfig;
  using IManagerResourceRequester = pendarlab::sdk::mavlink_hub::IManagerResourceRequester;

  AgentConfigParseResult MockAgentDefinition::parseConfig(const std::unordered_map<std::string, std::string>& cfg) const
  {
    AgentConfigParseResult result;
    result.parsed = AgentConfig();
    return result;
  }

  std::unique_ptr<Agent> MockAgentDefinition::create(const AgentConfig& cfg, std::unique_ptr<IManagerResourceRequester> requester) const
  {
    return std::make_unique<MockAgent>();
  }

} // namespace pendarlab::app::mavlink_hub::test