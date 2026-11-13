# mavlink_hub examples

Ready-to-use example configuration files for `mavlink_hub`: a [UserPlan](#userplan-files)
(declarative description of the MAVLink endpoints and agents to bring up) and the
[app config](#app-config) that wires the required plugin libraries and (optionally)
applies the plan at startup. Copy them and adapt the names, types, and config
values to your own setup.

These examples are **self-contained**: they use only the in-repo mock plugins that
`mavlink_hub` builds for its own tests (`test/mock_agent/` and
`test/mock_byte_transport/`), so you can run the examples without installing any
external plugin libraries.

The JSON schema of these files is defined by the parsers in
`src/common/json_utils/JsonUtils.cpp` (`jsonToUserPlan`, `jsonToAppConfig`).

## UserPlan files

- `user_plan_best_effort.json` — the same system declared with the
  `best_effort` policy.
- `user_plan_discard.json` — the same system declared with the `discard` policy.

Both files declare one endpoint (`mock_byte_transport`) and one agent
(`mock_agent`).

### Structure

```json
{
  "policy": "discard" | "best_effort",   // optional; defaults handled by the parser
  "endpoint_list": [                      // optional; omitted = empty list
    {
      "name": "unique endpoint name",
      "type": "registered transport type name",
      "config": { "key": "string value", ... },  // string->string map, validated by the transport's config parser
      "connect_on_create": true | false
    }
  ],
  "agent_list": [                         // optional; omitted = empty list
    {
      "name": "unique agent name",
      "type": "registered agent type name",
      "config": { "key": "string value", ... },  // string->string map, validated by the agent's config parser
      "start_on_create": true | false            // optional; default false
    }
  ]
}
```

Notes:

- All `config` values must be JSON strings, even numbers.
- Endpoint and agent names are unique keys: the parsed plan maps entries by name,
  and agents reference endpoints by their `name`.
- `type` values must match the names the corresponding plugin libraries are
  registered under in the app config (`transport_lib_list` / `agent_lib_list`).
- `connect_on_create` on an endpoint entry connects the endpoint as part of plan
  execution; `start_on_create` on an agent entry starts the agent after it is
  created as part of plan execution. Both default to `false`, so omitting them
  keeps the entry created/added but not connected/started.

### Policies

The `policy` controls how validation failures inside `endpoint_list` /
`agent_list` entries are handled when the plan is executed
(`UserPlanPolicy`, see `src/manager/types/UserPlanPolicy.h`):

- `discard` — strict. Any invalid or missing entry aborts plan execution;
  nothing from the failing list is applied.
- `best_effort` — lenient. Invalid or missing entries are skipped and the
  remaining valid entries are still applied.

## App config

`app_config_example.json` shows the startup configuration that:

1. loads the plugin libraries that provide the `type`s used in the plan
   (`mock_byte_transport` from the in-repo `MavlinkHubMockByteTransport`,
   `mock_agent` from the in-repo `MavlinkHubMockAgent`), and
2. loads and applies the UserPlan at startup via `path_to_startup_user_plan`
   and `apply_user_plan_on_startup`.

```json
{
  "agent_lib_list":     [ { "name": "...", "path": ".../lib<agent>.so", "sym": "get<...>Definition" } ],
  "transport_lib_list": [ { "name": "...", "path": ".../lib<transport>.so", "sym": "get<...>Definition" } ],
  "path_to_startup_user_plan": "user_plan_best_effort.json",
  "apply_user_plan_on_startup": true
}
```

Notes:

- `path_to_startup_user_plan` is resolved to an absolute path by the parser, so
  it can be relative to the current working directory of the `mavlink_hub`
  process. Adjust it to wherever you keep your plan file.
- The `name` of each library entry is the type name referenced by the plan's
  `endpoint_list[].type` / `agent_list[].type`; `sym` is the exported
  `extern "C"` factory symbol of the plugin.
- Set `apply_user_plan_on_startup` to `false` to only load the plan (it can
  then be applied later via the `load_plan_from_path` / apply user commands).

## Resolving the mock library paths

The `path` fields in `app_config_example.json` point at the mock plugin
libraries **as built into your CMake build tree**, relative to the repository
root (the `-B build` directory from the build instructions in the main
`README.md`):

| `name`               | CMake target                | Built library file          | `sym`                       |
| -------------------- | --------------------------- | --------------------------- | --------------------------- |
| `mock_byte_transport` | `MavlinkHubMockByteTransport` | `build/libMavlinkHubMockByteTransport.so` | `getMockTransportDefinition` |
| `mock_agent`          | `MavlinkHubMockAgent`       | `build/libMavlinkHubMockAgent.so`        | `getMockAgentDefinition`     |

The example sets these paths relative to the repo root, so when you run
`mavlink_hub` from the repo root after building, the paths work as-is. If you
build into a different directory (or run the executable from elsewhere), update
the `path` values to point at your build tree — for example, you can use CMake's
`$<TARGET_FILE:MavlinkHubMockAgent>` generator expression to get the absolute
path to each target's built library.

> These mocks come from the `MavlinkHubMockAgent` and
> `MavlinkHubMockByteTransport` SHARED targets defined in `CMakeLists.txt`
> (`test/mock_agent/`, `test/mock_byte_transport/`). They are built only to
> exercise the hub, not as installed plugins.

## Example plugin config values

The values used in the example plans come from the in-repo mock plugins, which
are intentionally minimal. Both mocks require exactly one config key,
`a_required_string` (any string value) — without it their config parsers reject
the entry with a message like `a required entry is missing: a_required_string`.

- `mock_byte_transport` (from `test/mock_byte_transport/`, target
  `MavlinkHubMockByteTransport`, `extern "C"` symbol
  `getMockTransportDefinition`): requires `a_required_string`.
- `mock_agent` (from `test/mock_agent/`, target `MavlinkHubMockAgent`,
  `extern "C"` symbol `getMockAgentDefinition`): requires `a_required_string`.

To swap in real plugins, replace these types with your own agent/transport
type names and provide the config keys those plugins require.
