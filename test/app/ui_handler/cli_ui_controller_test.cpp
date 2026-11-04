#include "app/ui_handler/CliUiController.h"
#include "mock_app_service/MockAppService.h"

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>

using pendarlab::app::mavlink_hub::CliUiController;
using pendarlab::app::mavlink_hub::AppServiceCommandDescriptor;
using pendarlab::app::mavlink_hub::AppServiceCommandType;
using pendarlab::app::mavlink_hub::test::MockAppService;

namespace
{
  constexpr std::chrono::milliseconds kSlowDelay{ 50 };
}

class CliUiControllerTest : public testing::Test
{
protected:
  void SetUp() override { controller = std::make_unique<CliUiController>(app_service); }

  MockAppService app_service;
  std::unique_ptr<CliUiController> controller;
};

TEST_F(CliUiControllerTest, LoadsCommandDescriptorsFromService)
{
  ASSERT_GT(controller->commandDescriptors().size(), 0u);
  ASSERT_EQ(controller->commandDescriptors().size(), controller->commandEntries().size());
  EXPECT_EQ(controller->commandDescriptors()[0].type, AppServiceCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(controller->commandEntries()[0], std::string(controller->commandDescriptors()[0].name));
}

TEST_F(CliUiControllerTest, NoCommandCommittedDispatchesNothing)
{
  EXPECT_EQ(controller->committedCommand(), -1);
  controller->executeCurrentCommand();
  controller->joinExecution();
  EXPECT_EQ(app_service.commands.empty(), true);
}

TEST_F(CliUiControllerTest, CommittedCommandWithOutOfRangeIndexDispatchesNothing)
{
  controller->commitCommand(12345);
  controller->executeCurrentCommand();
  controller->joinExecution();
  EXPECT_EQ(app_service.commands.empty(), true);
}

TEST_F(CliUiControllerTest, ExecuteDispatchesCommittedCommandTypeAndPayload)
{
  controller->commitCommand(0); // LOAD_PLAN_FROM_PATH
  controller->setPayload("{\"path\": \"/tmp/plan.json\"}");

  controller->executeCurrentCommand();
  controller->joinExecution();

  ASSERT_EQ(app_service.commands.size(), 1u);
  EXPECT_EQ(app_service.commands[0].cmd_type, AppServiceCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(app_service.commands[0].payload, "{\"path\": \"/tmp/plan.json\"}");
}

TEST_F(CliUiControllerTest, StoresResultAfterExecution)
{
  controller->commitCommand(0);
  controller->executeCurrentCommand();
  controller->joinExecution();

  auto result = controller->commandResult();
  ASSERT_EQ(result.has_value(), true);
  EXPECT_EQ(result->success, true);
}

TEST_F(CliUiControllerTest, OnUpdateInvokedWhenCommandCompletes)
{
  app_service.slow_command = AppServiceCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = kSlowDelay;

  std::atomic<int> update_count{ 0 };
  controller->setOnUpdate([&] { update_count.fetch_add(1); });

  const int apply_index = static_cast<int>(AppServiceCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);
  controller->executeCurrentCommand();
  controller->joinExecution();

  EXPECT_EQ(update_count.load(), 1);
  EXPECT_EQ(controller->executing(), false);
}

TEST_F(CliUiControllerTest, MarksExecutingWhileInFlightAndClearsAfter)
{
  app_service.slow_command = AppServiceCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = kSlowDelay;

  const int apply_index = static_cast<int>(AppServiceCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);

  std::atomic<bool> observed_executing{ false };
  controller->executeCurrentCommand();
  observed_executing = controller->executing();

  controller->joinExecution();
  EXPECT_EQ(observed_executing, true);
  EXPECT_EQ(controller->executing(), false);
}

TEST_F(CliUiControllerTest, IgnoresSecondExecuteWhileInFlight)
{
  app_service.slow_command = AppServiceCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = kSlowDelay;

  const int apply_index = static_cast<int>(AppServiceCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);

  controller->executeCurrentCommand();
  controller->executeCurrentCommand();
  controller->joinExecution();

  EXPECT_EQ(app_service.commands.size(), 1u);
}

TEST_F(CliUiControllerTest, ExecutedCommandNameReflectsCommittedCommand)
{
  const int apply_index = static_cast<int>(AppServiceCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);
  controller->executeCurrentCommand();
  controller->joinExecution();

  EXPECT_EQ(controller->executedCommand(), "apply_current_plan");
}

TEST_F(CliUiControllerTest, ClearPayloadEmptiesPayload)
{
  controller->setPayload("some payload");
  controller->clearPayload();
  EXPECT_EQ(controller->payload().empty(), true);
}

TEST_F(CliUiControllerTest, ClearResultEmptiesResult)
{
  controller->commitCommand(0);
  controller->executeCurrentCommand();
  controller->joinExecution();
  ASSERT_EQ(controller->commandResult().has_value(), true);

  controller->clearResult();
  EXPECT_EQ(controller->commandResult().has_value(), false);
}

TEST_F(CliUiControllerTest, SelectionAndCommitAreStored)
{
  controller->selectCommand(3);
  EXPECT_EQ(controller->selectedCommand(), 3);

  controller->commitCommand(5);
  EXPECT_EQ(controller->committedCommand(), 5);
}

TEST_F(CliUiControllerTest, OverviewRowsDefaultToEmpty)
{
  // The view reads agent/endpoint rows from the controller; until the
  // data/discovery wiring populates them they are empty.
  EXPECT_EQ(controller->agentRows().empty(), true);
  EXPECT_EQ(controller->endpointRows().empty(), true);
}

TEST_F(CliUiControllerTest, PopulateOverviewOnStartFetchesBothStatusAllProgrammatically)
{
  using pendarlab::app::mavlink_hub::OverviewRow;

  // A hub already holding registered agents and endpoints (e.g. from a plan
  // applied on startup). The mock surfaces them through the status-list payloads.
  app_service.agent_status_all["agent-1"] = 2;      // AgentState::ACTIVE
  app_service.agent_status_all["agent-2"] = 0;      // AgentState::IDLE
  app_service.endpoint_status_all["endpoint-1"] = 2; // MavlinkEndpointState::CONNECTED
  app_service.endpoint_status_all["endpoint-2"] = 0; // MavlinkEndpointState::DISCONNECTED

  controller->updateOverviewData();

  // Both commands were issued programmatically through the service.
  ASSERT_EQ(app_service.commands.size(), 2u);
  EXPECT_EQ(app_service.commands[0].cmd_type, AppServiceCommandType::GET_AGENT_STATUS_ALL);
  EXPECT_EQ(app_service.commands[1].cmd_type, AppServiceCommandType::GET_ENDPOINT_STATUS_ALL);
  // No user interaction is involved: the committed command is untouched.
  EXPECT_EQ(controller->committedCommand(), -1);

  // Row order is not guaranteed (the mock returns an unordered map), so find by
  // name. Each row carries name + lowercase status; type/report are left blank.
  const auto& agent_rows = controller->agentRows();
  ASSERT_EQ(agent_rows.size(), 2u);
  bool saw_agent_1 = false;
  bool saw_agent_2 = false;
  for (const auto& row : agent_rows) {
    EXPECT_EQ(row.type, "");
    EXPECT_EQ(row.report, "");
    if (row.name == "agent-1") {
      saw_agent_1 = true;
      EXPECT_EQ(row.status, "active");
    } else if (row.name == "agent-2") {
      saw_agent_2 = true;
      EXPECT_EQ(row.status, "idle");
    }
  }
  EXPECT_EQ(saw_agent_1, true);
  EXPECT_EQ(saw_agent_2, true);

  const auto& endpoint_rows = controller->endpointRows();
  ASSERT_EQ(endpoint_rows.size(), 2u);
  bool saw_endpoint_1 = false;
  bool saw_endpoint_2 = false;
  for (const auto& row : endpoint_rows) {
    EXPECT_EQ(row.type, "");
    EXPECT_EQ(row.report, "");
    if (row.name == "endpoint-1") {
      saw_endpoint_1 = true;
      EXPECT_EQ(row.status, "connected");
    } else if (row.name == "endpoint-2") {
      saw_endpoint_2 = true;
      EXPECT_EQ(row.status, "disconnected");
    }
  }
  EXPECT_EQ(saw_endpoint_1, true);
  EXPECT_EQ(saw_endpoint_2, true);
}

TEST_F(CliUiControllerTest, PopulateOverviewOnStartParsesReportedStatuses)
{
  // Order is not guaranteed for an unordered map, so find by name.
  app_service.agent_status_all["a"] = 3; // AgentState::STOPPING
  app_service.agent_status_all["b"] = 1; // AgentState::STARTING
  app_service.endpoint_status_all["e1"] = 3; // MavlinkEndpointState::DISCONNECTING
  app_service.endpoint_status_all["e2"] = 1; // MavlinkEndpointState::CONNECTING

  controller->updateOverviewData();

  auto find_status = [](const std::vector<pendarlab::app::mavlink_hub::OverviewRow>& rows,
                        const std::string& name) -> std::string {
    for (const auto& row : rows) {
      if (row.name == name) {
        return row.status;
      }
    }
    return "<missing>";
  };

  EXPECT_EQ(controller->agentRows().size(), 2u);
  EXPECT_EQ(find_status(controller->agentRows(), "a"), "stopping");
  EXPECT_EQ(find_status(controller->agentRows(), "b"), "starting");

  EXPECT_EQ(controller->endpointRows().size(), 2u);
  EXPECT_EQ(find_status(controller->endpointRows(), "e1"), "disconnecting");
  EXPECT_EQ(find_status(controller->endpointRows(), "e2"), "connecting");
}

TEST_F(CliUiControllerTest, PopulateOverviewOnStartLeavesRowsEmptyWhenNoData)
{
  controller->updateOverviewData();
  EXPECT_EQ(controller->agentRows().empty(), true);
  EXPECT_EQ(controller->endpointRows().empty(), true);
}

TEST_F(CliUiControllerTest, PopulateOverviewOnStartDoesNotAlterAppServiceCommandPath)
{
  // The startup fetch must not touch the user-command execution path: the
  // committed command, executing flag, and result stay as the user left them,
  // and a later user execution still behaves normally.
  controller->commitCommand(0); // LOAD_PLAN_FROM_PATH
  controller->setPayload("{\"path\": \"/tmp/plan.json\"}");

  // Simulate the user-command path being used before/independently of the
  // startup fetch (e.g. an async command already completed).
  controller->executeCurrentCommand();
  controller->joinExecution();
  ASSERT_EQ(app_service.commands.size(), 1u);

  // Now run the startup fetch; it must add its two status commands without
  // disturbing the user's committed command, executing flag, or result.
  controller->updateOverviewData();
  ASSERT_EQ(app_service.commands.size(), 3u);

  EXPECT_EQ(controller->committedCommand(), 0);
  EXPECT_EQ(controller->executing(), false);
  ASSERT_EQ(controller->commandResult().has_value(), true);
  EXPECT_EQ(controller->commandResult()->success, true);

  // The user-command path still executes exactly as before after the fetch.
  controller->executeCurrentCommand();
  controller->joinExecution();
  ASSERT_EQ(app_service.commands.size(), 4u);
  EXPECT_EQ(app_service.commands[3].cmd_type, AppServiceCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(app_service.commands[3].payload, "{\"path\": \"/tmp/plan.json\"}");
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
