#include "agent_registry/AgentRegistry.h"
#include "agent_registry/AgentRegistryUserAccess.h"
#include "app/app_service/AppService.h"
#include "app/startup/AppConfigurator.h"
#include "app/types/AppConfig.h"
#include "app/ui_handler/CliUiController.h"
#include "byte_transport/Registry.h"
#include "byte_transport/RegistryUserAccess.h"
#include "gtest/gtest.h"
#include "manager/Manager.h"
#include "mock_agent/MockAgentDefinition.h"
#include "mock_byte_transport/MockByteTransportDefinition.h"
#include "mock_lib_loader/MockLibLoader.h"

#include <memory>
#include <string>

using namespace pendarlab::app::mavlink_hub;
using test::MockAgentDefinition;
using test::MockByteTransportDefinition;
using test::MockLibLoader;

using TransportRegistry = pendarlab::lib::comm::byte_transport::Registry;
using TransportRegistryUser = pendarlab::lib::comm::byte_transport::RegistryUserAccess;
using AgentRegistry = pendarlab::app::mavlink_hub::AgentRegistry;
using AgentRegistryUser = pendarlab::app::mavlink_hub::AgentRegistryUserAccess;

namespace
{
  const std::string test_plan_dir = TEST_PLAN_DIR;
  const std::string g_mock_transport_name = "mock_byte_transport";
  const std::string g_mock_agent_name = "mock_agent";

  static MockAgentDefinition g_mock_agent_def;
  static MockByteTransportDefinition g_mock_byte_transport_def;

  // Wires registries with the mock agent/transport definitions, a real Manager,
  // a real AppService, and a MockLibLoader - mirroring what App::run() builds so
  // the startup plan path (load -> apply) can be exercised for real.
  class CliUiControllerPlanTest : public testing::Test
  {
  protected:
    CliUiControllerPlanTest() :
        transport_registry_user(transport_registry.createUser()),
        agent_registry_user(agent_registry.createUser()),
        manager(*agent_registry_user, *transport_registry_user),
        app_service(manager, lib_loader)
    {
      transport_registry.addTransportDefinition(g_mock_transport_name, g_mock_byte_transport_def);
      agent_registry.addAgentDefinition(g_mock_agent_name, g_mock_agent_def);
    }

    void SetUp() override {}
    void TearDown() override {}

    TransportRegistry transport_registry;
    std::unique_ptr<TransportRegistryUser> transport_registry_user;
    AgentRegistry agent_registry;
    std::unique_ptr<AgentRegistryUser> agent_registry_user;
    MockLibLoader lib_loader;
    Manager manager;
    AppService app_service;
  };

  AppConfig makeStartupConfig(const std::string& plan_path)
  {
    AppConfig config;
    config.path_to_startup_user_plan = plan_path;
    config.apply_user_plan_on_startup = true;
    return config;
  }
} // namespace

// The startup plan scenario: a user plan is loaded (and applied) during startup,
// and then the controller's startup population reflects the agents and endpoints
// that the plan introduced. This exercises reading the plan file, parsing the
// plan JSON, and converting it to a UserPlan as part of the test.
TEST_F(CliUiControllerPlanTest, OverviewReflectsAgentsAndEndpointsFromStartupLoadedPlan)
{
  // Load and apply the plan during startup, exactly as App::run() does.
  const auto result = startup::applyConfig(makeStartupConfig(test_plan_dir + "startup_plan.json"), lib_loader, app_service);
  ASSERT_EQ(result.success, true);

  // Sanity check that the plan really introduced the expected entities.
  EXPECT_EQ(manager.getAgentList().size(), 2u);
  EXPECT_EQ(manager.getMavlinkEndpointList().size(), 2u);

  CliUiController controller(app_service);
  controller.updateOverviewData();

  auto find_status = [](const std::vector<OverviewRow>& rows, const std::string& name) -> std::string {
    for (const auto& row : rows) {
      if (row.name == name) {
        return row.status;
      }
    }
    return "<missing>";
  };

  // Agents created by the plan are IDLE; their row shows the lowercase status
  // and empty Type/Report.
  ASSERT_EQ(controller.agentRows().size(), 2u);
  EXPECT_EQ(find_status(controller.agentRows(), "ag_iris"), "idle");
  EXPECT_EQ(find_status(controller.agentRows(), "ag_heartbeat"), "idle");
  for (const auto& row : controller.agentRows()) {
    EXPECT_EQ(row.type, "");
    EXPECT_EQ(row.report, "");
  }

  // Endpoints created by the plan are DISCONNECTED (connect_on_create=false).
  ASSERT_EQ(controller.endpointRows().size(), 2u);
  EXPECT_EQ(find_status(controller.endpointRows(), "ep_serial"), "disconnected");
  EXPECT_EQ(find_status(controller.endpointRows(), "ep_udp"), "disconnected");
  for (const auto& row : controller.endpointRows()) {
    EXPECT_EQ(row.type, "");
    EXPECT_EQ(row.report, "");
  }
}

// The startup population also reflects agents/endpoints the plan started
// (start_on_create drives an agent from IDLE into a non-IDLE state), showing
// that the status value comes from the hub's live state, not a hardcoded one.
TEST_F(CliUiControllerPlanTest, OverviewShowsStartedAgentStateFromStartupLoadedPlan)
{
  // Load a plan whose agent is started on create.
  const auto result = startup::applyConfig(makeStartupConfig(test_plan_dir + "startup_plan_start_agent.json"), lib_loader, app_service);
  ASSERT_EQ(result.success, true);

  CliUiController controller(app_service);
  controller.updateOverviewData();

  ASSERT_EQ(controller.agentRows().size(), 1u);
  // The started MockAgent leaves IDLE (it reports ACTIVE while running).
  EXPECT_NE(controller.agentRows()[0].name, "");
  EXPECT_NE(controller.agentRows()[0].status, "idle");
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
