#include "app/app_service/AppServiceCommandDescriptor.h"

#include <gtest/gtest.h>
#include <set>
#include <string>

using pendarlab::app::mavlink_hub::appServiceCommandDescriptors;
using pendarlab::app::mavlink_hub::AppServiceCommandType;

namespace
{
  constexpr int kExpectedCommandCount = 25;
}

class AppServiceCommandDescriptorTest : public testing::Test
{
protected:
  void SetUp() override {}
};

TEST_F(AppServiceCommandDescriptorTest, HasOneEntryPerAppServiceCommandType)
{
  EXPECT_EQ(appServiceCommandDescriptors().size(), static_cast<std::size_t>(kExpectedCommandCount));
}

TEST_F(AppServiceCommandDescriptorTest, EntriesAreOrderedLikeTheEnum)
{
  const auto& descriptors = appServiceCommandDescriptors();
  ASSERT_GE(descriptors.size(), 1u);
  EXPECT_EQ(descriptors[0].type, AppServiceCommandType::LOAD_PLAN_FROM_PATH);
  EXPECT_EQ(descriptors[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_STATUS_ALL)].type, AppServiceCommandType::GET_AGENT_STATUS_ALL);
  EXPECT_EQ(descriptors[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT)].type, AppServiceCommandType::GET_AGENT_REPORT);
  EXPECT_EQ(descriptors[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT_ALL)].type, AppServiceCommandType::GET_AGENT_REPORT_ALL);
}

TEST_F(AppServiceCommandDescriptorTest, NamesAreUniqueAndNonEmpty)
{
  std::set<std::string> names;
  for (const auto& descriptor : appServiceCommandDescriptors()) {
    EXPECT_EQ(false, descriptor.name.empty());
    EXPECT_EQ(true, names.insert(std::string(descriptor.name)).second) << "duplicate name: " << descriptor.name;
  }
  EXPECT_EQ(names.size(), appServiceCommandDescriptors().size());
}

TEST_F(AppServiceCommandDescriptorTest, PayloadHintIsEmptyWhenNoPayloadRequired)
{
  for (const auto& descriptor : appServiceCommandDescriptors()) {
    if (!descriptor.requires_payload) {
      EXPECT_EQ(true, descriptor.payload_hint.empty()) << "hint should be empty for: " << descriptor.name;
    }
  }
}

TEST_F(AppServiceCommandDescriptorTest, DescriptionsArePopulatedAndWithinLimit)
{
  constexpr std::size_t kMaxDescriptionLength = 150;
  for (const auto& descriptor : appServiceCommandDescriptors()) {
    EXPECT_EQ(false, descriptor.description.empty()) << "description should be non-empty for: " << descriptor.name;
    EXPECT_LE(descriptor.description.size(), kMaxDescriptionLength)
        << "description too long for: " << descriptor.name;
  }
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
