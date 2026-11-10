#include "app/ui_handler/CliCommandDescriptor.h"

#include <gtest/gtest.h>
#include <string>

using pendarlab::app::mavlink_hub::CliCommandDescriptor;
using pendarlab::app::mavlink_hub::AppServiceCommandDescriptor;
using pendarlab::app::mavlink_hub::AppServiceCommandType;

// Build a small canonical descriptor to construct CliCommandDescriptor from.
AppServiceCommandDescriptor makeDescriptor()
{
  AppServiceCommandDescriptor descriptor;
  descriptor.type = AppServiceCommandType::GET_ENDPOINT_STATUS;
  descriptor.name = "get_endpoint_status";
  descriptor.requires_payload = true;
  descriptor.payload_hint = "endpoint name";
  return descriptor;
}

class CliCommandDescriptorTest : public testing::Test
{
protected:
  void SetUp() override { descriptor = makeDescriptor(); }

  AppServiceCommandDescriptor descriptor;
};

// The constructor mirrors every member of the AppServiceCommandDescriptor it is
// built from: type, name, requires_payload, and payload_hint.
TEST_F(CliCommandDescriptorTest, DerivesAllMembersFromAppServiceCommandDescriptor)
{
  const CliCommandDescriptor command(descriptor);
  EXPECT_EQ(command.type, descriptor.type);
  EXPECT_EQ(command.name, std::string(descriptor.name));
  EXPECT_EQ(command.requires_payload, descriptor.requires_payload);
  EXPECT_EQ(command.payload_hint, std::string(descriptor.payload_hint));
}

// An empty (default) name means "use the descriptor's name".
TEST_F(CliCommandDescriptorTest, EmptyNameUsesDescriptorName)
{
  const CliCommandDescriptor command(descriptor, "");
  EXPECT_EQ(command.name, std::string(descriptor.name));
}

// A non-empty name overrides the descriptor's name.
TEST_F(CliCommandDescriptorTest, NonEmptyNameOverridesDescriptorName)
{
  const CliCommandDescriptor command(descriptor, "my_display_name");
  EXPECT_EQ(command.name, "my_display_name");
}

// name is owned (a std::string), not a borrowed view into the descriptor.
TEST_F(CliCommandDescriptorTest, NameIsOwnedCopy)
{
  std::string source = std::string(descriptor.name);
  CliCommandDescriptor command(makeDescriptor());
  command.name = source;
  source = "mutated";
  EXPECT_EQ(command.name, "get_endpoint_status");
}

// description is intentionally left empty for now (wired later in MHP-039).
TEST_F(CliCommandDescriptorTest, DescriptionDefaultsToEmpty)
{
  const CliCommandDescriptor command(descriptor);
  EXPECT_EQ(command.description, "");
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
