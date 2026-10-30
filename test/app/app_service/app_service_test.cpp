#include "app/app_service/AppService.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"
#include "common/types/OperationResult.h"
#include "manager/IManager.h"

#include <gtest/gtest.h>
#include <jsoncons/json.hpp>
#include <mock_lib_loader/MockLibLoader.h>
#include <mock_manager/MockManager.h>

#include <string>
#include <unordered_map>
#include <vector>

using namespace jsoncons;

using MockManager = pendarlab::app::mavlink_hub::test::MockManager;
using MockLibLoader = pendarlab::app::mavlink_hub::test::MockLibLoader;
using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
using AgentState = pendarlab::sdk::mavlink_hub::AgentState;

class AppServiceGetCommandTest : public testing::Test
{
protected:
  AppServiceGetCommandTest() :
      app_service(manager, lib_loader)
  {
  }

  void SetUp() override
  {
    manager.endpoint_list = { "endpoint-1", "endpoint-2" };
    manager.endpoint_states["endpoint-1"] = MavlinkEndpointState::CONNECTED;
    manager.endpoint_states["endpoint-2"] = MavlinkEndpointState::DISCONNECTED;
    manager.agent_list = { "agent-1", "agent-2" };
    manager.agent_states["agent-1"] = AgentState::ACTIVE;
    manager.agent_states["agent-2"] = AgentState::IDLE;
  }

  MockManager manager;
  MockLibLoader lib_loader;
  pendarlab::app::mavlink_hub::AppService app_service;

  static pendarlab::app::mavlink_hub::UserCommand commandFor(pendarlab::app::mavlink_hub::UserCommandType type,
                                                             const std::string& payload = "")
  {
    return pendarlab::app::mavlink_hub::UserCommand{ type, payload };
  }
};

// ---------------------------------------------------------------------------
// GET_ENDPOINT_LIST
// ---------------------------------------------------------------------------
TEST_F(AppServiceGetCommandTest, GetEndpointListSucceedsAndEncodesManagerList)
{
  auto result = app_service.executeCommand(commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_ENDPOINT_LIST));

  ASSERT_EQ(result.success, true);

  json j = json::parse(result.data);
  ASSERT_TRUE(j.contains("endpoint_list"));
  ASSERT_TRUE(j["endpoint_list"].is_array());
  ASSERT_EQ(j["endpoint_list"].size(), 2);
  EXPECT_EQ(j["endpoint_list"][0].as_string(), "endpoint-1");
  EXPECT_EQ(j["endpoint_list"][1].as_string(), "endpoint-2");
}

// ---------------------------------------------------------------------------
// GET_ENDPOINT_STATUS
// ---------------------------------------------------------------------------
TEST_F(AppServiceGetCommandTest, GetEndpointStatusSucceedsForRegisteredEndpoint)
{
  auto result = app_service.executeCommand(
      commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_ENDPOINT_STATUS, "endpoint-1"));

  ASSERT_EQ(result.success, true);

  json j = json::parse(result.data);
  ASSERT_TRUE(j.contains("endpoint_status"));
  EXPECT_EQ(j["endpoint_status"].as_string(), "connected");
}

TEST_F(AppServiceGetCommandTest, GetEndpointStatusFailsWhenPayloadIsMissing)
{
  auto result = app_service.executeCommand(commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_ENDPOINT_STATUS, ""));

  ASSERT_EQ(result.success, false);
  ASSERT_FALSE(result.message.empty());
  EXPECT_NE(result.message[0].find("endpoint name is missing"), std::string::npos);
}

TEST_F(AppServiceGetCommandTest, GetEndpointStatusFailsForUnregisteredEndpoint)
{
  auto result = app_service.executeCommand(
      commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_ENDPOINT_STATUS, "unknown-endpoint"));

  ASSERT_EQ(result.success, false);
  ASSERT_FALSE(result.message.empty());
  EXPECT_NE(result.message[0].find("is not registered"), std::string::npos);
}

// ---------------------------------------------------------------------------
// GET_ENDPOINT_STATUS_ALL
// ---------------------------------------------------------------------------
TEST_F(AppServiceGetCommandTest, GetEndpointStatusAllSucceedsAndEncodesManagerStates)
{
  auto result = app_service.executeCommand(commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_ENDPOINT_STATUS_ALL));

  ASSERT_EQ(result.success, true);

  json j = json::parse(result.data);
  ASSERT_TRUE(j.contains("endpoint_status_list"));
  ASSERT_TRUE(j["endpoint_status_list"].is_array());
  ASSERT_EQ(j["endpoint_status_list"].size(), 2);

  // The existing JsonUtils endpointStatusListToJsonString encoder emits the raw
  // MavlinkEndpointState enum value (0=DISCONNECTED ... 2=CONNECTED), not a string.
  std::unordered_map<std::string, std::string> name_to_status;
  for (const auto& entry : j["endpoint_status_list"].array_range()) {
    name_to_status[entry["name"].as_string()] = entry["status"].as_string();
  }
  EXPECT_EQ(name_to_status["endpoint-1"], "2"); // CONNECTED
  EXPECT_EQ(name_to_status["endpoint-2"], "0"); // DISCONNECTED
}

// ---------------------------------------------------------------------------
// GET_AGENT_LIST
// ---------------------------------------------------------------------------
TEST_F(AppServiceGetCommandTest, GetAgentListSucceedsAndEncodesManagerList)
{
  auto result = app_service.executeCommand(commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_AGENT_LIST));

  ASSERT_EQ(result.success, true);

  json j = json::parse(result.data);
  ASSERT_TRUE(j.contains("agent_list"));
  ASSERT_TRUE(j["agent_list"].is_array());
  ASSERT_EQ(j["agent_list"].size(), 2);
  EXPECT_EQ(j["agent_list"][0].as_string(), "agent-1");
  EXPECT_EQ(j["agent_list"][1].as_string(), "agent-2");
}

// ---------------------------------------------------------------------------
// GET_AGENT_STATUS
// ---------------------------------------------------------------------------
TEST_F(AppServiceGetCommandTest, GetAgentStatusSucceedsForRegisteredAgent)
{
  auto result = app_service.executeCommand(
      commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_AGENT_STATUS, "agent-1"));

  ASSERT_EQ(result.success, true);

  json j = json::parse(result.data);
  ASSERT_TRUE(j.contains("agent_status"));
  EXPECT_EQ(j["agent_status"].as_string(), "active");
}

TEST_F(AppServiceGetCommandTest, GetAgentStatusFailsWhenPayloadIsMissing)
{
  auto result = app_service.executeCommand(commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_AGENT_STATUS, ""));

  ASSERT_EQ(result.success, false);
  ASSERT_FALSE(result.message.empty());
  EXPECT_NE(result.message[0].find("agent name is missing"), std::string::npos);
}

TEST_F(AppServiceGetCommandTest, GetAgentStatusFailsForUnregisteredAgent)
{
  auto result = app_service.executeCommand(
      commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_AGENT_STATUS, "unknown-agent"));

  ASSERT_EQ(result.success, false);
  ASSERT_FALSE(result.message.empty());
  EXPECT_NE(result.message[0].find("is not registered"), std::string::npos);
}

// ---------------------------------------------------------------------------
// GET_AGENT_STATUS_ALL
// ---------------------------------------------------------------------------
TEST_F(AppServiceGetCommandTest, GetAgentStatusAllSucceedsAndEncodesManagerStates)
{
  auto result = app_service.executeCommand(commandFor(pendarlab::app::mavlink_hub::UserCommandType::GET_AGENT_STATUS_ALL));

  ASSERT_EQ(result.success, true);

  json j = json::parse(result.data);
  ASSERT_TRUE(j.contains("agent_status_list"));
  ASSERT_TRUE(j["agent_status_list"].is_array());
  ASSERT_EQ(j["agent_status_list"].size(), 2);

  // The existing JsonUtils agentStatusListToJsonString encoder emits the raw
  // AgentState enum value (0=IDLE ... 2=ACTIVE), not a string.
  std::unordered_map<std::string, std::string> name_to_status;
  for (const auto& entry : j["agent_status_list"].array_range()) {
    name_to_status[entry["name"].as_string()] = entry["status"].as_string();
  }
  EXPECT_EQ(name_to_status["agent-1"], "2"); // ACTIVE
  EXPECT_EQ(name_to_status["agent-2"], "0"); // IDLE
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}
