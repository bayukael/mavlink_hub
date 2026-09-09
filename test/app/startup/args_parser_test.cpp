#include "app/startup/ArgsParser.h"
#include "app/types/AppConfig.h"

#include <gtest/gtest.h>
#include <string>

using pendarlab::app::mavlink_hub::startup::ParseResult;
using pendarlab::app::mavlink_hub::startup::parseArgs;

namespace
{
  const std::string test_data_dir = TEST_DATA_DIR;
}

class ArgsParserTest : public testing::Test
{
protected:
  void SetUp() override {}
};

TEST_F(ArgsParserTest, NoArgsShouldReturnOkWithDefaultConfig)
{
  char arg0[] = "mavlink_hub";
  char* argv[] = { arg0 };
  ParseResult result = parseArgs(1, argv);

  EXPECT_EQ(result.status, ParseResult::Status::Ok);
  ASSERT_TRUE(result.config.has_value());
  EXPECT_TRUE(result.config->agent_lib_list.empty());
  EXPECT_TRUE(result.config->transport_lib_list.empty());
  EXPECT_TRUE(result.config->path_to_extra_lib_list.empty());
  EXPECT_TRUE(result.config->path_to_startup_user_plan.empty());
}

TEST_F(ArgsParserTest, ValidConfigFileShouldReturnOkWithPopulatedConfig)
{
  char arg0[] = "mavlink_hub";
  char arg1[] = "-c";
  std::string config_path = test_data_dir + "valid_config.json";
  char* argv[] = { arg0, arg1, const_cast<char*>(config_path.c_str()) };
  ParseResult result = parseArgs(3, argv);

  EXPECT_EQ(result.status, ParseResult::Status::Ok);
  ASSERT_TRUE(result.config.has_value());
  ASSERT_EQ(result.config->agent_lib_list.size(), 1u);
  EXPECT_EQ(result.config->agent_lib_list["mock_agent"].name, "mock_agent");
  ASSERT_EQ(result.config->transport_lib_list.size(), 1u);
  EXPECT_EQ(result.config->transport_lib_list["mock_transport"].name, "mock_transport");
}

TEST_F(ArgsParserTest, HelpShouldReturnShowHelpWithZeroExitCode)
{
  char arg0[] = "mavlink_hub";
  char arg1[] = "--help";
  char* argv[] = { arg0, arg1 };
  ParseResult result = parseArgs(2, argv);

  EXPECT_EQ(result.status, ParseResult::Status::ShowHelp);
  EXPECT_EQ(result.exit_code, 0);
}

TEST_F(ArgsParserTest, InvalidOptionShouldReturnShowHelpWithNonZeroExitCode)
{
  char arg0[] = "mavlink_hub";
  char arg1[] = "--bogus_option";
  char* argv[] = { arg0, arg1 };
  ParseResult result = parseArgs(2, argv);

  EXPECT_EQ(result.status, ParseResult::Status::ShowHelp);
  EXPECT_NE(result.exit_code, 0);
}

TEST_F(ArgsParserTest, NonexistentConfigFileShouldReturnFatalError)
{
  char arg0[] = "mavlink_hub";
  char arg1[] = "-c";
  char arg2[] = "/path/that/does/not/exist.json";
  char* argv[] = { arg0, arg1, arg2 };
  ParseResult result = parseArgs(3, argv);

  EXPECT_EQ(result.status, ParseResult::Status::FatalError);
  EXPECT_EQ(result.exit_code, 1);
}

TEST_F(ArgsParserTest, InvalidJsonConfigFileShouldReturnFatalError)
{
  char arg0[] = "mavlink_hub";
  char arg1[] = "-c";
  std::string config_path = test_data_dir + "invalid_config.json";
  char* argv[] = { arg0, arg1, const_cast<char*>(config_path.c_str()) };
  ParseResult result = parseArgs(3, argv);

  EXPECT_EQ(result.status, ParseResult::Status::FatalError);
  EXPECT_EQ(result.exit_code, 1);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
