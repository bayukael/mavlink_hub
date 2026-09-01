cmake_minimum_required(VERSION 3.13)
project(pendarlab_app_mavlink_hub)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(NAMESPACE pendarlab)
set(PACKAGE_FILENAME mavlink_hub)
set(PACKAGE_NAME PendarlabMavlinkHub)
set(MAVLINK_HEADER_DIR "" CACHE PATH "Path to mavlink header directory")

find_package(GTest CONFIG REQUIRED)
find_package(PendarlabMavlinkHubSdk REQUIRED)
find_package(PendarlabMavlinkEndpoint REQUIRED)

##############################################################################
###########################     AgentRegistry     ############################
##############################################################################
add_library(MavlinkHubAgentRegistry SHARED
  src/agent_registry/AgentRegistry.cpp
  src/agent_registry/AgentRegistryUser.cpp
)
target_include_directories(MavlinkHubAgentRegistry PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubAgentRegistry
    PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubAgentRegistry PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_agent_registry)

##############################################################################
##############################     Manager     ###############################
##############################################################################
add_library(MavlinkHubManager SHARED
  src/manager/Manager.cpp
  src/manager/ManagerResourceRequester.cpp
  src/manager/mavlink_endpoint_user/MavlinkEndpointUser.cpp
)
target_include_directories(MavlinkHubManager PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubManager
    PUBLIC pendarlab::MavlinkHubSdk
    PUBLIC pendarlab::MavlinkEndpoint
    PUBLIC MavlinkHubAgentRegistry
)
set_target_properties(MavlinkHubManager PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_manager)

# ##############################################################################
# ########################     MavlinkEndpointUser     #########################
# ##############################################################################
# add_library(MavlinkHubEndpointUser SHARED
#   src/mavlink_endpoint_user/MavlinkEndpointUser.cpp
# )
# target_include_directories(MavlinkHubEndpointUser PUBLIC 
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
#     $<INSTALL_INTERFACE:include>
# )
# target_link_libraries(MavlinkHubEndpointUser
#     PUBLIC MavlinkHubManager
# )
# set_target_properties(MavlinkHubEndpointUser PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_endpoint_user)

##############################################################################
#########################     ShutdownController     #########################
##############################################################################
add_library(MavlinkHubShutdownController SHARED
  src/app/shutdown_controller/ShutdownController.cpp
)
target_include_directories(MavlinkHubShutdownController PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubShutdownController
    PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubShutdownController PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_shutdown_controller)

##############################################################################
#########################     StartupArgsParser    #########################
##############################################################################
add_library(MavlinkHubStartupArgsParser SHARED
  src/app/startup/StartupArgsParser.cpp
)
target_include_directories(MavlinkHubStartupArgsParser PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubStartupArgsParser
    PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubStartupArgsParser PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_startup_args_parser)

##############################################################################
############################     CliUiHandler     ############################
##############################################################################
add_library(MavlinkHubCliUiHandler SHARED
  src/app/ui_handler/CliUiHandler.cpp
)
target_include_directories(MavlinkHubCliUiHandler PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubCliUiHandler
    PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubCliUiHandler PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_cli_ui_handler)

##############################################################################
##############################     LibLoader     #############################
##############################################################################
add_library(MavlinkHubLibLoader SHARED
  src/app/lib_loader/LibLoader.cpp
)
target_include_directories(MavlinkHubLibLoader PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubLibLoader
    PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubLibLoader PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_lib_loader)

##############################################################################
##############################     JsonUtils     #############################
##############################################################################
add_library(MavlinkHubJsonUtils SHARED
  src/common/json_utils/JsonUtils.cpp
)
target_include_directories(MavlinkHubJsonUtils PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/jsoncons-1.9.0/include>
)
target_link_libraries(MavlinkHubJsonUtils
    PUBLIC pendarlab::MavlinkHubSdk
    PUBLIC pendarlab::MavlinkEndpoint
)
set_target_properties(MavlinkHubJsonUtils PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_json_utils)

##############################################################################
#############################     AppService     #############################
##############################################################################
add_library(MavlinkHubAppService SHARED
  src/app/app_service/AppService.cpp
)
target_include_directories(MavlinkHubAppService PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubAppService
  PUBLIC MavlinkHubShutdownController
  PUBLIC MavlinkHubStartupArgsParser
  PUBLIC MavlinkHubCliUiHandler
  PUBLIC MavlinkHubLibLoader
  PUBLIC MavlinkHubJsonUtils
  PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubAppService PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_app_service)

##############################################################################
################################     App     #################################
##############################################################################
add_library(MavlinkHubApp SHARED
  src/app/App.cpp
)
target_include_directories(MavlinkHubApp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(MavlinkHubApp
  PUBLIC MavlinkHubManager
  PUBLIC MavlinkHubShutdownController
  PUBLIC MavlinkHubStartupArgsParser
  PUBLIC MavlinkHubCliUiHandler
  PUBLIC MavlinkHubAppService
  PUBLIC pendarlab::MavlinkHubSdk
)
set_target_properties(MavlinkHubApp PROPERTIES OUTPUT_NAME pendarlab-mavlink_hub_app)

# ##############################################################################
# ################################     Main     ################################
# ##############################################################################
# add_executable(mavlink_hub 
#   src/main.cpp
# )
# target_include_directories(mavlink_hub PUBLIC 
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
# )
# target_compile_options(mavlink_hub
#   PRIVATE -Wno-address-of-packed-member
# )
# target_link_libraries(mavlink_hub
#   PRIVATE mavlink_hub_app
# )

##############################################################################
#############################     MockAgent    ###############################
##############################################################################
add_library(MavlinkHubMockAgent SHARED
  test/mock_agent/MockAgent.cpp
  test/mock_agent/MockAgentDefinition.cpp
)
target_include_directories(MavlinkHubMockAgent PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
)
target_link_libraries(MavlinkHubMockAgent
  PRIVATE pendarlab::MavlinkHubSdk
)

##############################################################################
#########################     MockByteTransport    ###########################
##############################################################################
add_library(MavlinkHubMockByteTransport SHARED
  test/mock_byte_transport/MockByteTransport.cpp
  test/mock_byte_transport/MockByteTransportDefinition.cpp
)
target_include_directories(MavlinkHubMockByteTransport PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
)
target_link_libraries(MavlinkHubMockByteTransport
  PRIVATE pendarlab::ByteTransport
)


##############################################################################
#########################     AgentRegistry Test    ##########################
##############################################################################
add_executable(MavlinkHubAgentRegistryTest
  test/agent_registry/agent_registry_test.cpp
)
target_include_directories(MavlinkHubAgentRegistryTest PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
)
target_compile_options(MavlinkHubAgentRegistryTest
  PRIVATE -Wno-address-of-packed-member
)
target_link_libraries(MavlinkHubAgentRegistryTest
  PRIVATE MavlinkHubAgentRegistry
  PRIVATE gtest
)

##############################################################################
############################     Manager Test    #############################
##############################################################################
add_executable(MavlinkHubManagerTest
  test/manager/manager_test.cpp
)
target_include_directories(MavlinkHubManagerTest PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${MAVLINK_HEADER_DIR}/include>
)
target_compile_options(MavlinkHubManagerTest
  PRIVATE -Wno-address-of-packed-member
)
target_link_libraries(MavlinkHubManagerTest
  PRIVATE pendarlab::ByteTransport
  PRIVATE MavlinkHubManager
  PRIVATE gtest
  PRIVATE pendarlab::MavlinkHubSdk
  PRIVATE MavlinkHubMockAgent
  PRIVATE MavlinkHubMockByteTransport
)

##############################################################################
##########################     Lib Loader Test     ###########################
##############################################################################
add_executable(MavlinkHubLibLoaderTest
  test/app/lib_loader/lib_loader_test.cpp
)
target_include_directories(MavlinkHubLibLoaderTest PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_compile_options(MavlinkHubLibLoaderTest
  PRIVATE -Wno-address-of-packed-member
)
target_link_libraries(MavlinkHubLibLoaderTest
  PRIVATE gtest
  PRIVATE pendarlab::ByteTransport
  PRIVATE MavlinkHubAgentRegistry
  PRIVATE MavlinkHubLibLoader
)
target_compile_definitions(MavlinkHubLibLoaderTest
  PRIVATE MOCK_BYTE_TRANSPORT_LIB_PATH="$<TARGET_FILE:MavlinkHubMockByteTransport>"
  PRIVATE MOCK_AGENT_LIB_PATH="$<TARGET_FILE:MavlinkHubMockAgent>"
)

##############################################################################
########################     Json Utilities Test     #########################
##############################################################################
add_executable(MavlinkHubJsonUtilsTest
  test/app/common/json_utils/json_utils_test.cpp
)
target_include_directories(MavlinkHubJsonUtilsTest PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/jsoncons-1.9.0/include>
)
target_link_libraries(MavlinkHubJsonUtilsTest
  PRIVATE gtest
  PRIVATE MavlinkHubJsonUtils
)
target_compile_definitions(MavlinkHubJsonUtilsTest
  PRIVATE TEST_PLAN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/app/common/json_utils/"
)

##############################################################################
##########################     App Service Test     ###########################
##############################################################################
add_executable(MavlinkHubAppServiceTest
  test/app/app_service/app_service_test.cpp
)
target_include_directories(MavlinkHubAppServiceTest PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/jsoncons-1.9.0/include>
)

