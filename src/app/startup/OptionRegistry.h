#pragma once

#include "app/startup/OptionDefinition.h"

#include <memory>
#include <vector>

namespace pendarlab::app::mavlink_hub::startup
{
  class OptionRegistry
  {
  public:
    OptionRegistry();
    ~OptionRegistry();
    OptionRegistry(OptionRegistry&&) noexcept;
    OptionRegistry& operator=(OptionRegistry&&) noexcept;

    bool addOptionDef(const OptionDefinition& option);
    bool removeOptionDef(const std::string& option_name);
    const OptionDefinition* getOptionDef(const std::string& option_name);
    std::vector<std::reference_wrapper<const OptionDefinition>> getOptionDefAll();

  private:
    struct OptionRegistryImpl;
    std::unique_ptr<OptionRegistryImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub::startup
