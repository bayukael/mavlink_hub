#pragma once

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <mavlink_hub_sdk/manager_resource_requester/IManagerResourceRequester.h>
#include <memory>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub::test
{
  class MockAgentDefinition : public pendarlab::sdk::mavlink_hub::AgentDefinition
  {
    using AgentConfigParseResult = pendarlab::sdk::mavlink_hub::AgentConfigParseResult;
    using Agent = pendarlab::sdk::mavlink_hub::Agent;
    using AgentConfig = pendarlab::sdk::mavlink_hub::AgentConfig;
    using IManagerResourceRequester = pendarlab::sdk::mavlink_hub::IManagerResourceRequester;

  public:
    virtual AgentConfigParseResult parseConfig(const std::unordered_map<std::string, std::string>& cfg) const override;
    virtual std::unique_ptr<Agent> create(const AgentConfig& cfg, std::unique_ptr<IManagerResourceRequester> requester) const override;
  };
} // namespace pendarlab::app::mavlink_hub::test