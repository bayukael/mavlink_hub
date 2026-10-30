#include "mock_agent/MockAgent.h"

namespace pendarlab::app::mavlink_hub::test
{
  const std::string MockAgent::kReportPayload = "MockAgent Report";

  MockAgent::MockAgent()
  {
  }
  MockAgent::~MockAgent()
  {
  }

  std::string MockAgent::report() const
  {
    return kReportPayload;
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