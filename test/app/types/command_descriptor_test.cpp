#include "app/types/CommandDescriptor.h"

#include <gtest/gtest.h>
#include <set>
#include <string>

using pendarlab::app::mavlink_hub::commandDescriptors;
using pendarlab::app::mavlink_hub::UserCommandType;

namespace
{
  constexpr int kExpectedCommandCount = 23;
}

class CommandDescriptorTest : public testing::Test
{
protected:
  void SetUp() override {}
};

TEST_F(CommandDescriptorTest, HasOneEntryPerUserCommandType)
{
  EXPECT_EQ(commandDescriptors().size(), static_cast<std::size_t>(kExpectedCommandCount));
}

TEST_F(CommandDescriptorTest, EntriesAreOrderedLikeTheEnum)
{
  const auto& descriptors = commandDescriptors();
  ASSERT_GE(descriptors.size(), 1u);
  EXPECT_EQ(descriptors[0].type, UserCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(descriptors[static_cast<std::size_t>(UserCommandType::GET_AGENT_STATUS_ALL)].type, UserCommandType::GET_AGENT_STATUS_ALL);
}

TEST_F(CommandDescriptorTest, NamesAreUniqueAndNonEmpty)
{
  std::set<std::string> names;
  for (const auto& descriptor : commandDescriptors()) {
    EXPECT_FALSE(descriptor.name.empty());
    EXPECT_TRUE(names.insert(std::string(descriptor.name)).second) << "duplicate name: " << descriptor.name;
  }
  EXPECT_EQ(names.size(), commandDescriptors().size());
}

TEST_F(CommandDescriptorTest, PayloadHintIsEmptyWhenNoPayloadRequired)
{
  for (const auto& descriptor : commandDescriptors()) {
    if (!descriptor.requires_payload) {
      EXPECT_TRUE(descriptor.payload_hint.empty()) << "hint should be empty for: " << descriptor.name;
    }
  }
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
