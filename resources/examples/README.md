# mavlink_hub examples

Ready-to-use example configuration files for `mavlink_hub`: a [UserPlan](#userplan-files)
(declarative description of the MAVLink endpoints and agents to bring up) and the
[app config](#app-config) that wires the required plugin libraries and (optionally)
applies the plan at startup. Copy them and adapt the names, types, and config
values to your own setup.

The JSON schema of these files is defined by the parsers in
`src/common/json_utils/JsonUtils.cpp` (`jsonToUserPlan`, `jsonToAppConfig`).

## UserPlan files

- `user_plan_best_effort.json` — the same system declared with the
  `best_effort` policy.
- `user_plan_discard.json` — the same system declared with the `discard` policy.

Both files declare one serial MAVLink endpoint and one Gazebo bridge agent.

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
      "config": { "key": "string value", ... }   // string->string map, validated by the agent's config parser
    }
  ]
}
```

Notes:

- All `config` values must be JSON strings, even numbers (`"baud_rate": "115200"`).
- Endpoint and agent names are unique keys: the parsed plan maps entries by name,
  and agents reference endpoints by their `name` (e.g. `mavlink_endpoint_name`).
- `type` values must match the names the corresponding plugin libraries are
  registered under in the app config (`transport_lib_list` / `agent_lib_list`).

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
   (`SerialByteTransport` from `serial_byte_transport`, `gz_agent` from
   `mavlink_hub_gz_agent`), and
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

## Example plugin config values

The values used in the example plan come from real plugins:

- `SerialByteTransport` (from the `serial_byte_transport` repository): requires
  `device_path` and `baud_rate`; optional serial knobs (parity, stop bits,
  flow control, ...) use defaults when omitted.
- `gz_agent` (from the `mavlink_hub_gz_agent` repository): requires
  `vehicle_name`, `world_name`, and `mavlink_endpoint_name`; supports
  `use_topic_generator`, `send_freq`, topic overrides, and magnetometer
  unit/frame options — see that repository's README for the full list.
