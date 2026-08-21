#include <agent_registry/AgentRegistry.h>
#include <agent_registry/AgentRegistryUserAccess.h>
#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUserAccess.h>
#include <gtest/gtest.h>
#include <manager/Manager.h>
#include <manager/ManagerResourceRequester.h>
#include <manager/types/AgentEntry.h>
#include <manager/types/ExecutionResultList.h>
#include <manager/types/MavlinkEndpointEntry.h>
#include <manager/types/UserPlan.h>
#include <manager/types/UserPlanPolicy.h>
#include <memory>
#include <mock_agent/MockAgentDefinition.h>
#include <mock_byte_transport/MockByteTransportDefinition.h>
#include <string>

using MockAgentDefinition = pendarlab::app::mavlink_hub::test::MockAgentDefinition;
using MockByteTransportDefinition = pendarlab::app::mavlink_hub::test::MockByteTransportDefinition;

static MockAgentDefinition g_mock_agent_def;
static MockByteTransportDefinition g_mock_byte_transport_def;

using TransportRegistry = pendarlab::lib::comm::byte_transport::Registry;
using TransportRegistryUser = pendarlab::lib::comm::byte_transport::RegistryUserAccess;
using AgentRegistry = pendarlab::app::mavlink_hub::AgentRegistry;
using AgentRegistryUser = pendarlab::app::mavlink_hub::AgentRegistryUserAccess;
using Manager = pendarlab::app::mavlink_hub::Manager;
using ExecutionResultList = pendarlab::app::mavlink_hub::ExecutionResultList;
using AgentEntry = pendarlab::app::mavlink_hub::AgentEntry;
using EndpointEntry = pendarlab::app::mavlink_hub::MavlinkEndpointEntry;
using UserPlan = pendarlab::app::mavlink_hub::UserPlan;
using UserPlanPolicy = pendarlab::app::mavlink_hub::UserPlanPolicy;
using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
using ManagerResourceRequester = pendarlab::app::mavlink_hub::ManagerResourceRequester;
using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

class ManagerTestSetup
{
public:
  ManagerTestSetup();

protected:
  TransportRegistry transport_registry;
  std::unique_ptr<TransportRegistryUser> transport_registry_user;
  AgentRegistry agent_registry;
  std::unique_ptr<AgentRegistryUser> agent_registry_user;
  Manager manager;
};

static const std::string g_mock_transport_name("mock_byte_transport");
static const std::string g_mock_agent_name("mock_agent");

ManagerTestSetup::ManagerTestSetup() :
    transport_registry_user(transport_registry.createUser()),
    agent_registry_user(agent_registry.createUser()),
    manager(*agent_registry_user, *transport_registry_user)
{
  transport_registry.addTransportDefinition(g_mock_transport_name, g_mock_byte_transport_def);
  agent_registry.addAgentDefinition(g_mock_agent_name, g_mock_agent_def);
}

// TEST - [ ManagerInitialTest ]
// # Adding a MockAgent should work
// # Adding a non-registered Agent type should not work.
// # With a MockAgent, validating the config should work
// # With a non-registered Agent, validating the config should fail

class ManagerInitialTest : public testing::Test, public ManagerTestSetup
{
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ManagerInitialTest, MockObjectsShouldHaveBeenAdded)
{
  ASSERT_EQ(transport_registry.isRegistered(g_mock_transport_name), true);
  ASSERT_EQ(agent_registry.isRegistered(g_mock_agent_name), true);
}

TEST_F(ManagerInitialTest, AddingEndpointWithUniqueNameShouldWork)
{
  auto result = manager.addMavlinkEndpoint("ep1");
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 1);
  result = manager.addMavlinkEndpoint("ep2");
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 2);
}

TEST_F(ManagerInitialTest, AddingEndpointWithSameNameShouldNotWork)
{
  std::string some_ep_name("same_ep_name");
  auto result = manager.addMavlinkEndpoint(some_ep_name);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 1);
  result = manager.addMavlinkEndpoint(some_ep_name);
  EXPECT_EQ(result.success, false);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 1);
}

TEST_F(ManagerInitialTest, ValidatingRegisteredEndpointWithCorrectConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.validateMavlinkEndpointConfig(g_mock_transport_name, config);
  EXPECT_EQ(result.success, true);
}

TEST_F(ManagerInitialTest, ValidatingRegisteredEndpointWithIncorrectConfigShouldReturnFalse)
{
  std::unordered_map<std::string, std::string> config{
    {"incorrect_string", ""}
  };
  auto result = manager.validateMavlinkEndpointConfig(g_mock_transport_name, config);
  EXPECT_EQ(result.success, false);
}

TEST_F(ManagerInitialTest, ValidatingNonRegisteredEndpointConfigShouldReturnFalse)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.validateMavlinkEndpointConfig("Non-registered_transport", config);
  EXPECT_EQ(result.success, false);
}

TEST_F(ManagerInitialTest, AddingAgentWithUniqueNameShouldWork)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.addAgent("ag1", g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getAgentList().size(), 1);
  result = manager.addAgent("ag2", g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getAgentList().size(), 2);
}

TEST_F(ManagerInitialTest, AddingAgentWithSameNameShouldNotWork)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.addAgent("ag1", g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getAgentList().size(), 1);
  result = manager.addAgent("ag1", g_mock_agent_name, config);
  EXPECT_EQ(result.success, false);
  EXPECT_EQ(manager.getAgentList().size(), 1);
}

TEST_F(ManagerInitialTest, ValidatingRegisteredAgentWithCorrectConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.validateAgentConfig(g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
}

TEST_F(ManagerInitialTest, ValidatingRegisteredAgentWithIncorrectConfigShouldReturnFalse)
{
  std::unordered_map<std::string, std::string> config{
    {"incorrect_string", ""}
  };
  auto result = manager.validateAgentConfig(g_mock_agent_name, config);
  EXPECT_EQ(result.success, false);
}

TEST_F(ManagerInitialTest, ValidatingNonRegisteredAgentConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  ;
  auto result = manager.validateAgentConfig("Non-registered_agent", config);
  EXPECT_EQ(result.success, false);
}

class EndpointManagementTest : public testing::Test, public ManagerTestSetup
{
protected:
  std::vector<std::string> endpoints_name{ "ep1", "ep2", "ep3", "ep4" };
  void SetUp() override
  {
    for (size_t i = 0; i < endpoints_name.size(); i++) {
      manager.addMavlinkEndpoint(endpoints_name[i]);
    }
  }
  void TearDown() override {}
};

// TEST - [ EndpointsManagementTest ]
// # Adding a new endpoint with distinct name should succeed.
// # Adding a new endpoint with same name should fail.
// # With a MockTransport, validating the config should work
// # With a non-registered Transport, validating the config should fail
// - Connecting to a non-existant endpoint should not work
// - Connecting to a MockTransport should work
// - Connecting to a non-registered Transport should not work
// - Getting endpoint state of an existing endpoint should work
// - Getting endpoint state of a non-existing endpoint should not work
// - Getting all endpoints' states should work (Mix of connected and disconnected)
// - Disconnecting an existing endpoint should work
// - Disconnecting a non-existing endpoint should not work
// - Removing existing endpoints should work
// - Removing non-existing endpoints should return false

TEST_F(EndpointManagementTest, GettingEndpointListShouldBeEqualToAddedEndpoints)
{
  auto endpoint_list = manager.getMavlinkEndpointList();
  for (size_t i = 0; i < endpoint_list.size(); i++) {
    std::string& ep_name = endpoint_list[i];
    bool found = false;
    for (size_t j = 0; j < endpoints_name.size(); j++) {
      if (ep_name == endpoints_name[j]) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
}

TEST_F(EndpointManagementTest, ConnectingNonExistingEndpointShouldResultFalse)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.connectMavlinkEndpoint("non_existing_endpoint", g_mock_transport_name, config);
  EXPECT_EQ(result.success, false);
}

TEST_F(EndpointManagementTest, ConnectingExistingEndpointShouldResultTrue)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.connectMavlinkEndpoint(endpoints_name[0], g_mock_transport_name, config);
  EXPECT_EQ(result.success, true);
}

TEST_F(EndpointManagementTest, ConnectingToUnregisteredTransportShouldResultFalse)
{
  std::unordered_map<std::string, std::string> config{
    {"a_required_string", ""}
  };
  auto result = manager.connectMavlinkEndpoint(endpoints_name[0], "non_existing_transport", config);
  EXPECT_EQ(result.success, false);
}

TEST_F(EndpointManagementTest, GettingANonExistingEndpointStateShouldReturnNullopt)
{
  std::optional<MavlinkEndpointState> state = manager.getMavlinkEndpointState("non_existing_endpoint");
  EXPECT_EQ(state, std::nullopt);
  EXPECT_EQ(state.has_value(), false);
}

TEST_F(EndpointManagementTest, GettingExistingEndpointStateShouldNotReturnNullopt)
{
  auto endpoint_list = manager.getMavlinkEndpointList();
  for (auto& ep_name : endpoint_list) {
    std::optional<MavlinkEndpointState> state = manager.getMavlinkEndpointState(ep_name);
    EXPECT_NE(state, std::nullopt);
    EXPECT_EQ(state.has_value(), true);
  }
}

TEST_F(EndpointManagementTest, getMavlinkEndpointStateAllShouldReturnAllEndpointsState)
{
  auto endpoint_list = manager.getMavlinkEndpointStateAll();
  for (auto& name : endpoints_name) {
    auto it = endpoint_list.find(name);
    ASSERT_NE(it, endpoint_list.end());
  }
  for (auto& [ep_name, state] : endpoint_list) {
    bool found = false;
    for (auto& name : endpoints_name) {
      if (ep_name == name) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
}

TEST_F(EndpointManagementTest, DisconnectAnExistingEndpointShouldWork)
{
  manager.connectMavlinkEndpoint(endpoints_name[0], g_mock_transport_name, std::unordered_map<std::string, std::string>());
  auto result = manager.disconnectMavlinkEndpoint(endpoints_name[0]);
  EXPECT_EQ(result.success, true);
}

TEST_F(EndpointManagementTest, DisconnectANonExistingEndpointShouldResultFalse)
{
  auto result = manager.disconnectMavlinkEndpoint("non_existing_endpoint");
  EXPECT_EQ(result.success, false);
}

TEST_F(EndpointManagementTest, RemoveAnExistingEndpointShouldWork)
{
  auto result = manager.removeMavlinkEndpoint(endpoints_name[0]);
  EXPECT_EQ(result.success, true);
}

TEST_F(EndpointManagementTest, RemoveANonExistingEndpointShouldResultFalse)
{
  auto result = manager.removeMavlinkEndpoint("non_existing_endpoint");
  EXPECT_EQ(result.success, false);
}

// TEST - [ AgentsManagementTest ]
// - Getting agent list should return all added agents
// - Starting an existing agent should work
// - Starting a non-existing agent should not work
// - Getting agent state of an existing agent should work
// - Getting agent state of a non-existing agent should not work
// - Getting all agents' states should return all added agents' state
// - Stopping an existing agent should work
// - Stopping a non-existing agent should not work
// - Removing existing agents should work
// - Removing non-existing agents should report something.
class AgentManagementTest : public testing::Test, public ManagerTestSetup
{
protected:
  std::unordered_map<std::string, std::string> agent_config;
  std::vector<std::string> agents_name{ "ag1", "ag2", "ag3", "ag4" };
  void SetUp() override
  {
    agent_config["a_required_string"] = "";
    for (size_t i = 0; i < agents_name.size(); i++) {
      manager.addAgent(agents_name[i], g_mock_agent_name, agent_config);
    }
  }
  void TearDown() override {}
};

TEST_F(AgentManagementTest, GettingAgentListShouldReturnAllAddedAgents)
{
  std::vector<std::string> agent_list = manager.getAgentList();
  for (auto& ag_name : agent_list) {
    bool found = false;
    for (auto& added_agent_name : agents_name) {
      if (ag_name == added_agent_name) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
}

TEST_F(AgentManagementTest, StartExistingAgentsShouldWork)
{
  std::vector<std::string> agent_list = manager.getAgentList();
  for (auto& ag_name : agent_list) {
    auto result = manager.startAgent(ag_name);
    EXPECT_EQ(result.success, true);
  }
}

TEST_F(AgentManagementTest, StartANonExistingAgentShouldReturnFalse)
{
  auto result = manager.startAgent("non_existing_agent");
  EXPECT_EQ(result.success, false);
}

TEST_F(AgentManagementTest, GettingExistingAgentStateShouldNotReturnNullopt)
{
  auto agent_list = manager.getAgentList();
  for (auto& ep_name : agent_list) {
    std::optional<AgentState> state = manager.getAgentState(ep_name);
    EXPECT_NE(state, std::nullopt);
    EXPECT_EQ(state.has_value(), true);
  }
}

TEST_F(AgentManagementTest, GettingANonExistingAgentStateShouldReturnNullopt)
{
  std::optional<AgentState> state = manager.getAgentState("non_existing_agent");
  EXPECT_EQ(state, std::nullopt);
  EXPECT_EQ(state.has_value(), false);
}

TEST_F(AgentManagementTest, GettingAllAgentStateShouldReturnAllAddedAgentsState)
{
  auto agents_state = manager.getAgentStateAll();
  for (auto& added_agent_name : agents_name) {
    auto it = agents_state.find(added_agent_name);
    ASSERT_NE(it, agents_state.end());
  }
  for (auto& [ag_name, state] : agents_state) {
    bool found = false;
    for (auto& added_agent_name : agents_name) {
      if (ag_name == added_agent_name) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
}

TEST_F(AgentManagementTest, StopExistingAgentsShouldWork)
{
  std::vector<std::string> agent_list = manager.getAgentList();
  for (auto& ag_name : agent_list) {
    auto result = manager.stopAgent(ag_name);
    EXPECT_EQ(result.success, true);
  }
}

TEST_F(AgentManagementTest, StopANonExistingAgentShouldReturnFalse)
{
  auto result = manager.stopAgent("non_existing_agent");
  EXPECT_EQ(result.success, false);
}

TEST_F(AgentManagementTest, RemoveExistingAgentsShouldWork)
{
  std::vector<std::string> agent_list = manager.getAgentList();
  for (auto& ag_name : agent_list) {
    auto result = manager.removeAgent(ag_name);
    EXPECT_EQ(result.success, true);
  }
}

TEST_F(AgentManagementTest, RemoveANonExistingAgentShouldReturnFalse)
{
  auto result = manager.removeAgent("non_existing_agent");
  EXPECT_EQ(result.success, false);
}

// TEST - [ UserPlanValidationTest ]
// - Given a non-empty manager and a user plan with endpoint and agent name already exists in the manager, validation should report that the
//   name already exists.
// - Given a user plan with unregistered type entry, validation should report that the type is not registered.
// - Given a user plan with registered type but invalid config, validation should report that the config is not valid.
class UserPlanValidationTest : public testing::Test, public ManagerTestSetup
{
protected:
  std::string existing_ep_name;
  std::string existing_ag_name;
  void SetUp() override
  {
    existing_ag_name = "existing_ag_name";
    existing_ep_name = "existing_ep_name";
    manager.addMavlinkEndpoint(existing_ep_name);
    manager.addAgent(existing_ag_name, g_mock_agent_name,
                     {
                         {"a_required_string", ""}
    });
  }
  void TearDown() override {}
};

TEST_F(UserPlanValidationTest, IfEndpointNameAlreadyExistsValidationShouldReportFalse)
{
  UserPlan plan;

  EndpointEntry ep_entry;
  ep_entry.type = g_mock_transport_name;
  ep_entry.config["a_required_string"] = "";
  plan.endpoint_list.emplace(existing_ep_name, ep_entry);

  ExecutionResultList result = manager.validatePlan(plan);
  ASSERT_NE(result.endpoint_plan_result.find(existing_ep_name), result.endpoint_plan_result.end());
  EXPECT_EQ(result.endpoint_plan_result[existing_ep_name].success, false);
}

TEST_F(UserPlanValidationTest, IfAgentNameAlreadyExistsValidationShouldReportFalse)
{
  UserPlan plan;

  AgentEntry ag_entry;
  ag_entry.type = g_mock_agent_name;
  ag_entry.config["a_required_string"] = "";
  plan.agent_list.emplace(existing_ag_name, ag_entry);

  ExecutionResultList result = manager.validatePlan(plan);
  ASSERT_NE(result.agent_plan_result.find(existing_ag_name), result.agent_plan_result.end());
  EXPECT_EQ(result.agent_plan_result[existing_ag_name].success, false);
}

TEST_F(UserPlanValidationTest, IfEndpointTypeIsNotRegisteredValidationShouldReportFalse)
{
  UserPlan plan;

  EndpointEntry ep_entry;
  std::string plan_ep_name("ep1");
  ep_entry.type = "unregistered_type";
  ep_entry.config["a_required_string"] = "";
  plan.endpoint_list.emplace(plan_ep_name, ep_entry);

  ExecutionResultList result = manager.validatePlan(plan);
  EXPECT_EQ(result.endpoint_plan_result[plan_ep_name].success, false);
}

TEST_F(UserPlanValidationTest, IfAgentTypeIsNotRegisteredValidationShouldReportFalse)
{
  UserPlan plan;

  AgentEntry ag_entry;
  std::string plan_ag_name("ag1");
  ag_entry.type = "unregistered_type";
  ag_entry.config["a_required_string"] = "";
  plan.agent_list.emplace(plan_ag_name, ag_entry);

  ExecutionResultList result = manager.validatePlan(plan);
  EXPECT_EQ(result.agent_plan_result[plan_ag_name].success, false);
}

class UserPlanSetup
{
protected:
  UserPlan plan;
  std::vector<std::string> ep_name_list;
  std::vector<std::string> ag_name_list;
  UserPlanSetup();
};

UserPlanSetup::UserPlanSetup()
{
  ep_name_list = { "ep1", "ep2", "ep3", "ep4" };
  ag_name_list = { "ag1", "ag2", "ag3", "ag4" };

  AgentEntry ag_entry;
  ag_entry.type = g_mock_agent_name;
  ag_entry.config["a_required_string"] = "";
  EndpointEntry ep_entry;
  ep_entry.type = g_mock_transport_name;
  ep_entry.config["a_required_string"] = "";
  ep_entry.connect_on_create = false;

  for (auto& ag_name : ag_name_list) {
    plan.agent_list.emplace(ag_name, ag_entry);
  }
  for (auto& ep_name : ep_name_list) {
    plan.endpoint_list.emplace(ep_name, ep_entry);
  }
}
// TEST - [ UserPlanExecutionWithEmptyManagerTest ]
// - Given an empty manager and a user plan with all valid entries, all entries should be registered into the manager
// - Given an empty manager and a user plan with some valid entries with BEST_EFFORT policy, all valid entries should be registered into the
//   manager and the invalid entries should not be registered into the manager
// - Given an empty manager and a user plan with some valid entries with DISCARD policy, all entries should not be registered into the
//   manager.
class UserPlanExecutionWithEmptyManagerTest : public testing::Test, public ManagerTestSetup, public UserPlanSetup
{
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(UserPlanExecutionWithEmptyManagerTest, ExecutingBestEffortPolicyValidUserPlanShouldSucceed)
{
  plan.policy = UserPlanPolicy::BEST_EFFORT;
  ExecutionResultList result = manager.executePlan(plan);

  // Check if names in the plan are registered
  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }

  // Check if the registered entries contain only the names in the plan
  for (auto& [ag_name, ag_result] : result.agent_plan_result) {
    bool found = false;
    for (auto& ag_name_plan : ag_name_list) {
      if (ag_name == ag_name_plan) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
  for (auto& [ep_name, ep_result] : result.endpoint_plan_result) {
    bool found = false;
    for (auto& ep_name_plan : ep_name_list) {
      if (ep_name == ep_name_plan) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
}

TEST_F(UserPlanExecutionWithEmptyManagerTest, ExecutingDiscardPolicyValidUserPlanShouldSucceed)
{
  plan.policy = UserPlanPolicy::DISCARD;
  ExecutionResultList result = manager.executePlan(plan);

  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
}

TEST_F(UserPlanExecutionWithEmptyManagerTest, ExecutingBestEffortPolicyValidUserPlanShouldRegisterValidEntriesOnly)
{
  plan.agent_list[ag_name_list[0]].type = "unregistered_type";
  plan.agent_list[ag_name_list[1]].config.erase("a_required_string");
  plan.endpoint_list[ep_name_list[2]].type = "unregistered_type";
  plan.endpoint_list[ep_name_list[3]].config.erase("a_required_string");
  plan.policy = UserPlanPolicy::BEST_EFFORT;
  ExecutionResultList result = manager.executePlan(plan);

  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    if (ag_name == ag_name_list[0] || ag_name == ag_name_list[1]) {
      EXPECT_EQ(it->second.success, false);

    } else {
      EXPECT_EQ(it->second.success, true);
    }
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    if (ep_name == ep_name_list[2] || ep_name == ep_name_list[3]) {
      EXPECT_EQ(it->second.success, false);

    } else {
      EXPECT_EQ(it->second.success, true);
    }
  }

  EXPECT_EQ(manager.getAgentList().size(), 2);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 2);
}

TEST_F(UserPlanExecutionWithEmptyManagerTest, ExecutingDiscardPolicyValidUserPlanShouldNotRegisterAnything)
{
  plan.agent_list[ag_name_list[0]].type = "unregistered_type";
  plan.agent_list[ag_name_list[1]].config.erase("a_required_string");
  plan.endpoint_list[ep_name_list[2]].type = "unregistered_type";
  plan.endpoint_list[ep_name_list[3]].config.erase("a_required_string");
  plan.policy = UserPlanPolicy::DISCARD;
  ExecutionResultList result = manager.executePlan(plan);

  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, false);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    EXPECT_EQ(it->second.success, false);
  }

  EXPECT_EQ(manager.getAgentList().size(), 0);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 0);
}

// TEST - [ UserPlanExecutionWithNonEmptyManagerTest ]
// - Given a non-empty manager and a user plan with all valid entries, all entries should be registered into the manager
// - given a non-empty manager and a user plan with all valid entries but has name collision with existing entries with BEST_EFFORT policy,
//   existing entries will not be removed, non-colliding entries should be added into the manager, and new entries that have name collision
//   will not be added.
// - Given a non-empty manager and a BEST_EFFORT user plan with invalid entries and name collision to existing entries, existing entries
//   will be kept and no new entries are added into the manager.
// - Given a non-empty manager and a DISCARD user plan with invalid entries and name collision to existing entries, existing entries will be
//   kept and no new entries are added into the manager.

class UserPlanExecutionWithNonEmptyManagerTest : public testing::Test, public ManagerTestSetup, public UserPlanSetup
{
protected:
  std::vector<std::string> manager_init_ep_list{ "m_ep1", "m_ep2", "m_ep3", "m_ep4" };
  std::vector<std::string> manager_init_ag_list{ "m_ag1", "m_ag2", "m_ag3", "m_ag4" };
  void SetUp() override
  {
    for (auto& name : manager_init_ep_list) {
      manager.addMavlinkEndpoint(name);
    }
    for (auto& name : manager_init_ag_list) {
      manager.addAgent(name, g_mock_agent_name,
                       {
                           {"a_required_string", ""}
      });
    }
  }
  void TearDown() override {}
};

TEST_F(UserPlanExecutionWithNonEmptyManagerTest, ExecutingBestEffortPolicyValidUserPlanShouldRegisterAllEntries)
{
  plan.policy = UserPlanPolicy::BEST_EFFORT;
  ExecutionResultList result = manager.executePlan(plan);
  // Check if names in the plan are registered
  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
}

TEST_F(UserPlanExecutionWithNonEmptyManagerTest, ExecutingDiscardPolicyValidUserPlanShouldRegisterAllEntries)
{
  plan.policy = UserPlanPolicy::DISCARD;
  ExecutionResultList result = manager.executePlan(plan);
  // Check if names in the plan are registered
  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
}

TEST_F(UserPlanExecutionWithNonEmptyManagerTest, ExecutingBestEffortPolicyWithSomeInvalidUserPlanShouldRegisterValidEntriesOnly)
{
  plan.endpoint_list["m_ep1"] = plan.endpoint_list[ep_name_list[2]];
  plan.endpoint_list.erase(ep_name_list[2]);
  ep_name_list[2] = "m_ep1";
  plan.endpoint_list[ep_name_list[3]].config.erase("a_required_string");
  plan.policy = UserPlanPolicy::BEST_EFFORT;
  ExecutionResultList result = manager.executePlan(plan);
  // Check if names in the plan are registered
  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, true);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    if (ep_name == ep_name_list[2] || ep_name == ep_name_list[3]) {
      EXPECT_EQ(it->second.success, false);

    } else {
      EXPECT_EQ(it->second.success, true);
    }
  }
}

TEST_F(UserPlanExecutionWithNonEmptyManagerTest, ExecutingDiscardPolicyWithSomeInvalidUserPlanShouldNotRegisterAnything)
{
  plan.endpoint_list["m_ep1"] = plan.endpoint_list[ep_name_list[2]];
  plan.endpoint_list.erase(ep_name_list[2]);
  ep_name_list[2] = "m_ep1";
  plan.endpoint_list[ep_name_list[3]].config.erase("a_required_string");
  plan.policy = UserPlanPolicy::DISCARD;
  ExecutionResultList result = manager.executePlan(plan);
  // Check if names in the plan are registered
  for (auto& ag_name : ag_name_list) {
    auto it = result.agent_plan_result.find(ag_name);
    ASSERT_NE(it, result.agent_plan_result.end());
    EXPECT_EQ(it->second.success, false);
  }
  for (auto& ep_name : ep_name_list) {
    auto it = result.endpoint_plan_result.find(ep_name);
    ASSERT_NE(it, result.endpoint_plan_result.end());
    EXPECT_EQ(it->second.success, false);
  }
}

class MavlinkEndpointUserManagementTest : public testing::Test, public ManagerTestSetup
{
protected:
  std::string requester_name = "the_requester";
  std::string endpoint_name = "the_endpoint";
  void SetUp() override { manager.addMavlinkEndpoint(endpoint_name); }
  void TearDown() override {}
};

TEST_F(MavlinkEndpointUserManagementTest, CreatingEndpointUserShouldIncreaseUserCount)
{
  std::unique_ptr<IMavlinkEndpointUser> ep_user1 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 1);
  std::unique_ptr<IMavlinkEndpointUser> ep_user2 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 2);
}

TEST_F(MavlinkEndpointUserManagementTest, RemovingEndpointUserShouldDecreaseUserCount)
{
  std::unique_ptr<IMavlinkEndpointUser> ep_user1 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  std::unique_ptr<IMavlinkEndpointUser> ep_user2 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  std::unique_ptr<IMavlinkEndpointUser> ep_user3 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 3);
  manager.removeMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 2);
  manager.removeMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 1);
  manager.removeMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 0);
}

TEST_F(MavlinkEndpointUserManagementTest, DeletingEndpointUserShouldDecreaseUserCount)
{
  std::unique_ptr<IMavlinkEndpointUser> ep_user1 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  std::unique_ptr<IMavlinkEndpointUser> ep_user2 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  std::unique_ptr<IMavlinkEndpointUser> ep_user3 = manager.createMavlinkEndpointUser(endpoint_name, requester_name);
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 3);
  ep_user1.reset();
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 2);
  ep_user2.reset();
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 1);
  ep_user3.reset();
  EXPECT_EQ(manager.getMavlinkEndpointUserList(endpoint_name).value()[requester_name], 0);
}

// TEST - [ ManagerResourceRequesterTest ]
// - Given a manager already containing endpoints:
//   - createMavlinkEndpointUser through requestMavlinkEndpoint should not be null if endpoint_name exists within the manager
//   - createMavlinkEndpointUser through requestMavlinkEndpoint should be null if endpoint_name does not exist within the manager
// - Given a manager already containing endpoints:
//   - getMavlinkEndpointUserList should return the correct number of users.
class ManagerResourceRequesterTest : public testing::Test, public ManagerTestSetup
{
protected:
  std::vector<std::string> ep_name_list{ "m_ep1", "m_ep2", "m_ep3", "m_ep4" };
  std::vector<std::string> requester_names{ "requester1", "requester2", "requester3", "requester4" };
  std::vector<ManagerResourceRequester> requesters;
  void SetUp() override
  {
    // add endpoints
    for (auto& name : ep_name_list) {
      manager.addMavlinkEndpoint(name);
    }

    // add ManagerResourceRequesters
    for (auto& name : requester_names) {
      requesters.push_back(ManagerResourceRequester(name, &manager));
    }
  }
  void TearDown() override {}
};

TEST_F(ManagerResourceRequesterTest, InitialUserShouldBeZeroForEachEndpoints)
{
  for (auto& name : ep_name_list) {
    auto result = manager.getMavlinkEndpointUserList(name);
    ASSERT_NE(result, std::nullopt);
    EXPECT_EQ(result.value().size(), 0);
  }
}

TEST_F(ManagerResourceRequesterTest, RequestMavlinkEndpointShouldNotBeNullIfEndpointExists)
{
  std::unique_ptr<IMavlinkEndpointUser> ep = requesters[0].requestMavlinkEndpoint(ep_name_list[0]);
  EXPECT_NE(ep, nullptr);
}

TEST_F(ManagerResourceRequesterTest, RequestMavlinkEndpointShouldBeNullIfEndpointDoesNotExist)
{
  std::unique_ptr<IMavlinkEndpointUser> ep = requesters[0].requestMavlinkEndpoint("not_existing_endpoint");
  EXPECT_EQ(ep, nullptr);
}

TEST_F(ManagerResourceRequesterTest, GettingNumberOfMavlinkEndpointUserFromANonExistingEndpointShouldNullopt)
{
  auto result = manager.getMavlinkEndpointUserList("not_existing_endpoint");
  EXPECT_EQ(result, std::nullopt);
}

TEST_F(ManagerResourceRequesterTest, GettingNumberOfMavlinkEndpointUserShouldGiveTheCorrectNumber)
{
  std::unique_ptr<IMavlinkEndpointUser> ep1 = requesters[0].requestMavlinkEndpoint(ep_name_list[0]);
  auto user_list_optional = manager.getMavlinkEndpointUserList(ep_name_list[0]);
  auto user_list = user_list_optional.value();
  EXPECT_EQ(user_list[requesters[0].getName()], 1);
  std::unique_ptr<IMavlinkEndpointUser> ep2 = requesters[0].requestMavlinkEndpoint(ep_name_list[0]);
  user_list = manager.getMavlinkEndpointUserList(ep_name_list[0]).value();
  EXPECT_EQ(user_list[requesters[0].getName()], 2);
  ep1.reset();
  user_list = manager.getMavlinkEndpointUserList(ep_name_list[0]).value();
  EXPECT_EQ(user_list[requesters[0].getName()], 1);
  ep1 = requesters[1].requestMavlinkEndpoint(ep_name_list[0]);
  user_list = manager.getMavlinkEndpointUserList(ep_name_list[0]).value();
  EXPECT_EQ(user_list.size(), 2);
  EXPECT_EQ(user_list[requesters[0].getName()], 1);
  EXPECT_EQ(user_list[requesters[1].getName()], 1);
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}