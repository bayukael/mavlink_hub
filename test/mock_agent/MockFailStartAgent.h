#pragma once

#include <mavlink_hub_sdk/agent/Agent.h>
#include <mavlink_hub_sdk/agent/AgentConfig.h>

namespace pendarlab::app::mavlink_hub::test
{
  /// A mock agent whose start() always fails (doStart returns false), used to
  /// exercise the plan execution path where start_on_create leads to a failed
  /// start.
  class MockFailStartAgent : public pendarlab::sdk::mavlink_hub::Agent
  {
    using AgentConfig = pendarlab::sdk::mavlink_hub::AgentConfig;

  public:
    MockFailStartAgent();
    virtual ~MockFailStartAgent();

    virtual std::string report() const override;

  protected:
    virtual bool doStart() override;
    virtual bool doStop() override;
    virtual bool doConfigure(const AgentConfig& config) override;
  };
} // namespace pendarlab::app::mavlink_hub::test
