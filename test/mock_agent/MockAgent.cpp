#include "mock_agent/MockAgent.h"

#include <string>

namespace pendarlab::app::mavlink_hub::test
{
  const std::string MockAgent::kReportPayload = "MockAgent Report";
  const unsigned int MockAgent::kReportCounterBound = 1000;

  MockAgent::MockAgent()
  {
  }
  MockAgent::~MockAgent()
  {
  }

  std::string MockAgent::report() const
  {
    std::lock_guard<std::mutex> lock(report_mutex_);
    ++report_counter_;
    if (report_counter_ > kReportCounterBound) {
      report_counter_ = 1;
    }
    return kReportPayload + ": " + std::to_string(report_counter_);
  }

  bool MockAgent::doStart()
  {
    return true;
  }

  bool MockAgent::doStop()
  {
    return true;
  }

  bool MockAgent::doConfigure(const AgentConfig& config)
  {
    return true;
  }

} // namespace pendarlab::app::mavlink_hub::test