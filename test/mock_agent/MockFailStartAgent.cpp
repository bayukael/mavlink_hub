#include "mock_agent/MockFailStartAgent.h"

namespace pendarlab::app::mavlink_hub::test
{
  MockFailStartAgent::MockFailStartAgent()
  {
  }
  MockFailStartAgent::~MockFailStartAgent()
  {
  }

  std::string MockFailStartAgent::report() const
  {
    return "MockFailStartAgent Report";
  }

  bool MockFailStartAgent::doStart()
  {
    return false;
  }

  bool MockFailStartAgent::doStop()
  {
    return true;
  }

  bool MockFailStartAgent::doConfigure(const AgentConfig& config)
  {
    return true;
  }

} // namespace pendarlab::app::mavlink_hub::test
