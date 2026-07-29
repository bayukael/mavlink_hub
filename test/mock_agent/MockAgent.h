#pragma once

#include <mavlink_hub_sdk/agent/Agent.h>
#include <mavlink_hub_sdk/agent/AgentConfig.h>

namespace pendarlab::app::mavlink_hub::test
{
  class MockAgent : public pendarlab::sdk::mavlink_hub::Agent
  {
    using AgentConfig = pendarlab::sdk::mavlink_hub::AgentConfig;

  public:
    MockAgent();
    virtual ~MockAgent();

  protected:
    virtual bool doStart() override;
    virtual bool doStop() override;
    virtual bool doConfigure(const AgentConfig& config) override;
  };
} // namespace pendarlab::app::mavlink_hub::test