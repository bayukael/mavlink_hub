#include "manager/Manager.h"

#include "manager/ManagerResourceRequester.h"
#include "manager/mavlink_endpoint_user/MavlinkEndpointUser.h"

#include <mavlink_endpoint/MavlinkEndpoint.h>
#include <mavlink_hub_sdk/agent/Agent.h>
#include <mavlink_hub_sdk/agent/AgentState.h>
#include <mavlink_hub_sdk/manager_resource_requester/IManagerResourceRequester.h>
#include <mavlink_hub_sdk/mavlink_endpoint_user/IMavlinkEndpointUser.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  using Agent = pendarlab::sdk::mavlink_hub::Agent;
  using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
  using MavlinkEndpoint = pendarlab::lib::comm::MavlinkEndpoint;
  using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
  using IManagerResourceRequester = pendarlab::sdk::mavlink_hub::IManagerResourceRequester;
  using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  struct EndpointEntry {
    EndpointEntry(std::shared_ptr<MavlinkEndpoint> ep);
    std::shared_ptr<MavlinkEndpoint> endpoint;
    std::unordered_map<std::string, int> user_count;
  };

  EndpointEntry::EndpointEntry(std::shared_ptr<MavlinkEndpoint> ep) : endpoint(ep), user_count()
  {
  }

  struct Manager::ManagerImpl {
    ManagerImpl(const AgentRegistryUserAccess&, const TransportRegistryUserAccess&);
    const AgentRegistryUserAccess& agent_registry;
    const TransportRegistryUserAccess& transport_registry;
    std::unordered_map<std::string, EndpointEntry> endpoint_db;
    std::unordered_map<std::string, std::unique_ptr<Agent>> agents;
  };

  Manager::ManagerImpl::ManagerImpl(const AgentRegistryUserAccess& agent_reg, const TransportRegistryUserAccess& bt_reg) :
      agent_registry(agent_reg), transport_registry(bt_reg)
  {
  }

  Manager::Manager(const AgentRegistryUserAccess& agent_reg, const TransportRegistryUserAccess& bt_reg) :
      d(std::make_unique<ManagerImpl>(agent_reg, bt_reg))
  {
  }
  Manager::~Manager()
  {
  }
  Manager::Manager(Manager&&) noexcept = default;
  Manager& Manager::operator=(Manager&&) noexcept = default;

  ExecutionResultList Manager::executePlan(const UserPlan& plan, const UserPlanPolicy& policy)
  {
    ExecutionResultList result;

    ExecutionResultList validation_result = validatePlan(plan);
    bool discard_plan = false;

    // Checking if we need to discard the plan based on endpoint list
    for (auto& [name, spec] : plan.endpoint_list) {
      OperationResult validity = validation_result.endpoint_plan_result[name];
      if (policy == UserPlanPolicy::DISCARD) {
        if ((!validity.success)) {
          discard_plan = true;
          break;
        }
      }
    }
    // Checking if we need to discard the plan based on agent list
    for (auto& [name, spec] : plan.agent_list) {
      OperationResult validity = validation_result.agent_plan_result[name];
      if (policy == UserPlanPolicy::DISCARD) {
        if ((!validity.success)) {
          discard_plan = true;
          break;
        }
      }
    }

    // Executing the endpoint plan based on the validation result
    const std::unordered_map<std::string, MavlinkEndpointEntry>& plan_endpoint = plan.endpoint_list;
    for (auto& [ep_name, ep_spec] : plan_endpoint) {
      const OperationResult& validity = validation_result.endpoint_plan_result[ep_name];
      OperationResult exec_result;
      if (!validity.success) {
        exec_result = validation_result.endpoint_plan_result[ep_name];
      } else {
        if (discard_plan) { // If the plan has to be discarded
          exec_result.success = false;
          exec_result.messages.push_back("[Manager]: Endpoint [" + ep_name + "] is discarded because of the policy");
        } else {
          OperationResult add_result = addMavlinkEndpoint(ep_name);
          exec_result.merge(add_result);
          if (add_result.success && ep_spec.connect_on_create) {
            OperationResult connect_result = connectMavlinkEndpoint(ep_name, ep_spec.type, ep_spec.config);
            exec_result.merge(connect_result);
          }
        }
      }
      result.endpoint_plan_result[ep_name] = exec_result;
    }

    // Executing the agent plan based on the validation result
    const std::unordered_map<std::string, AgentEntry>& plan_agent = plan.agent_list;
    for (auto& [ag_name, ag_spec] : plan_agent) {
      const OperationResult& validity = validation_result.agent_plan_result[ag_name];
      OperationResult exec_result;
      if (!validity.success) {
        exec_result = validation_result.agent_plan_result[ag_name];
      } else {
        if (discard_plan) {
          OperationResult discard_report;
          discard_report.success = false;
          discard_report.messages.push_back("[Manager]: Agent [" + ag_name + "] is valid but the plan is discarded because of the policy");
          exec_result.merge(discard_report);
        } else {
          OperationResult add_result = addAgent(ag_name, ag_spec.type, ag_spec.config);
          exec_result.merge(add_result);
        }
      }
      result.agent_plan_result[ag_name] = exec_result;
    }

    return result;
  }

  ExecutionResultList Manager::validatePlan(const UserPlan& plan)
  {
    // TODO: Add feature to check name collision within the plan itself
    ExecutionResultList result;

    const std::unordered_map<std::string, MavlinkEndpointEntry>& plan_endpoint = plan.endpoint_list;
    for (auto& [ep_name, ep_spec] : plan_endpoint) {
      OperationResult entry_result;

      OperationResult name_availability_result;
      auto it = d->endpoint_db.find(ep_name);
      if (it == d->endpoint_db.end()) {
        name_availability_result.success = true;
        name_availability_result.messages.push_back("[Manager]: The name [" + ep_name +
                                                    "] is AVAILABLE to be used as a Mavlink Endpoint name.");
      } else {
        name_availability_result.success = false;
        name_availability_result.messages.push_back("[Manager]: The name [" + ep_name +
                                                    "] is NOT AVAILABLE to be used as a Mavlink Endpoint name.");
      }
      entry_result.merge(name_availability_result);

      OperationResult config_validation_result;
      if (d->transport_registry.isRegistered(ep_spec.type)) {
        auto parse_result = d->transport_registry[ep_spec.type]->parseConfig(ep_spec.config);
        if (parse_result.ok()) {
          config_validation_result.success = true;
          config_validation_result.messages.push_back("[Manager]: The endpoint type [" + ep_spec.type +
                                                      "] is registered and the given config is VALID.");
        } else {
          config_validation_result.success = false;
          config_validation_result.messages.push_back("Manager]: The endpoint type [" + ep_spec.type +
                                                      "] is registered HOWEVER the given config is INVALID.");
        }
      } else {
        config_validation_result.success = false;
        config_validation_result.messages.push_back("[Manager]: The endpoint type [" + ep_spec.type + "] is not registered.");
      }
      entry_result.merge(config_validation_result);
      result.endpoint_plan_result[ep_name] = entry_result;
    }

    const std::unordered_map<std::string, AgentEntry>& plan_agent = plan.agent_list;
    for (auto& [ag_name, ag_spec] : plan_agent) {
      OperationResult entry_result;

      OperationResult name_collision_result;
      auto it = d->agents.find(ag_name);
      if (it == d->agents.end()) {
        name_collision_result.success = true;
        name_collision_result.messages.push_back("[Manager]: The name [" + ag_name + "] is AVAILABLE to be used as an Agent name.");
      } else {
        name_collision_result.success = false;
        name_collision_result.messages.push_back("[Manager]: The name [" + ag_name + "] is NOT AVAILABLE to be used as an Agent name.");
      }
      entry_result.merge(name_collision_result);

      OperationResult config_validation_result;
      if (d->agent_registry.isRegistered(ag_spec.type)) {
        auto parse_result = d->agent_registry[ag_spec.type]->parseConfig(ag_spec.config);
        if (parse_result.ok()) {
          config_validation_result.success = true;
          config_validation_result.messages.push_back("[Manager]: The agent type [" + ag_spec.type +
                                                      "] is registered and the given config is VALID.");
        } else {
          config_validation_result.success = false;
          config_validation_result.messages.push_back("Manager]: The agent type [" + ag_spec.type +
                                                      "] is registered HOWEVER the given config is INVALID.");
        }
      } else {
        config_validation_result.success = false;
        config_validation_result.messages.push_back("[Manager]: The agent type [" + ag_spec.type + "] is not registered.");
      }
      entry_result.merge(config_validation_result);
      result.agent_plan_result[ag_name] = entry_result;
    }

    return result;
  }

  std::vector<std::string> Manager::getMavlinkEndpointList()
  {
    std::vector<std::string> list;
    for (auto& [name, ep] : d->endpoint_db) {
      list.push_back(name);
    }
    return list;
  }

  std::optional<MavlinkEndpointState> Manager::getMavlinkEndpointState(const std::string& name)
  {
    auto it = d->endpoint_db.find(name);
    if (it == d->endpoint_db.end()) {
      return std::nullopt;
    }
    return it->second.endpoint->getState();
  }

  std::unordered_map<std::string, MavlinkEndpointState> Manager::getMavlinkEndpointStateAll()
  {
    std::unordered_map<std::string, MavlinkEndpointState> states;
    for (auto& [name, ep] : d->endpoint_db) {
      states[name] = ep.endpoint->getState();
    }
    return states;
  }

  std::vector<std::string> Manager::getAgentList()
  {
    std::vector<std::string> list;
    for (auto& [name, agent] : d->agents) {
      list.push_back(name);
    }
    return list;
  }

  std::optional<AgentState> Manager::getAgentState(const std::string& name)
  {
    auto it = d->agents.find(name);
    if (it == d->agents.end()) {
      return std::nullopt;
    }
    return it->second->getState();
  }

  std::unordered_map<std::string, AgentState> Manager::getAgentStateAll()
  {
    std::unordered_map<std::string, AgentState> states;
    for (auto& [name, agent] : d->agents) {
      states[name] = agent->getState();
    }
    return states;
  }

  OperationResult Manager::validateMavlinkEndpointConfig(const std::string& type,
                                                         const std::unordered_map<std::string, std::string>& config)
  {
    OperationResult result;
    if (!d->transport_registry.isRegistered(type)) {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to validate a mavlink endpoint config with type: " + type +
                                ". The type does not exist.");
    } else {
      auto parse_result = d->transport_registry[type]->parseConfig(config);
      if (parse_result.ok()) {
        result.success = true;
        result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
      } else {
        result.success = false;
        result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
        result.messages.push_back("[Manager] : Failed to validate the given mavlink endpoint config with type: " + type);
      }
    }
    return result;
  }

  OperationResult Manager::addMavlinkEndpoint(const std::string& name)
  {
    OperationResult result;
    auto it = d->endpoint_db.find(name);
    if (it != d->endpoint_db.end()) {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to add a mavlink endpoint with name: " + name + ". It already exists.");
    } else {
      d->endpoint_db.emplace(name, EndpointEntry(MavlinkEndpoint::create(d->transport_registry)));
      result.success = true;
      result.messages.push_back("[Manager] : Mavlink Endpoint is created with name: " + name);
    }
    return result;
  }

  std::unique_ptr<IMavlinkEndpointUser> Manager::createMavlinkEndpointUser(const std::string& endpoint_name,
                                                                           const std::string& requester_name)
  {
    auto it = d->endpoint_db.find(endpoint_name);
    if (it == d->endpoint_db.end()) {
      return nullptr;
    }

    std::shared_ptr<MavlinkEndpoint>& endpoint = it->second.endpoint;
    std::unordered_map<std::string, int>& endpoint_users = it->second.user_count;
    auto it_requester_name = endpoint_users.find(requester_name);
    if (it_requester_name == endpoint_users.end()) {
      endpoint_users[requester_name] = 1;
    } else {
      endpoint_users[requester_name]++;
    }
    return std::make_unique<MavlinkEndpointUser>(this, endpoint, endpoint_name, requester_name);
  }

  bool Manager::removeMavlinkEndpointUser(const std::string& endpoint_name, const std::string& requester_name)
  {
    auto it = d->endpoint_db.find(endpoint_name);
    if (it != d->endpoint_db.end()) {
      std::unordered_map<std::string, int>& endpoint_users = it->second.user_count;
      auto it_requester_name = endpoint_users.find(requester_name);
      if (it_requester_name != endpoint_users.end()) {
        endpoint_users[requester_name]--;
        if (endpoint_users[requester_name] == 0) {
          endpoint_users.erase(requester_name);
        }
      }
    }

    return true;
  }

  std::optional<std::unordered_map<std::string, int>> Manager::getMavlinkEndpointUserList(const std::string& endpoint_name)
  {
    auto it = d->endpoint_db.find(endpoint_name);
    if (it == d->endpoint_db.end()) {
      return std::nullopt;
    }
    return it->second.user_count;
  }

  OperationResult Manager::connectMavlinkEndpoint(const std::string& endpoint_name, const std::string& type,
                                                  const std::unordered_map<std::string, std::string>& config)
  {
    OperationResult result;
    auto it = d->endpoint_db.find(endpoint_name);
    if (it == d->endpoint_db.end()) {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to connect to mavlink endpoint: " + endpoint_name + ". It does not exist.");
    } else {
      auto& endpoint = it->second.endpoint;
      if (endpoint->connect(type, config)) {
        result.success = true;
        result.messages.push_back("[Manager] : Mavlink endpoint [" + endpoint_name + "] is connected to [" + type + "]");
      } else {
        result.success = false;
        result.messages.push_back("[Manager] : Failed to connect to mavlink endpoint: " + endpoint_name + ". Please check the config.");
      }
    }

    return result;
  }

  OperationResult Manager::disconnectMavlinkEndpoint(const std::string& endpoint_name)
  {
    OperationResult result;
    auto it = d->endpoint_db.find(endpoint_name);
    if (it == d->endpoint_db.end()) {
      result.success = false;
      result.messages.push_back("[Manager] : Cannot find mavlink endpoint: " + endpoint_name);
    } else {
      auto& endpoint = it->second.endpoint;
      if (endpoint->disconnect()) {
        result.success = true;
        result.messages.push_back("[Manager] : Mavlink endpoint [" + endpoint_name + "] is successfully disconnected");
      } else {
        result.success = false;
        result.messages.push_back("[Manager] : Failed to disconnect to mavlink endpoint: " + endpoint_name);
      }
    }
    return result;
  }

  OperationResult Manager::removeMavlinkEndpoint(const std::string& endpoint_name)
  {
    OperationResult result;
    result.success = true;
    auto it = d->endpoint_db.find(endpoint_name);
    if (it == d->endpoint_db.end()) {
      result.success = false;
      result.messages.push_back("[Manager] : Cannot find mavlink endpoint: " + endpoint_name);
    } else {
      d->endpoint_db.erase(endpoint_name);
      result.success = true;
      result.messages.push_back("[Manager] : Mavlink endpoint [" + endpoint_name + "] is successfully removed");
    }
    return result;
  }

  OperationResult Manager::validateAgentConfig(const std::string& type, const std::unordered_map<std::string, std::string>& config)
  {
    OperationResult result;
    if (!d->agent_registry.isRegistered(type)) {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to validate an agent config with type: " + type + ". The type is not registered.");
    } else {
      auto parse_result = d->agent_registry[type]->parseConfig(config);
      if (parse_result.ok()) {
        result.success = true;
        result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
        result.messages.push_back("[Manager] : The given agent config is valid.");
      } else {
        result.success = false;
        result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
        result.messages.push_back("[Manager] : Failed to validate the given agent config with type: " + type);
      }
    }
    return result;
  }

  OperationResult Manager::addAgent(const std::string& name, const std::string& type,
                                    const std::unordered_map<std::string, std::string>& config)
  {
    OperationResult result;
    auto it = d->agents.find(name);
    if (it == d->agents.end()) {
      if (!d->agent_registry.isRegistered(type)) {
        result.success = false;
        result.messages.push_back("[Manager] : Failed to add agent [" + name + "]. The type is not registered.");
      } else {
        auto parse_result = d->agent_registry[type]->parseConfig(config);
        std::unique_ptr<Agent> p_agent = nullptr;
        if (parse_result.ok()) {
          p_agent = d->agent_registry[type]->create(parse_result.parsed.value(), std::make_unique<ManagerResourceRequester>(name, this));
        }

        if (p_agent) {
          d->agents[name] = std::move(p_agent);
          result.success = true;
          result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
          result.messages.push_back("[Manager] : Agent [" + name + "] is added.");
        } else {
          result.success = false;
          result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
          result.messages.push_back("[Manager] : Failed to add Agent [" + name + "]. Please check the config.");
        }
      }
    } else {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to add Agent [" + name + "]. It already exists.");
    }

    return result;
  }

  OperationResult Manager::editAgent(const std::string& name, const std::string& type,
                                     const std::unordered_map<std::string, std::string>& config)
  {
    OperationResult result;
    auto it = d->agents.find(name);
    if (it != d->agents.end()) {
      if (!d->agent_registry.isRegistered(type)) {
        result.success = false;
        result.messages.push_back("[Manager] : Failed to edit agent [" + name + "]. The type is not registered.");
      } else {
        auto parse_result = d->agent_registry[type]->parseConfig(config);
        bool configure_success = false;
        if (parse_result.ok()) {
          configure_success = d->agents[name]->configure(parse_result.parsed.value());
        }

        if (configure_success) {
          result.success = true;
          result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
          result.messages.push_back("[Manager] : Agent [" + name + "] config is modified.");
        } else {
          result.success = false;
          result.messages.insert(result.messages.end(), parse_result.messages.begin(), parse_result.messages.end());
          result.messages.push_back("[Manager] : Failed to edit Agent [" + name + "] config. Please check the config.");
        }
      }
    } else {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to edit Agent [" + name + "]. It does not exist.");
    }
    return result;
  }

  OperationResult Manager::startAgent(const std::string& name)
  {
    OperationResult result;
    auto it = d->agents.find(name);
    if (it != d->agents.end()) {
      bool start_success = d->agents[name]->start();
      result.success = true;
      if (start_success) {
        result.success = true;
        result.messages.push_back("[Manager] : Agent [" + name + "] is started.");
      } else {
        result.success = false;
        std::string reason;
        if (d->agents[name]->getState() != AgentState::IDLE) {
          reason = " Agent state is not IDLE.";
        } else {
          reason = "Unknown reason.";
        }
        result.messages.push_back("[Manager] : Failed to start Agent [" + name + "]." + reason);
      }
    } else {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to start Agent [" + name + "]. It does not exist.");
    }
    return result;
  }

  OperationResult Manager::stopAgent(const std::string& name)
  {
    OperationResult result;
    auto it = d->agents.find(name);
    if (it != d->agents.end()) {
      bool stop_success = d->agents[name]->stop();

      if (stop_success) {
        result.success = true;
        result.messages.push_back("[Manager] : Agent [" + name + "] is stopped.");
      } else {
        result.success = false;
        std::string reason;
        if (d->agents[name]->getState() == AgentState::STARTING) {
          reason = " Agent is STARTING.";
        } else {
          reason = "Unknown reason.";
        }
        result.messages.push_back("[Manager] : Failed to stop Agent [" + name + "]." + reason);
      }
    } else {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to stop Agent [" + name + "]. It does not exist.");
    }
    return result;
  }

  OperationResult Manager::removeAgent(const std::string& name)
  {
    OperationResult result;
    auto it = d->agents.find(name);
    if (it != d->agents.end()) {
      AgentState agent_state = d->agents[name]->getState();
      d->agents.erase(name);
      std::string agent_state_str;
      switch (agent_state) {
        case AgentState::IDLE: agent_state_str = "IDLE"; break;

        case AgentState::STARTING: agent_state_str = "STARTING"; break;

        case AgentState::ACTIVE: agent_state_str = "ACTIVE"; break;

        case AgentState::STOPPING: agent_state_str = "STOPPING"; break;

        default: break;
      }

      result.success = true;
      result.messages.push_back("[Manager] : Agent [" + name + "] is stopped. Its last state is: [" + agent_state_str + "]");
    } else {
      result.success = false;
      result.messages.push_back("[Manager] : Failed to remove Agent [" + name + "]. It does not exist.");
    }
    return result;
  }
} // namespace pendarlab::app::mavlink_hub