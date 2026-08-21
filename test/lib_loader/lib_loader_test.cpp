#include <agent_registry/AgentRegistry.h>
#include <app/lib_loader/LibLoader.h>
#include <byte_transport/Registry.h>
#include <common/types/OperationResult.h>
#include <gtest/gtest.h>

using TransportRegistry = pendarlab::lib::comm::byte_transport::Registry;
using AgentRegistry = pendarlab::app::mavlink_hub::AgentRegistry;
using LibLoader = pendarlab::app::mavlink_hub::LibLoader;
using OperationResult = pendarlab::app::mavlink_hub::OperationResult;
class LibLoaderTestSetup
{
public:
  LibLoaderTestSetup();

protected:
  TransportRegistry transport_registry;
  AgentRegistry agent_registry;
  LibLoader lib_loader;
};

LibLoaderTestSetup::LibLoaderTestSetup() : lib_loader(agent_registry, transport_registry)
{
}

class LibLoaderInitialTest : public testing::Test, public LibLoaderTestSetup
{
public:
  LibLoaderInitialTest();

protected:
  const std::string mock_agent_definition_sym_str;
  const std::string mock_transport_definition_sym_str;
  void SetUp() override {}
  void TearDown() override {}
};

LibLoaderInitialTest::LibLoaderInitialTest() :
    mock_agent_definition_sym_str("getMockAgentDefinition"), mock_transport_definition_sym_str("getMockTransportDefinition")
{
}

TEST_F(LibLoaderInitialTest, LoadAgentLibWithExistingLibAndCorrectSymbolShouldReturnSuccess)
{
  auto result = lib_loader.loadAgentLib("mock_agent", MOCK_AGENT_LIB_PATH, mock_agent_definition_sym_str);
  EXPECT_EQ(result.success, true);
}

TEST_F(LibLoaderInitialTest, LoadAgentLibWithExistingLibAndIncorrectSymbolShouldReturnFailed)
{
  auto result = lib_loader.loadAgentLib("mock_agent", MOCK_AGENT_LIB_PATH, "some_incorrect_symbol_name");
  EXPECT_EQ(result.success, false);
}

TEST_F(LibLoaderInitialTest, LoadAgentLibWithIncorrectLibPathAndCorrectSymbolShouldReturnFailed)
{
  auto result = lib_loader.loadAgentLib("mock_agent", "some_incorrect_path", mock_agent_definition_sym_str);
  EXPECT_EQ(result.success, false);
}

TEST_F(LibLoaderInitialTest, LoadTransportLibWithExistingLibAndCorrectSymbolShouldReturnSuccess)
{
  auto result = lib_loader.loadTransportLib("mock_byte_transport", MOCK_BYTE_TRANSPORT_LIB_PATH, mock_transport_definition_sym_str);
  EXPECT_EQ(result.success, true);
}

TEST_F(LibLoaderInitialTest, LoadTransportLibWithExistingLibAndIncorrectSymbolShouldReturnFailed)
{
  auto result = lib_loader.loadTransportLib("mock_byte_transport", MOCK_BYTE_TRANSPORT_LIB_PATH, "some_incorrect_symbol_name");
  EXPECT_EQ(result.success, false);
}

TEST_F(LibLoaderInitialTest, LoadTransportLibWithIncorrectLibPathAndCorrectSymbolShouldReturnFailed)
{
  auto result = lib_loader.loadTransportLib("mock_byte_transport", "some_incorrect_path", mock_transport_definition_sym_str);
  EXPECT_EQ(result.success, false);
}
/*
TEST:
- Given a correct full path to an existing lib that has the correct symbol, the result must be success
*/

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}