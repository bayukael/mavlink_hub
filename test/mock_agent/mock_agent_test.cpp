#include "mock_agent/MockAgent.h"

#include "gtest/gtest.h"

#include <string>

using MockAgent = pendarlab::app::mavlink_hub::test::MockAgent;

namespace
{
  // Every report() return value must contain the fixed payload plus a counter
  // suffix, so the fixed part stays stable while the full string changes.
  bool reportHasFixedPrefix(const std::string& report)
  {
    return report.rfind(MockAgent::kReportPayload, 0) == 0u;
  }
} // namespace

// The report changes across consecutive calls: repeated calls return
// observably different reports (MockAgent Report: 1, ...: 2, ...), which is
// what lets the CLI's periodic report poll be validated as actually re-fetching
// fresh data.
TEST(MockAgentReportTest, ConsecutiveCallsReturnDifferentReports)
{
  MockAgent agent;

  const std::string first = agent.report();
  const std::string second = agent.report();
  const std::string third = agent.report();

  EXPECT_NE(first, second);
  EXPECT_NE(second, third);
  EXPECT_NE(first, third);
}

// Every report() return value still carries the fixed payload prefix, so
// consumers that only inspect the constant part (e.g. prefix match) keep
// passing regardless of the current counter value.
TEST(MockAgentReportTest, EveryReportRetainsFixedPayloadPrefix)
{
  MockAgent agent;

  for (int i = 0; i < 100; ++i) {
    EXPECT_TRUE(reportHasFixedPrefix(agent.report()));
  }
}

// The counter starts at 1 on the first call, increments by 1 on each call, and
// wraps back to 1 after reaching the configured bound.
TEST(MockAgentReportTest, CounterStartsAtOneAndWrapsAtBound)
{
  MockAgent agent;

  EXPECT_EQ(agent.report(), MockAgent::kReportPayload + ": 1");
  EXPECT_EQ(agent.report(), MockAgent::kReportPayload + ": 2");

  // Drain the counter up to (but not past) the bound: after the two calls above
  // the counter is 2, so `bound - 3` more calls bring it to `bound - 1`.
  const unsigned int calls_to_just_below_bound = MockAgent::kReportCounterBound - 3;
  for (unsigned int i = 0; i < calls_to_just_below_bound; ++i) {
    agent.report();
  }
  // Counter is now bound-1, so this call returns exactly the bound value.
  EXPECT_EQ(agent.report(), MockAgent::kReportPayload + ": " + std::to_string(MockAgent::kReportCounterBound));
  // The next call exceeds the bound and wraps back to 1.
  EXPECT_EQ(agent.report(), MockAgent::kReportPayload + ": 1");
  EXPECT_EQ(agent.report(), MockAgent::kReportPayload + ": 2");
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
