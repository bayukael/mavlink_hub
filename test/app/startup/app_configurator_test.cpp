#include "app/startup/AppConfigurator.h"
#include "app/types/AppConfig.h"
#include "mock_app_service/MockAppService.h"
#include "mock_lib_loader/MockLibLoader.h"

#include <gtest/gtest.h>
#include <string>

using pendarlab::app::mavlink_hub::AppConfig;
using pendarlab::app::mavlink_hub::LibInfo;
using pendarlab::app::mavlink_hub::startup::applyConfig;
using pendarlab::app::mavlink_hub::test::MockAppService;
using pendarlab::app::mavlink_hub::test::MockLibLoader;
using pendarlab::app::mavlink_hub::UserCommandType;

namespace
{
  const std::string test_data_dir = TEST_DATA_DIR;
}

class AppConfiguratorTest : public testing::Test
{
protected:
  void SetUp() override {}

  AppConfig makeConfig()
  {
    AppConfig config;
    config.agent_lib_list.emplace("agent_a", LibInfo{ "agent_a", "/lib/agent_a.so", "getAgentA" });
    config.transport_lib_list.emplace("transport_a", LibInfo{ "transport_a", "/lib/transport_a.so", "getTransportA" });
    return config;
  }

  MockLibLoader lib_loader;
  MockAppService app_service;
};

TEST_F(AppConfiguratorTest, EmptyConfigShouldReturnSuccessAndMakeNoCalls)
{
  AppConfig config;
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, true);
  EXPECT_EQ(lib_loader.loaded_agent_libs.empty(), true);
  EXPECT_EQ(lib_loader.loaded_transport_libs.empty(), true);
  EXPECT_EQ(app_service.commands.empty(), true);
}

TEST_F(AppConfiguratorTest, LoadsAgentAndTransportLibsFromConfig)
{
  auto config = makeConfig();
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, true);
  ASSERT_EQ(lib_loader.loaded_agent_libs.size(), 1u);
  EXPECT_EQ(lib_loader.loaded_agent_libs[0].name, "agent_a");
  EXPECT_EQ(lib_loader.loaded_agent_libs[0].path, "/lib/agent_a.so");
  EXPECT_EQ(lib_loader.loaded_agent_libs[0].sym, "getAgentA");
  ASSERT_EQ(lib_loader.loaded_transport_libs.size(), 1u);
  EXPECT_EQ(lib_loader.loaded_transport_libs[0].name, "transport_a");
  EXPECT_EQ(app_service.commands.empty(), true);
}

TEST_F(AppConfiguratorTest, AgentLibLoadFailureShouldReturnFailure)
{
  auto config = makeConfig();
  lib_loader.fail_agent_lib = true;
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, false);
}

TEST_F(AppConfiguratorTest, TransportLibLoadFailureShouldReturnFailure)
{
  auto config = makeConfig();
  lib_loader.fail_transport_lib = true;
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, false);
}

TEST_F(AppConfiguratorTest, MissingExtraLibListFileShouldReturnFailure)
{
  auto config = makeConfig();
  config.path_to_extra_lib_list = test_data_dir + "non_existent_file.json";
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, false);
  EXPECT_EQ(lib_loader.loaded_agent_libs.size(), 1u);
  EXPECT_EQ(lib_loader.loaded_transport_libs.size(), 1u);
}

TEST_F(AppConfiguratorTest, ValidExtraLibListFileShouldLoadExtraLibs)
{
  auto config = makeConfig();
  config.path_to_extra_lib_list = test_data_dir + "extra_lib_list.json";
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, true);
  ASSERT_EQ(lib_loader.loaded_agent_libs.size(), 2u);
  EXPECT_EQ(lib_loader.loaded_agent_libs[1].name, "mock_agent_from_extra");
  ASSERT_EQ(lib_loader.loaded_transport_libs.size(), 2u);
  EXPECT_EQ(lib_loader.loaded_transport_libs[1].name, "mock_transport_from_extra");
}

TEST_F(AppConfiguratorTest, PlanLoadCommandShouldBeIssuedWhenStartupPlanPathSet)
{
  auto config = makeConfig();
  config.path_to_startup_user_plan = "/path/to/plan.json";
  config.apply_user_plan_on_startup = false;
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, true);
  ASSERT_EQ(app_service.commands.size(), 1u);
  EXPECT_EQ(app_service.commands[0].cmd_type, UserCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(app_service.commands[0].payload, "/path/to/plan.json");
}

TEST_F(AppConfiguratorTest, PlanApplyCommandShouldBeIssuedWhenApplyOnStartup)
{
  auto config = makeConfig();
  config.path_to_startup_user_plan = "/path/to/plan.json";
  config.apply_user_plan_on_startup = true;
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, true);
  ASSERT_EQ(app_service.commands.size(), 2u);
  EXPECT_EQ(app_service.commands[0].cmd_type, UserCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(app_service.commands[1].cmd_type, UserCommandType::APPLY_CURRENT_PLAN);
}

TEST_F(AppConfiguratorTest, PlanApplyShouldBeSkippedWhenLoadFails)
{
  auto config = makeConfig();
  config.path_to_startup_user_plan = "/path/to/plan.json";
  config.apply_user_plan_on_startup = true;
  app_service.fail_load_plan = true;
  auto result = applyConfig(config, lib_loader, app_service);

  EXPECT_EQ(result.success, false);
  ASSERT_EQ(app_service.commands.size(), 1u);
  EXPECT_EQ(app_service.commands[0].cmd_type, UserCommandType::LOAD_PLAN_FROM_PATH);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
