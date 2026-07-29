#include <agent_registry/AgentRegistry.h>
#include <agent_registry/AgentRegistryUserAccess.h>
#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUserAccess.h>
#include <gtest/gtest.h>
#include <manager/Manager.h>
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
using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
using AgentState = pendarlab::sdk::mavlink_hub::AgentState;

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

TEST_F(ManagerInitialTest, ValidatingRegisteredEndpointConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config;
  auto result = manager.validateMavlinkEndpointConfig(g_mock_transport_name, config);
  EXPECT_EQ(result.success, true);
}

TEST_F(ManagerInitialTest, ValidatingNonRegisteredEndpointConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config;
  auto result = manager.validateMavlinkEndpointConfig("Non-registered_transport", config);
  EXPECT_EQ(result.success, false);
}

TEST_F(ManagerInitialTest, AddingAgentWithUniqueNameShouldWork)
{
  std::unordered_map<std::string, std::string> config;
  auto result = manager.addAgent("ag1", g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getAgentList().size(), 1);
  result = manager.addAgent("ag2", g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getAgentList().size(), 2);
}

TEST_F(ManagerInitialTest, AddingAgentWithSameNameShouldNotWork)
{
  std::unordered_map<std::string, std::string> config;
  auto result = manager.addAgent("ag1", g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
  EXPECT_EQ(manager.getAgentList().size(), 1);
  result = manager.addAgent("ag1", g_mock_agent_name, config);
  EXPECT_EQ(result.success, false);
  EXPECT_EQ(manager.getAgentList().size(), 1);
}

TEST_F(ManagerInitialTest, ValidatingRegisteredAgentConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config;
  auto result = manager.validateAgentConfig(g_mock_agent_name, config);
  EXPECT_EQ(result.success, true);
}

TEST_F(ManagerInitialTest, ValidatingNonRegisteredAgentConfigShouldWork)
{
  std::unordered_map<std::string, std::string> config;
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
  auto result =
      manager.connectMavlinkEndpoint("non_existing_endpoint", g_mock_transport_name, std::unordered_map<std::string, std::string>());
  EXPECT_EQ(result.success, false);
}

TEST_F(EndpointManagementTest, ConnectingExistingEndpointShouldResultTrue)
{
  auto result = manager.connectMavlinkEndpoint(endpoints_name[0], g_mock_transport_name, std::unordered_map<std::string, std::string>());
  EXPECT_EQ(result.success, true);
}

TEST_F(EndpointManagementTest, ConnectingToUnregisteredTransportShouldResultFalse)
{
  auto result = manager.connectMavlinkEndpoint(endpoints_name[0], "non_existing_transport", std::unordered_map<std::string, std::string>());
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

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}