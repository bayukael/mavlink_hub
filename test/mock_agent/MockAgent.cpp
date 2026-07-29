#include "mock_agent/MockAgent.h"

namespace pendarlab::app::mavlink_hub::test
{
  MockAgent::MockAgent()
  {
  }
  MockAgent::~MockAgent()
  {
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