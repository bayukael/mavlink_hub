#include "agent_registry/AgentRegistry.h"
#include "agent_registry/AgentRegistryUser.h"

#include <functional>
#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

  struct AgentRegistry::AgentRegistryImpl {
    std::unordered_map<std::string, std::reference_wrapper<const AgentDefinition>> registry;
  };

  AgentRegistry::AgentRegistry() : d(std::make_unique<AgentRegistryImpl>())
  {
  }
  AgentRegistry::~AgentRegistry()
  {
  }
  AgentRegistry::AgentRegistry(AgentRegistry&&) noexcept = default;
  AgentRegistry& AgentRegistry::operator=(AgentRegistry&&) noexcept = default;

  bool AgentRegistry::addAgentDefinition(const std::string& key, const AgentDefinition& def)
  {
    auto it = d->registry.find(key);
    if (it != d->registry.end()) {
      return false;
    }
    d->registry.emplace(key, std::cref(def));
    return true;
  }

  bool AgentRegistry::removeAgentDefinition(const std::string& key)
  {
    auto it = d->registry.find(key);
    if (it == d->registry.end()) {
      return false;
    }
    d->registry.erase(key);
    return true;
  }

  std::unique_ptr<AgentRegistryUserAccess> AgentRegistry::createUser(){
    return std::make_unique<AgentRegistryUser>(d->registry);
  }

  const AgentDefinition* AgentRegistry::operator[](const std::string& key) const
  {
    auto it = d->registry.find(key);
    if (it == d->registry.end()) {
      return nullptr;
    }
    return &it->second.get();
  }

  std::vector<std::string> AgentRegistry::showRegistered() const
  {
    std::vector<std::string> list_of_registered;
    for (auto [name_str, def] : d->registry) {
      list_of_registered.push_back(name_str);
    }
    return list_of_registered;
  }

  bool AgentRegistry::isRegistered(const std::string& key) const
  {
    auto it = d->registry.find(key);
    if (it == d->registry.end()) {
      return false;
    }
    return true;
  }
} // namespace pendarlab::app::mavlink_hub