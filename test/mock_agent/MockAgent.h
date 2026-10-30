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

    /// Simple, easily-checkable report payload returned verbatim by report().
    static const std::string kReportPayload;

    virtual std::string report() const override;

  protected:
    virtual bool doStart() override;
    virtual bool doStop() override;
    virtual bool doConfigure(const AgentConfig& config) override;
  };
} // namespace pendarlab::app::mavlink_hub::test