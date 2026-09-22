#include "app/ui_handler/CliUiController.h"
#include "mock_app_service/MockAppService.h"

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using pendarlab::app::mavlink_hub::CliUiController;
using pendarlab::app::mavlink_hub::CommandDescriptor;
using pendarlab::app::mavlink_hub::UserCommandType;
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
  EXPECT_EQ(controller->commandDescriptors()[0].type, UserCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(controller->commandEntries()[0], std::string(controller->commandDescriptors()[0].name));
}

TEST_F(CliUiControllerTest, NoCommandCommittedDispatchesNothing)
{
  EXPECT_EQ(controller->committedCommand(), -1);
  controller->executeCurrentCommand();
  controller->joinExecution();
  EXPECT_TRUE(app_service.commands.empty());
}

TEST_F(CliUiControllerTest, CommittedCommandWithOutOfRangeIndexDispatchesNothing)
{
  controller->commitCommand(12345);
  controller->executeCurrentCommand();
  controller->joinExecution();
  EXPECT_TRUE(app_service.commands.empty());
}

TEST_F(CliUiControllerTest, ExecuteDispatchesCommittedCommandTypeAndPayload)
{
  controller->commitCommand(0); // LOAD_PLAN_FROM_PATH
  controller->setPayload("{\"path\": \"/tmp/plan.json\"}");

  controller->executeCurrentCommand();
  controller->joinExecution();

  ASSERT_EQ(app_service.commands.size(), 1u);
  EXPECT_EQ(app_service.commands[0].cmd_type, UserCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(app_service.commands[0].payload, "{\"path\": \"/tmp/plan.json\"}");
}

TEST_F(CliUiControllerTest, StoresResultAfterExecution)
{
  controller->commitCommand(0);
  controller->executeCurrentCommand();
  controller->joinExecution();

  auto result = controller->commandResult();
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->success);
}

TEST_F(CliUiControllerTest, OnUpdateInvokedWhenCommandCompletes)
{
  app_service.slow_command = UserCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = kSlowDelay;

  std::atomic<int> update_count{ 0 };
  controller->setOnUpdate([&] { update_count.fetch_add(1); });

  const int apply_index = static_cast<int>(UserCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);
  controller->executeCurrentCommand();
  controller->joinExecution();

  EXPECT_EQ(update_count.load(), 1);
  EXPECT_FALSE(controller->executing());
}

TEST_F(CliUiControllerTest, MarksExecutingWhileInFlightAndClearsAfter)
{
  app_service.slow_command = UserCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = kSlowDelay;

  const int apply_index = static_cast<int>(UserCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);

  std::atomic<bool> observed_executing{ false };
  controller->executeCurrentCommand();
  observed_executing = controller->executing();

  controller->joinExecution();
  EXPECT_TRUE(observed_executing);
  EXPECT_FALSE(controller->executing());
}

TEST_F(CliUiControllerTest, IgnoresSecondExecuteWhileInFlight)
{
  app_service.slow_command = UserCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = kSlowDelay;

  const int apply_index = static_cast<int>(UserCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);

  controller->executeCurrentCommand();
  controller->executeCurrentCommand();
  controller->joinExecution();

  EXPECT_EQ(app_service.commands.size(), 1u);
}

TEST_F(CliUiControllerTest, ExecutedCommandNameReflectsCommittedCommand)
{
  const int apply_index = static_cast<int>(UserCommandType::APPLY_CURRENT_PLAN);
  controller->commitCommand(apply_index);
  controller->executeCurrentCommand();
  controller->joinExecution();

  EXPECT_EQ(controller->executedCommand(), "apply_current_plan");
}

TEST_F(CliUiControllerTest, ClearPayloadEmptiesPayload)
{
  controller->setPayload("some payload");
  controller->clearPayload();
  EXPECT_TRUE(controller->payload().empty());
}

TEST_F(CliUiControllerTest, ClearResultEmptiesResult)
{
  controller->commitCommand(0);
  controller->executeCurrentCommand();
  controller->joinExecution();
  ASSERT_TRUE(controller->commandResult().has_value());

  controller->clearResult();
  EXPECT_FALSE(controller->commandResult().has_value());
}

TEST_F(CliUiControllerTest, SelectionAndCommitAreStored)
{
  controller->selectCommand(3);
  EXPECT_EQ(controller->selectedCommand(), 3);

  controller->commitCommand(5);
  EXPECT_EQ(controller->committedCommand(), 5);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
