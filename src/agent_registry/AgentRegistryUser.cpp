#include "agent_registry/AgentRegistryUser.h"

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <memory>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;
  struct AgentRegistryUser::AgentRegistryUserImpl {
    AgentRegistryUserImpl(const std::unordered_map<std::string, std::reference_wrapper<const AgentDefinition>>&);
    const std::unordered_map<std::string, std::reference_wrapper<const AgentDefinition>>& registry;
  };

  AgentRegistryUser::AgentRegistryUserImpl::AgentRegistryUserImpl(
      const std::unordered_map<std::string, std::reference_wrapper<const AgentDefinition>>& reg) :
      registry(reg)
  {
  }

  AgentRegistryUser::AgentRegistryUser(const std::unordered_map<std::string, std::reference_wrapper<const AgentDefinition>>& registry) :
      d(std::make_unique<AgentRegistryUserImpl>(registry))
  {
  }

  AgentRegistryUser::~AgentRegistryUser()
  {
  }

  AgentRegistryUser::AgentRegistryUser(AgentRegistryUser&&) noexcept = default;
  AgentRegistryUser& AgentRegistryUser::operator=(AgentRegistryUser&&) noexcept = default;

  const AgentDefinition* AgentRegistryUser::operator[](const std::string& key) const
  {
    auto it = d->registry.find(key);
    if (it == d->registry.end()) {
      return nullptr;
    }
    return &it->second.get();
  }

  std::vector<std::string> AgentRegistryUser::showRegistered() const
  {
    std::vector<std::string> list_of_registered;
    for (auto [name_str, transport_definition] : d->registry) {
      list_of_registered.push_back(name_str);
    }
    return list_of_registered;
  }

  bool AgentRegistryUser::isRegistered(const std::string& key) const
  {
    auto it = d->registry.find(key);
    if (it == d->registry.end()) {
      return false;
    }
    return true;
  }
} // namespace pendarlab::app::mavlink_hub