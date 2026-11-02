#include "mock_agent/MockFailStartAgentDefinition.h"

#include "mock_agent/MockFailStartAgent.h"

#include <memory>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub::test
{
  using AgentConfigParseResult = pendarlab::sdk::mavlink_hub::AgentConfigParseResult;
  using Agent = pendarlab::sdk::mavlink_hub::Agent;
  using AgentConfig = pendarlab::sdk::mavlink_hub::AgentConfig;
  using IManagerResourceRequester = pendarlab::sdk::mavlink_hub::IManagerResourceRequester;

  AgentConfigParseResult MockFailStartAgentDefinition::parseConfig(const std::unordered_map<std::string, std::string>& cfg) const
  {
    AgentConfigParseResult result;
    std::string required_entry("a_required_string");
    auto it = cfg.find(required_entry);
    if (it == cfg.end()) {
      result.parsed = nullptr;
      result.messages.push_back("[MockFailStartAgent] : a required entry is missing: " + required_entry);
    } else {
      result.parsed = std::make_unique<AgentConfig>();
    }

    return result;
  }

  std::unique_ptr<Agent> MockFailStartAgentDefinition::create(const AgentConfig& cfg, std::unique_ptr<IManagerResourceRequester> requester) const
  {
    return std::make_unique<MockFailStartAgent>();
  }

} // namespace pendarlab::app::mavlink_hub::test
