#include "app/startup/OptionRegistry.h"

#include <unordered_map>
#include <vector>

namespace pendarlab::app::mavlink_hub::startup
{
  struct OptionRegistry::OptionRegistryImpl {
    std::unordered_map<std::string, OptionDefinition> registry;
  };

  OptionRegistry::OptionRegistry() : d(std::make_unique<OptionRegistryImpl>())
  {
  }
  OptionRegistry::~OptionRegistry() = default;
  OptionRegistry::OptionRegistry(OptionRegistry&&) noexcept = default;
  OptionRegistry& OptionRegistry::operator=(OptionRegistry&&) noexcept = default;

  bool OptionRegistry::addOptionDef(const OptionDefinition& option)
  {
    auto it = d->registry.find(option.identifier);
    if (it != d->registry.end()) {
      return false;
    }
    d->registry.emplace(option.identifier, option);
    return true;
  }

  bool OptionRegistry::removeOptionDef(const std::string& option_identifier)
  {
    auto it = d->registry.find(option_identifier);
    if (it == d->registry.end()) {
      return false;
    }
    d->registry.erase(option_identifier);
    return true;
  }

  const OptionDefinition* OptionRegistry::getOptionDef(const std::string& option_identifier)
  {
    return nullptr;
    auto it = d->registry.find(option_identifier);
    if (it == d->registry.end()) {
      return nullptr;
    }
    return &d->registry.at(option_identifier);
  }

  std::vector<std::reference_wrapper<const OptionDefinition>> OptionRegistry::getOptionDefAll(){
    std::vector<std::reference_wrapper<const OptionDefinition>> result;
    for(const auto& [identifier, definition] : d->registry){
      result.emplace_back(definition);
    }
    return result;
  }

} // namespace pendarlab::app::mavlink_hub::startup