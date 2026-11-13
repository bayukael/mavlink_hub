# mavlink_hub

`mavlink_hub` is an application that acts as a hub for communicating with multiple MAVLink endpoints through a set of runtime loaded *agents*. It lets an operator connect to MAVLink-speaking systems (autopilots, ground control stations) directly, and to non-MAVLink-speaking systems (simulators, onboard compute, middleware) through pluggable agents that translate between the MAVLink world and the foreign system's protocol.

This repository contains the hub application itself. It builds on three cooperating external packages:

- **mavlink_hub_sdk** — the `Agent`, `AgentDefinition`, `AgentState`, endpoint-user, and resource requester interfaces the hub programs against, and the surface used to implement custom agents.
- **mavlink_endpoint** — the MAVLink endpoint abstraction sitting on top of a pluggable `byte_transport` (serial, UDP, TCP, ...).
- **byte_transport** — pluggable transports behind a common registry/definition interface.

## What it does

The hub is MAVLink-centric: every endpoint it owns is a MAVLink endpoint. Live endpoints are created and owned by a **Manager** at runtime, as are live agents. Agents ask the Manager for resources (such as a mavlink endpoint) through an injected `ManagerResourceRequester` and receive a safe `IMavlinkEndpointUser` handle they can listen on and write to.

An operator drives the hub through a CLI UI. Commands are dispatched through the **Application** layer to the **Manager**, which owns endpoints and agents. The UI, application, and manager are the three in-repo components; endpoints and agents themselves are provided by the external packages above and their instances are created and managed by the hub at runtime.

For details in `mavlink_hub` architecture, see [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Quick start (build)

Requires:

- CMake >= 3.13 and GoogleTest (`find_package(GTest CONFIG REQUIRED)`).
- The SDK/endpoint packages installed and discoverable via `CMAKE_PREFIX_PATH` (external `pendarlab` SDK packages) plus `MAVLINK_HEADER_DIR` for the raw MAVLink headers.

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

The `mavlink_hub` main executable is built from the `mavlink_hub` target in `CMakeLists.txt` (entry point: `src/main.cpp`). It is configured from a JSON config file and driven through the CLI UI:

- **Config file** (`-c/--config <path>`) — JSON describing the agent and transport libraries to load at startup (keyed by registration name), an optional extra-libraries file, and an optional startup user plan (with a flag to apply it on startup). See `AppConfig` in `src/app/types/AppConfig.h`.

- **Commands** — the UI presents a command menu backed by the descriptor table in `src/app/app_service/AppServiceCommandDescriptor.cpp`. Commands cover:
  - plan management (`load_plan_from_path`, `load_plan_from_json_text`, `get_current_plan`, `check_current_plan`, `apply_current_plan`, `delete_current_plan`, `get_status`)
  - endpoint management (`add_mavlink_endpoint`, `edit_mavlink_endpoint`, `connect_mavlink_endpoint`, `disconnect_mavlink_endpoint`, `remove_mavlink_endpoint`, `get_endpoint_list`, `get_endpoint_status`, `get_endpoint_status_all`)
  - agent management (`add_agent`, `edit_agent`, `start_agent`, `stop_agent`, `remove_agent`, `get_agent_list`, `get_agent_status`, `get_agent_status_all`).

  Commands that take a payload accept JSON (validated against the schemas in `resources/json_schemas/` where applicable).

  Note: many `AppService::executeCommand` cases are currently stubs, so not every listed command is fully implemented yet (see `docs/ARCHITECTURE.md` §7).

### Running with the example config

`mavlink_hub` ships self-contained examples in `resources/examples/` that use
only the in-repo mock plugins built for the tests (`MavlinkHubMockAgent` and
`MavlinkHubMockByteTransport`), so no external plugin libraries are needed:

- `resources/examples/app_config_example.json` — startup config that loads the
  two mock plugins and applies a user plan on startup.
- `resources/examples/user_plan_best_effort.json` and
  `resources/examples/user_plan_discard.json` — user plans declaring one
  endpoint (`mock_byte_transport`) and one agent (`mock_agent`).

To run the hub with them:

1. **Build** (see [Quick start (build)](#quick-start-build) above), producing
   the mock plugin libraries `build/libMavlinkHubMockAgent.so` and
   `build/libMavlinkHubMockByteTransport.so`.

2. **Resolve the mock library paths.** `app_config_example.json` references the
   mock libraries relative to the repo root (e.g. `build/libMavlinkHubMockAgent.so`).
   If you build into a different directory (or run from a different working
   directory), update the `path` values to point at your build tree — e.g. via
   CMake's `$<TARGET_FILE:MavlinkHubMockAgent>` generator expression. See
   [`resources/examples/README.md`](resources/examples/README.md) for details.

3. **Run the hub with the config** (from the repository root, so the relative
   paths in the example resolve):

   ```bash
   ./build/mavlink_hub -c resources/examples/app_config_example.json
   ```

   `app_config_example.json` has `apply_user_plan_on_startup: true`, so the
   hub loads `resources/examples/user_plan_best_effort.json` and applies it at
   startup (bringing up the `mock_byte_transport` endpoint and `mock_agent`
   agent). To just load the plan without applying it, set
   `apply_user_plan_on_startup` to `false`.

   You can also point `path_to_startup_user_plan` at either user plan file —
   both declare the same system, differing only in their `policy`
   (`best_effort` vs. `discard`).

Both mock plugins require a single config key, `a_required_string` (any string
value), which the example user plans already provide. See the
[examples README](resources/examples/README.md) for the full plugin/config
reference.

## Repository status

The project is mid-development. Notable gaps (detailed in `docs/ARCHITECTURE.md` §7):

- `AppService::executeCommand` has many stub cases.
- `include/mavlink_hub/` is reserved for future public/installed headers and is currently empty.
- The `HttpRequestHandler` is planned but not yet active.
- `CliUiHandler::getResult()` is a stub.
