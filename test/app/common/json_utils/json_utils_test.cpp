#include <common/json_utils/JsonUtils.h>
#include <fstream>
#include <gtest/gtest.h>
#include <common/types/OperationResult.h>
#include <manager/types/AgentEntry.h>
#include <manager/types/ExecutionResultList.h>
#include <manager/types/MavlinkEndpointEntry.h>
#include <manager/types/UserPlan.h>
#include <optional>
#include <string>
#include <unordered_map>

using MavlinkEndpointEntry = pendarlab::app::mavlink_hub::MavlinkEndpointEntry;
using AgentEntry = pendarlab::app::mavlink_hub::AgentEntry;
using UserPlan = pendarlab::app::mavlink_hub::UserPlan;
using ExecutionResultList = pendarlab::app::mavlink_hub::ExecutionResultList;
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
  EXPECT_EQ(parsed_plan.has_value(), false);
}

TEST_F(JsonUtilsConvertToUserPlanTest, ValidPlanWithBestEffortPolicyAndNoAgentListIsExtractedProperly)
{
  using namespace pendarlab::app::mavlink_hub;
  std::string file_path = file_dir + "plan_best_effort_no_agent_list.json";
  correct_agent_list.clear();

  std::ifstream file_stream(file_path);
  ASSERT_EQ(file_stream.is_open(), true);

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
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

  std::optional<UserPlan> parsed_plan = fstreamToUserPlan(file_stream);
  EXPECT_EQ(parsed_plan.has_value(), false);
}

class ExecutionResultListToStringTest : public testing::Test
{
protected:
  ExecutionResultList list;
  void SetUp() override {
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

TEST_F(ExecutionResultListToStringTest, ConvertingExecutionResultListToStringShouldProduceCorrectString){
  std::cout << executionResultListToString(list) << std::endl;
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}