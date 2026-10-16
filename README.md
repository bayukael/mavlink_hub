# mavlink_hub

`mavlink_hub` (Pendarlab) is an application that acts as a hub for communicating with
multiple MAVLink endpoints through a set of runtime loaded *agents*. It lets an operator connect
to MAVLink-speaking systems (autopilots, ground control stations) directly, and to
non-MAVLink-speaking systems (simulators, onboard compute, middleware) through pluggable agents
that translate between the MAVLink world and the foreign system's protocol.

This repository contains the hub application itself. It builds on three cooperating external
packages:

- **mavlink_hub_sdk** — the `Agent`, `AgentDefinition`, `AgentState`, endpoint-user, and resource
  requester interfaces the hub programs against, and the surface used to implement custom agents.
- **mavlink_endpoint** — the MAVLink endpoint abstraction sitting on top of a pluggable
  `byte_transport` (serial, UDP, TCP, ...).
- **byte_transport** — pluggable transports behind a common registry/definition interface.

## What it does

The hub is MAVLink-centric: every endpoint it owns is a MAVLink endpoint. Live endpoints are
created and owned by a **Manager** at runtime, as are live agents. Agents ask the Manager for
resources (such as a mavlink endpoint) through an injected `ManagerResourceRequester` and receive
a safe `IMavlinkEndpointUser` handle they can listen on and write to.

An operator drives the hub through a CLI UI. Commands are dispatched through the
**Application** layer to the **Manager**, which owns endpoints and agents. The UI, application,
and manager are the three in-repo components; endpoints and agents themselves are provided by the
external packages above and their instances are created and managed by the hub at runtime.

For a full C4-style walkthrough of the system context, components, and class-level internals, see
[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Quick start (build)

Requires:

- CMake >= 3.13 and GoogleTest (`find_package(GTest CONFIG REQUIRED)`).
- The SDK/endpoint packages installed and discoverable via `CMAKE_PREFIX_PATH`
  (external `pendarlab` SDK packages) plus `MAVLINK_HEADER_DIR` for the raw MAVLink headers.

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=<path-to-pendarlab-packages> \
  -DMAVLINK_HEADER_DIR=<path-to-mavlink-headers>
cmake --build build
```

Run the tests (GTest executables are run directly from the build dir; there is no `ctest`):

```bash
./build/MavlinkHubManagerTest
./build/MavlinkHubLibLoaderTest
./build/MavlinkHubJsonUtilsTest
# ... and the other MavlinkHub*Test executables
```

Run a single test:

```bash
./build/MavlinkHubManagerTest --gtest_filter='*SomeTestCase*'
```

There are no lint/format targets; building and running the tests is the only verification.

## Usage

The `mavlink_hub` main executable target is currently **commented out** in `CMakeLists.txt`; only
the libraries and tests build. When enabled, the executable is configured from a JSON config file
and driven through the CLI UI:

- **Config file** (`-c/--config <path>`) — JSON describing the agent and transport libraries to
  load at startup (keyed by registration name), an optional extra-libraries file, and an optional
  startup user plan (with a flag to apply it on startup). See `AppConfig` in
  `src/app/types/AppConfig.h`.

- **Commands** — the UI presents a command menu backed by the descriptor table in
  `src/app/types/CommandDescriptor.cpp`. Commands cover plan management
  (`load_plan_from_path`, `load_plan_from_json_text`, `get_current_plan`, `check_current_plan`,
  `apply_current_plan`, `delete_current_plan`, `get_status`), endpoint management
  (`add_mavlink_endpoint`, `edit_mavlink_endpoint`, `connect_mavlink_endpoint`,
  `disconnect_mavlink_endpoint`, `remove_mavlink_endpoint`, `get_endpoint_list`,
  `get_endpoint_status`, `get_endpoint_status_all`), and agent management (`add_agent`,
  `edit_agent`, `start_agent`, `stop_agent`, `remove_agent`, `get_agent_list`,
  `get_agent_status`, `get_agent_status_all`). Commands that take a payload accept JSON
  (validated against the schemas in `resources/json_schemas/` where applicable).

  Note: many `AppService::executeCommand` cases are currently stubs, so not every listed command is
  fully implemented yet (see `docs/ARCHITECTURE.md` §7).

## Repository status

The project is mid-development. Notable gaps (detailed in `docs/ARCHITECTURE.md` §7):

- `AppService::executeCommand` has many stub cases.
- `include/mavlink_hub/` is reserved for future public/installed headers and is currently empty.
- The `mavlink_hub` main executable and `HttpRequestHandler` are planned but not yet active.
- `CliUiHandler::getResult()` is a stub.
