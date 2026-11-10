```
╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│                                                   Mavlink Hub UI                                                    │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
╭──────────────────────┬────────────────────────────╮╭────────────────────────────────────────────────────────────────╮
│    Command Lists     │           Payload          ││                        Command Result                          │
├──────────────────────┼────────────────────────────┤├────────────────────────────────────────────────────────────────┤
│ > Command 1          │                            ││                                                                │
│   Command 2          │                            ││                                                                │
│   Command 3          │                            ││                                                                │
│   Command 4          │                            ││                                                                │
│                      │                            ││                                                                │
│                      │                            ││                                                                │
│                      │                            ││                                                                │
├──────────────────────┴──────────┬─────────────────┤│                                                                │
│ Selected Command: Command 1     │     Action      ││                                                                │
│          Payload: not required  ├─────────────────┤│                                                                │
├─────────────────────────────────┤[Execute Command]││                                                                │
│ Description:                    │[ Clear Payload ]││                                                                │
│ Here is a description of the    │[ Clear Result  ]││                                                                │
│ selected command.               ├─────────────────┤│                                                                │
│                                 │      Ready      ││                                                                │
╰─────────────────────────────────┴─────────────────╯╰────────────────────────────────────────────────────────────────╯
╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│                                     Report Refresh Period: - 1000 ms +                                              │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
╭──────────────────────────────────────────────────────────────────────────╮╭─────────────────────────────────────────╮
│                                Agent List                                ││              Endpoint List              │ 
├──────────────────┬─────────────────────┬─────────────┬───────────────────┤├──────────────┬────────┬─────────────────┤
│       Name       │        Type         │   Status    │      Report       ││     Name     │  Type  │     Status      │
├──────────────────┼─────────────────────┼─────────────┼───────────────────┤├──────────────┼────────┼─────────────────┤
│ Drone 1          │ gz_agent            │ STARTING    │ 50/50/50/50/30/10 ││ Pixhawk6X-1  │ udp    │ DISCONNECTED    │
│ Drone 2          │ gz_agent            │ ACTIVE      │ 50/50/50/50/30/10 ││ Pixhawk6X-2  │ serial │ CONNECTED       │
│ heartbeat        │ heartbeat_generator │ ACTIVE      │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
│                  │                     │             │                   ││              │        │                 │
╰──────────────────┴─────────────────────┴─────────────┴───────────────────╯╰──────────────┴────────┴─────────────────╯
╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│ Application Status: Running                                                                                         │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
```
---

# Parts & Components

This section names every part of the layout above so it can be implemented in
`CliUiHandler` (MHP-007) and referenced unambiguously. Names in parentheses are
the symbols already used in `CliUiHandler.cpp` where they exist; new names are
proposed for the agent/endpoint tables and their rows/columns.

## Annotated map

Each region is labeled with its part name. Locations refer to the mockup rows
(1-based as rendered above).
```
╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│ 1. Title Bar                                                                                                        │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
╭──────────────────────┬──────────────────────────────────╮╭──────────────────────────────────────────────────────────╮
│ 2. Command Lists Pane│  3. Payload Pane                 ││  4. Command Result Pane                                  │
│    └─ menu items     │     └─ input                     ││                                                          │
│ 2a. Selector         │                                  ││                                                          │
│                      │                                  ││                                                          │
├─────────────────────┬┴──────────────────────────────────┤│                                                          │
│ 5. Command Info     │ 6. Action Pane                    ││                                                          │
│                     │    ├─ 6a. Execute Command Button  ││                                                          │
│                     │    ├─ 6b. Clear Payload Button    ││                                                          │
│                     │    ├─ 6c. Clear Result Button     ││                                                          │
│                     │    └─ 6d. Action Status           ││                                                          │
│                     │                                   ││                                                          │
╰─────────────────────┴───────────────────────────────────╯╰──────────────────────────────────────────────────────────╯
╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│ 10. Report Refresh Period (third row)                                                                               │
│     (decrease '-' button / current value / increase '+' button)                                                     │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
╭──────────────────────────────────────────────────────────────────────────╮╭─────────────────────────────────────────╮
│                           7a. Agent List Header                          ││        8a. Endpoint List Header         │ 
├──────────────────┬─────────────────────┬─────────────┬───────────────────┤├──────────────┬────────┬─────────────────┤
│    7b. Name      │     7c. Type        │ 7d. Status  │   7e. Report      ││  8b. Name    │8c.Type │  8d. Status     │
│                  │                     │             │                   ││              │        │                 │
╰──────────────────┴─────────────────────┴─────────────┴───────────────────╯╰──────────────┴────────┴─────────────────╯
╭─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│ 9. Application Status Bar                                                                                           │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
```

## Part list

### 1. Title Bar (`title_bar`)
The full-width bordered banner at the very top showing the application name,
**"Mavlink Hub UI"**. Static text, no data.

### Command Pane
The left portion of the second row (the command controls). It groups five
parts: Command Lists, Payload, Command Info, Action panes, and the Selector.
The group itself is not labeled in the mockup; it is the existing
`command_pane`.

#### 2. Command Lists Pane (`command_menu`)
The panel titled **"Command Lists"**. Its header is centered/bold; beneath it
is a menu of the available commands (e.g. `Command 1`..`Command 4`).

##### 2a. Selector
The marker `>` in front of the currently selected/highlighted menu item
(`> Command 1`). Indicates the active menu entry.

#### 3. Payload Pane (`payload`)
The panel titled **"Payload"**. Contains a text input where the user types the
command payload (JSON).

#### 5. Command Info (`command_info`)
The two-line block under the Command Lists / Payload panes:
- **Selected Command:** showing the currently committed command name.
- **Payload:** showing whether the committed command requires a payload
  (`required` / `not required`).
- When the committed command carries a description, an optional third section
  below a separator shows the command's human-readable
  **Description:** (label centered) and the description text.

#### 6. Action Pane (`action_pane`)
The panel titled **"Action"** containing the command action buttons and the
action status line.

##### 6a. Execute Command button (`execute_command_button`)
`[Execute Command]` — runs the committed command.

##### 6b. Clear Payload button (`clear_payload_button`)
`[Clear Payload]` — clears the payload input.

##### 6c. Clear Result button (`clear_result_button`)
`[Clear Result]` — clears the command result pane.

##### 6d. Action Status (`action_status`)
The status line at the bottom of the Action Pane showing `Ready` (idle) or
`Executing...` (a command is running).

### 4. Command Result Pane (`command_result_pane`)
The right portion of the second row, titled **"Command Result"**. Displays the
outcome of the last executed command (success/failure and messages). This pane
is existing behavior and out of MHP-006's core scope, named here for
completeness.

### 10. Report Refresh Period Control (third row, `report_refresh_period_row`)
The full-width bordered bar on the **third row** that controls the live periodic
agent-report refresh. It renders exactly **`Report Refresh Period: - <value> ms +`**
(`report_refresh_period_row`), where the `-` and `+` are bare buttons
(`decrease_period_button` / `increase_period_button`) and `<value>` is the current
refresh period in milliseconds. The `-` button decreases and the `+` button
increases the period by 100 ms per press, bounded to the range
[100, 5000] ms (default 1000 ms); the control is a bar, so the row below it is the
overview. While the period is at its lower bound the `-` button renders dimmed and
is a no-op; at the upper bound the `+` button does the same, so no further
single-step change is possible in that direction at the bound. It is not labeled
in the mockup; proposed symbol `report_refresh_period_row`.

### Overview Pane
The **fourth row** containing the two tables. Not labeled in the mockup; proposed
group name for Agent List + Endpoint List. It is the part that MHP-006/007
primarily specify.

#### 7. Agent List Table (`agent_list`)
The left table, titled **"Agent List"**. One row per agent. MHP-007 must render
these rows from the agent list data.

##### 7a. Agent List Header
The row of column captions: **Name | Type | Status | Report**.

##### 7b. Agent Name Column
The agent's name (e.g. `Drone 1`, `heartbeat`).

##### 7c. Agent Type Column
The agent's type — the library name that provides the agent
(e.g. `gz_agent`, `heartbeat_generator`).

##### 7d. Agent Status Column
The agent's current state (e.g. `STARTING`, `ACTIVE`), one of the `AgentState`
enum values.

##### 7e. Agent Report Column
The column for the agent's report. It contains a **header** (the `Report`
caption in the table header) and a **report cell** for each agent list entry —
the opaque, free-form text string rendered as-is for that agent (e.g.
`50/50/50/50/30/10`). The CLI does **not** parse or reformat this text.

> Scope note: per MHP-006, the report's acquisition mechanism and full display
> wiring are deferred. This layout only specifies its *presentation* as an opaque
> text cell. MHP-007 structures the column; the report is populated and live-refreshed
> by the periodic agent-report refresh (MHP-010) through the controller's
> `updateOverviewData()`.

#### 8. Endpoint List Table (`endpoint_list`)
The right table, titled **"Endpoint List"**. One row per endpoint. Endpoints
have **no** report.

##### 8a. Endpoint List Header
The row of column captions: **Name | Type | Status**.

##### 8b. Endpoint Name Column
The endpoint's name (e.g. `Pixhawk6X-1`).

##### 8c. Endpoint Type Column
The endpoint's transport type (e.g. `udp`, `serial`).

##### 8d. Endpoint Status Column
The endpoint's connection state (e.g. `CONNECTED`, `DISCONNECTED`), one of the
`MavlinkEndpointState` values.

### 9. Application Status Bar (`app_status`)
The full-width bordered bar at the very bottom showing the application status
text, e.g. **"Application Status: Running"** (or an exit/waiting message while
stopping).