#pragma once

#include <mavlink_hub_sdk/agent/Agent.h>
#include <mavlink_hub_sdk/agent/AgentConfig.h>

#include <mutex>

namespace pendarlab::app::mavlink_hub::test
{
  class MockAgent : public pendarlab::sdk::mavlink_hub::Agent
  {
    using AgentConfig = pendarlab::sdk::mavlink_hub::AgentConfig;

  public:
    MockAgent();
    virtual ~MockAgent();

    /// Simple, easily-checkable report payload prefix returned by report().
    /// report() appends an incrementing counter suffix so successive calls
    /// return observably different reports.
    static const std::string kReportPayload;

    /// Report counter wraps back to 1 after reaching this bound.
    static const unsigned int kReportCounterBound;

    virtual std::string report() const override;

  protected:
    virtual bool doStart() override;
    virtual bool doStop() override;
    virtual bool doConfigure(const AgentConfig& config) override;

  private:
    /// Monotonic counter appended to each report() return value. mutable so
    /// report() (declared const) can advance it; protected by a mutex because
    /// report() may be called concurrently from the CLI's report poll worker.
    mutable unsigned int report_counter_{ 0 };
    mutable std::mutex report_mutex_;
  };
} // namespace pendarlab::app::mavlink_hub::test