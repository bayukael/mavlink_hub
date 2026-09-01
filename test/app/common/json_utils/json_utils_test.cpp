#include <common/json_utils/JsonUtils.h>
#include <common/types/OperationResult.h>
#include <fstream>
#include <gtest/gtest.h>
#include <jsoncons/json.hpp>
#include <manager/types/AgentEntry.h>
#include <manager/types/ExecutionResultList.h>
#include <manager/types/MavlinkEndpointEntry.h>
#include <manager/types/UserPlan.h>
#include <mavlink_endpoint/MavlinkEndpointState.h>
#include <mavlink_hub_sdk/agent/AgentState.h>
#include <optional>
#include <string>
#include <unordered_map>

using namespace jsoncons;

using MavlinkEndpointEntry = pendarlab::app::mavlink_hub::MavlinkEndpointEntry;
using AgentEntry = pendarlab::app::mavlink_hub::AgentEntry;
using UserPlan = pendarlab::app::mavlink_hub::UserPlan;
using ExecutionResultList = pendarlab::app::mavlink_hub::ExecutionResultList;
using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
using OperationResult = pendarlab::app::mavlink_hub::OperationResult;

class JsonUtilsConvertToUserPlanTest : public testing::Test
{
protected:
  std::string file_dir;
  std::unordered_map<std::string, AgentEntry> correct_agent_list;
  std::unordered_map<std::string, MavlinkEndpointEntry> correct_endpoint_list;
  void SetUp() override
  {
    file_dir = TEST_PLAN_DIR;

    AgentEntry agent_entry;
    agent_entry.config = std::unordered_map<std::string, std::string>();
    for (size_t i = 1; i <= 3; i++) {
      std::string agent_idx = std::to_string(i);
      std::string agent_name = "agent-" + agent_idx;
      std::string agent_type = "agent-type-" + agent_idx;
      agent_entry.type = agent_type;
      correct_agent_list[agent_name] = agent_entry;
    };
    MavlinkEndpointEntry endpoint_entry;
    endpoint_entry.config = std::unordered_map<std::string, std::string>();
    for (size_t i = 1; i <= 2; i++) {
      std::string endpoint_idx = std::to_string(i);
      std::string endpoint_name = "endpoint-" + endpoint_idx;
      std::string endpoint_type = "endpoint-type-" + endpoint_idx;
      endpoint_entry.type = endpoint_type;
      correct_endpoint_list[endpoint_name] = endpoint_entry;
    };
  }
  void TearDown() override {}
};

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithBestEffortPolicyIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_best_effort.json";

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  // Check whether all entries in agent_list exist in correct_agent_list
  ASSERT_EQ(user_plan.agent_list.size(), correct_agent_list.size());
  for (const auto& [agent_name, agent_entry] : user_plan.agent_list) {
    auto it = correct_agent_list.find(agent_name);
    ASSERT_NE(it, correct_agent_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(agent_entry.config.size(), correct_agent_list[agent_name].config.size());
    for (const auto& [config_key, config_val] : agent_entry.config) {
      auto it2 = correct_agent_list[agent_name].config.find(config_key);
      EXPECT_NE(it2, correct_agent_list[agent_name].config.end());
    }
  }

  ASSERT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
  for (const auto& [endpoint_name, endpoint_entry] : user_plan.endpoint_list) {
    auto it = correct_endpoint_list.find(endpoint_name);
    ASSERT_NE(it, correct_endpoint_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(endpoint_entry.config.size(), correct_endpoint_list[endpoint_name].config.size());
    for (const auto& [config_key, config_val] : endpoint_entry.config) {
      auto it2 = correct_endpoint_list[endpoint_name].config.find(config_key);
      EXPECT_NE(it2, correct_endpoint_list[endpoint_name].config.end());
    }
  }
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithDiscardPolicyIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_discard.json";

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  // Check whether all entries in agent_list exist in correct_agent_list
  ASSERT_EQ(user_plan.agent_list.size(), correct_agent_list.size());
  for (const auto& [agent_name, agent_entry] : user_plan.agent_list) {
    auto it = correct_agent_list.find(agent_name);
    ASSERT_NE(it, correct_agent_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(agent_entry.config.size(), correct_agent_list[agent_name].config.size());
    for (const auto& [config_key, config_val] : agent_entry.config) {
      auto it2 = correct_agent_list[agent_name].config.find(config_key);
      EXPECT_NE(it2, correct_agent_list[agent_name].config.end());
    }
  }

  ASSERT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
  for (const auto& [endpoint_name, endpoint_entry] : user_plan.endpoint_list) {
    auto it = correct_endpoint_list.find(endpoint_name);
    ASSERT_NE(it, correct_endpoint_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(endpoint_entry.config.size(), correct_endpoint_list[endpoint_name].config.size());
    for (const auto& [config_key, config_val] : endpoint_entry.config) {
      auto it2 = correct_endpoint_list[endpoint_name].config.find(config_key);
      EXPECT_NE(it2, correct_endpoint_list[endpoint_name].config.end());
    }
  }
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithUnknownPolicyMustReturnNullopt)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_unknown_policy.json";

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  EXPECT_EQ(parsed_plan.has_value(), false);
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithBestEffortPolicyAndNoAgentListIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_best_effort_no_agent_list.json";
  correct_agent_list.clear();

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  EXPECT_EQ(user_plan.agent_list.size(), correct_agent_list.size());

  ASSERT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
  for (const auto& [endpoint_name, endpoint_entry] : user_plan.endpoint_list) {
    auto it = correct_endpoint_list.find(endpoint_name);
    ASSERT_NE(it, correct_endpoint_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(endpoint_entry.config.size(), correct_endpoint_list[endpoint_name].config.size());
    for (const auto& [config_key, config_val] : endpoint_entry.config) {
      auto it2 = correct_endpoint_list[endpoint_name].config.find(config_key);
      EXPECT_NE(it2, correct_endpoint_list[endpoint_name].config.end());
    }
  }
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithDiscardPolicyAndNoAgentListIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_discard_no_agent_list.json";
  correct_agent_list.clear();

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  EXPECT_EQ(user_plan.agent_list.size(), correct_agent_list.size());

  ASSERT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
  for (const auto& [endpoint_name, endpoint_entry] : user_plan.endpoint_list) {
    auto it = correct_endpoint_list.find(endpoint_name);
    ASSERT_NE(it, correct_endpoint_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(endpoint_entry.config.size(), correct_endpoint_list[endpoint_name].config.size());
    for (const auto& [config_key, config_val] : endpoint_entry.config) {
      auto it2 = correct_endpoint_list[endpoint_name].config.find(config_key);
      EXPECT_NE(it2, correct_endpoint_list[endpoint_name].config.end());
    }
  }
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithBestEffortPolicyAndNoEndpointListIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_best_effort_no_endpoint_list.json";
  correct_endpoint_list.clear();

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  // Check whether all entries in agent_list exist in correct_agent_list
  ASSERT_EQ(user_plan.agent_list.size(), correct_agent_list.size());
  for (const auto& [agent_name, agent_entry] : user_plan.agent_list) {
    auto it = correct_agent_list.find(agent_name);
    ASSERT_NE(it, correct_agent_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(agent_entry.config.size(), correct_agent_list[agent_name].config.size());
    for (const auto& [config_key, config_val] : agent_entry.config) {
      auto it2 = correct_agent_list[agent_name].config.find(config_key);
      EXPECT_NE(it2, correct_agent_list[agent_name].config.end());
    }
  }

  EXPECT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithDiscardPolicyAndNoEndpointListIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_discard_no_endpoint_list.json";
  correct_endpoint_list.clear();

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  // Check whether all entries in agent_list exist in correct_agent_list
  ASSERT_EQ(user_plan.agent_list.size(), correct_agent_list.size());
  for (const auto& [agent_name, agent_entry] : user_plan.agent_list) {
    auto it = correct_agent_list.find(agent_name);
    ASSERT_NE(it, correct_agent_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(agent_entry.config.size(), correct_agent_list[agent_name].config.size());
    for (const auto& [config_key, config_val] : agent_entry.config) {
      auto it2 = correct_agent_list[agent_name].config.find(config_key);
      EXPECT_NE(it2, correct_agent_list[agent_name].config.end());
    }
  }

  EXPECT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
}

TEST_F(JsonUtilsConvertToUserPlanTest, PlanWithBestEffortPolicyAndSomeInvalidEntriesCanStillBeExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_best_effort_missing_required_field.json";
  correct_agent_list.erase("agent-2");
  correct_endpoint_list.erase("endpoint-2");

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  ASSERT_EQ(parsed_plan.has_value(), true);
  auto& user_plan = parsed_plan.value();

  // Check whether all entries in agent_list exist in correct_agent_list
  ASSERT_EQ(user_plan.agent_list.size(), correct_agent_list.size());
  for (const auto& [agent_name, agent_entry] : user_plan.agent_list) {
    auto it = correct_agent_list.find(agent_name);
    ASSERT_NE(it, correct_agent_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(agent_entry.config.size(), correct_agent_list[agent_name].config.size());
    for (const auto& [config_key, config_val] : agent_entry.config) {
      auto it2 = correct_agent_list[agent_name].config.find(config_key);
      EXPECT_NE(it2, correct_agent_list[agent_name].config.end());
    }
  }

  ASSERT_EQ(user_plan.endpoint_list.size(), correct_endpoint_list.size());
  for (const auto& [endpoint_name, endpoint_entry] : user_plan.endpoint_list) {
    auto it = correct_endpoint_list.find(endpoint_name);
    ASSERT_NE(it, correct_endpoint_list.end());

    // Now we check whether the config of an agent is the same as the correct one.
    ASSERT_EQ(endpoint_entry.config.size(), correct_endpoint_list[endpoint_name].config.size());
    for (const auto& [config_key, config_val] : endpoint_entry.config) {
      auto it2 = correct_endpoint_list[endpoint_name].config.find(config_key);
      EXPECT_NE(it2, correct_endpoint_list[endpoint_name].config.end());
    }
  }
}

TEST_F(JsonUtilsConvertToUserPlanTest, PlanWithDiscardPolicyAndSomeInvalidEntriesShouldReturnNullopt)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_discard_missing_required_field.json";
  correct_agent_list.erase("agent-2");
  correct_endpoint_list.erase("endpoint-2");

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = json_utils::fstreamToUserPlan(file_stream);
  EXPECT_EQ(parsed_plan.has_value(), false);
}

class ExecutionResultListToJsonStringTest : public testing::Test
{
protected:
  ExecutionResultList list;
  void SetUp() override
  {
    OperationResult op_result;
    op_result.success = true;
    op_result.messages.push_back("endpoint-1 success");
    list.endpoint_plan_result["endpoint-1"] = op_result;

    op_result.messages.clear();
    op_result.success = true;
    op_result.messages.push_back("endpoint-2 success but with some warning");
    op_result.messages.push_back("endpoint-2 uses a deprecated API");
    list.endpoint_plan_result["endpoint-2"] = op_result;

    op_result.messages.clear();
    op_result.success = false;
    op_result.messages.push_back("agent-1 failed to create");
    op_result.messages.push_back("agent-1 type does not exist");
    list.agent_plan_result["agent-1"] = op_result;

    op_result.messages.clear();
    op_result.success = true;
    op_result.messages.push_back("agent-2 failed to create");
    op_result.messages.push_back("agent-2 cannot obtain required resources");
    list.agent_plan_result["agent-2"] = op_result;
  }
  void TearDown() override {}
};

TEST_F(ExecutionResultListToJsonStringTest, ConvertingExecutionResultListToJsonStringShouldProduceCorrectString)
{
  using namespace pendarlab::app::mavlink_hub;

  json j_actual;
  bool parse_success = true;
  try {
    j_actual = json::parse(json_utils::executionResultListToJsonString(list));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  ASSERT_EQ(j_actual.contains("endpoint_plan_result"), true);
  ASSERT_EQ(j_actual["endpoint_plan_result"].is_array(), true);
  ASSERT_EQ(j_actual.contains("agent_plan_result"), true);
  ASSERT_EQ(j_actual["agent_plan_result"].is_array(), true);

  for (const auto& list_entry : j_actual["endpoint_plan_result"].array_range()) {
    ASSERT_EQ(list_entry.contains("name"), true);
    ASSERT_EQ(list_entry.contains("success"), true);
    ASSERT_EQ(list_entry.contains("messages"), true);
    ASSERT_EQ(list_entry["messages"].is_array(), true);
    bool name_ok = false;
    bool success_ok = false;
    bool messages_ok = true;
    for (const auto& [name, op_result] : list.endpoint_plan_result) {
      if (name == list_entry["name"]) {
        name_ok = true;
        if (op_result.success == list_entry["success"].as_bool()) {
          success_ok = true;
        }
        for (const auto& actual_msg : list_entry["messages"].array_range()) {
          bool found_msg = false;
          for (const auto& expected_msg : op_result.messages) {
            if (actual_msg == expected_msg) {
              found_msg = true;
            }
          }
          if (!found_msg) {
            messages_ok = false;
            break;
          }
        }
      }
    }
    EXPECT_EQ(name_ok, true);
    EXPECT_EQ(success_ok, true);
    EXPECT_EQ(messages_ok,true);
  }
}

class StatusToJsonStringTest : public testing::Test
{
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(StatusToJsonStringTest, AgentActiveStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;

  j_expected["agent_status"] = "active";
  try {
    j_actual = json::parse(json_utils::agentStatusToJsonString(AgentState::ACTIVE));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, AgentIdleStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;

  j_expected["agent_status"] = "idle";
  try {
    j_actual = json::parse(json_utils::agentStatusToJsonString(AgentState::IDLE));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, AgentStartingStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;

  j_expected["agent_status"] = "starting";
  try {
    j_actual = json::parse(json_utils::agentStatusToJsonString(AgentState::STARTING));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, AgentStoppingStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;

  j_expected["agent_status"] = "stopping";
  try {
    j_actual = json::parse(json_utils::agentStatusToJsonString(AgentState::STOPPING));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, EndpointDisconnectedStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;
  j_expected["endpoint_status"] = "disconnected";
  try {
    j_actual = json::parse(json_utils::endpointStatusToJsonString(MavlinkEndpointState::DISCONNECTED));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, EndpointConnectedStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;
  j_expected["endpoint_status"] = "connected";
  try {
    j_actual = json::parse(json_utils::endpointStatusToJsonString(MavlinkEndpointState::CONNECTED));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, EndpointDisconnectingStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;
  j_expected["endpoint_status"] = "disconnecting";
  try {
    j_actual = json::parse(json_utils::endpointStatusToJsonString(MavlinkEndpointState::DISCONNECTING));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

TEST_F(StatusToJsonStringTest, EndpointConnectingStateShouldReturnProperJsonString)
{
  using namespace pendarlab::app::mavlink_hub;
  json j_expected;
  json j_actual;
  bool parse_success = true;
  j_expected["endpoint_status"] = "connecting";
  try {
    j_actual = json::parse(json_utils::endpointStatusToJsonString(MavlinkEndpointState::CONNECTING));
  } catch (const ser_error& e) {
    parse_success = false;
  }

  ASSERT_EQ(parse_success, true);
  EXPECT_EQ(j_actual, j_expected);
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}