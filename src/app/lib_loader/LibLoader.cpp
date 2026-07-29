#include "app/lib_loader/LibLoader.h"

#include <dlfcn.h>
#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <unordered_map>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;
  using TransportDefinition = pendarlab::lib::comm::byte_transport::TransportDefinition;

  struct LibLoader::LibLoaderImpl {
    LibLoaderImpl(AgentRegistryAdminAccess&, TransportRegistryAdminAccess&);
    ~LibLoaderImpl();
    AgentRegistryAdminAccess& agent_registry;
    TransportRegistryAdminAccess& transport_registry;
    std::unordered_map<std::string, void*> agent_lib_handles;
    std::unordered_map<std::string, void*> transport_lib_handles;
  };

  LibLoader::LibLoaderImpl::LibLoaderImpl(AgentRegistryAdminAccess& a_reg, TransportRegistryAdminAccess& t_reg) :
      agent_registry(a_reg), transport_registry(t_reg)
  {
  }

  LibLoader::LibLoaderImpl::~LibLoaderImpl(){
    for (auto [agent_def_name, lib_handle] : agent_lib_handles) {
      if (lib_handle) {
        dlclose(lib_handle);
      }
    }
    for (auto [transport_def_name, lib_handle] : transport_lib_handles) {
      if (lib_handle) {
        dlclose(lib_handle);
      }
    }

  }

  LibLoader::LibLoader(AgentRegistryAdminAccess& a_reg, TransportRegistryAdminAccess& t_reg) :
      d(std::make_unique<LibLoaderImpl>(a_reg, t_reg))
  {
  }

  LibLoader::~LibLoader()
  {
  }

  LibLoader::LibLoader(LibLoader&&) noexcept = default;
  LibLoader& LibLoader::operator=(LibLoader&&) noexcept = default;

  OperationResult LibLoader::loadAgentLib(const std::string& agent_def_name, const std::string& lib_path, const std::string& sym)
  {
    using GetAgentDefinition = const AgentDefinition* (*)();

    OperationResult result;
    void* lib_handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!lib_handle) {
      OperationResult open_result;
      open_result.success = false;
      open_result.messages.push_back("[LibLoader::loadAgentLib]: The lib - " + lib_path + " cannot be found");
      result.merge(open_result);
      return result;
    }

    auto lib_sym = dlsym(lib_handle, sym.c_str());
    if (!lib_sym) {
      OperationResult sym_result;
      sym_result.success = false;
      sym_result.messages.push_back("[LibLoader::loadAgentLib]: The lib " + lib_path + " is found but does not contain the symbol: " + sym);
      result.merge(sym_result);
      dlclose(lib_handle);
      return result;
    }

    auto getAgentDefinition = reinterpret_cast<GetAgentDefinition>(lib_sym);
    auto agent_definition = getAgentDefinition();
    if (!agent_definition) {
      OperationResult def_result;
      def_result.success = false;
      def_result.messages.push_back("[LibLoader::loadAgentLib]: The lib " + lib_path + " and the symbol " + sym +
                                    " are found but there is no AgentDefinition");
      result.merge(def_result);
      dlclose(lib_handle);
      return result;
    }

    OperationResult good_result;
    d->agent_registry.addAgentDefinition(agent_def_name, *agent_definition);
    d->agent_lib_handles.emplace(agent_def_name, lib_handle);
    good_result.success = true;
    good_result.messages.push_back("[LibLoader::loadAgentLib]: The symbol " + sym + " at lib " + lib_path +
                                   " has been successfully loaded");
    result.merge(good_result);

    return result;
  }

  OperationResult LibLoader::loadTransportLib(const std::string& transport_def_name, const std::string& lib_path, const std::string& sym)
  {
    using GetTransportDefinition = const TransportDefinition* (*)();

    OperationResult result;
    void* lib_handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!lib_handle) {
      OperationResult open_result;
      open_result.success = false;
      open_result.messages.push_back("[LibLoader::loadTransportLib]: The lib " + lib_path + " cannot be found");
      result.merge(open_result);
      return result;
    }

    auto lib_sym = dlsym(lib_handle, sym.c_str());
    if (!lib_sym) {
      OperationResult sym_result;
      sym_result.success = false;
      sym_result.messages.push_back("[LibLoader::loadTransportLib]: The lib " + lib_path +
                                    " is found but does not contain the symbol: " + sym);
      result.merge(sym_result);
      dlclose(lib_handle);
      return result;
    }

    auto getTransportDefinition = reinterpret_cast<GetTransportDefinition>(lib_sym);
    auto transport_definition = getTransportDefinition();
    if (!transport_definition) {
      OperationResult def_result;
      def_result.success = false;
      def_result.messages.push_back("[LibLoader::loadTransportLib]: The lib " + lib_path + " and the symbol " + sym +
                                    " are found but there is no TransportDefinition");
      result.merge(def_result);
      dlclose(lib_handle);
      return result;
    }

    OperationResult good_result;
    d->transport_registry.addTransportDefinition(transport_def_name, *transport_definition);
    d->transport_lib_handles.emplace(transport_def_name, lib_handle);
    good_result.success = true;
    good_result.messages.push_back("[LibLoader::loadTransportLib]: The symbol " + sym + " at lib " + lib_path +
                                   " has been successfully loaded");
    result.merge(good_result);

    return result;
  }

} // namespace pendarlab::app::mavlink_hub