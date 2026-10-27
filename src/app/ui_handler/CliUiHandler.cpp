#include "app/ui_handler/CliUiHandler.h"

#include "app/ui_handler/CliUiController.h"
#include "common/types/OperationResult.h"

#include <algorithm>
#include <ftxui.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  struct CliUiHandler::CliUiHandlerImpl {
    CliUiHandlerImpl(IAppService& appsrv);

    CliUiController controller;

    std::string app_status = "Running";

    const std::string command_info_selected_label = " Selected Command: ";
    const std::string command_info_payload_hint_label = "          Payload: ";
    int max_command_info_len = 0;
    int max_command_info_label_len = 0;
    int max_command_name_len = 0;

    const std::string action_execute_command_label = "Execute Command";
    const std::string action_clear_payload_label = "Clear Payload";
    const std::string action_clear_result_label = "Clear Result";
    int max_action_label_len = 0;

    bool running = false;
    std::thread ui_thread;

    // Resizable overview-table column widths (pixels), adjustable by dragging the
    // column separators. Persisted across renders for the UI session.
    int agent_name_w = 12;
    int agent_name_type_w = 24;
    int agent_report_w = 10;
    int ep_type_status_w = 12;
    int ep_status_w = 8;

    // Width (pixels) of the Agent List pane in the overview row; the Endpoint
    // List pane fills the rest. Dragging the boundary between them adjusts it.
    int overview_agent_width = 60;

    void generateUserInterface();
  };

  CliUiHandler::CliUiHandlerImpl::CliUiHandlerImpl(IAppService& appsrv) : controller(appsrv)
  {
    const std::vector<CommandDescriptor>& descriptors = controller.commandDescriptors();
    for (const CommandDescriptor& descriptor : descriptors) {
      max_command_name_len = std::max(max_command_name_len, static_cast<int>(descriptor.name.size()));
    }

    max_command_info_label_len = std::max(command_info_selected_label.size(), command_info_payload_hint_label.size());
    max_command_info_len = max_command_name_len + max_command_info_label_len;

    max_action_label_len = std::max(max_action_label_len, static_cast<int>(action_execute_command_label.size()));
    max_action_label_len = std::max(max_action_label_len, static_cast<int>(action_clear_payload_label.size()));
    max_action_label_len = std::max(max_action_label_len, static_cast<int>(action_clear_result_label.size()));
  }

  void CliUiHandler::CliUiHandlerImpl::generateUserInterface()
  {
    using namespace ftxui;

    std::vector<std::string>& command_entries = controller.commandEntries();
    int& selected_command = controller.selectedCommand();

    MenuOption command_options;
    command_options.on_enter = [this] { controller.commitCommand(controller.selectedCommand()); };
    Component command_menu = Menu(&command_entries, &selected_command, command_options);
    // clang-format off
    Component command_menu_with_scroll = Renderer(command_menu, [&] { 
      return hbox(
        command_menu->Render(),
        filler()
      ) | vscroll_indicator | frame;
    });
    Component command_menu_complete = Renderer(command_menu_with_scroll, [&] {
      return vbox(
        text("Command Lists") | center | bold,
        separator(),
        command_menu_with_scroll->Render()
      );
    });
    // clang-format on

    std::string& payload = controller.payload();
    // clang-format off
    Component payload_input = Input(&payload, "command payload (JSON)");
    Component payload_complete = Renderer(payload_input, [&] {
      return vbox(
        text("Payload") | center | bold,
        separator(),
        payload_input->Render()
      );
    });
    // clang-format on
    // clang-format off
    Component command_info = Renderer([this] {
      std::string selected_name = "none";
      std::string payload_hint = "-";
      const int committed_command = controller.committedCommand();
      const std::vector<CommandDescriptor>& descriptors = controller.commandDescriptors();
      if (committed_command >= 0 && committed_command < static_cast<int>(descriptors.size())) {
        const CommandDescriptor& descriptor = descriptors[static_cast<std::size_t>(committed_command)];
        selected_name = std::string(descriptor.name);
        payload_hint = descriptor.requires_payload ? "required" : "not required";
      }

      return vbox(
        text(command_info_selected_label + selected_name) | bold,
        text(command_info_payload_hint_label + payload_hint) | dim
      );
    });
    // clang-format on

    ButtonOption action_button_options;
    action_button_options.transform = [&](EntryState state) {
      Element label = hbox(text("["), filler(), text(state.label), filler(), text("]"));
      Element e = state.focused ? inverted(label) : label;

      return e;
    };
    Component execute_command_button = Button(
        "Execute Command", [this] { controller.executeCurrentCommand(); }, action_button_options);
    Component clear_payload_button = Button(
        "Clear Payload", [this] { controller.clearPayload(); }, action_button_options);
    Component clear_result_button = Button(
        "Clear Result", [this] { controller.clearResult(); }, action_button_options);

    Component command_pane_container = Container::Horizontal({ command_menu_complete, payload_complete });
    Component action_menu = Container::Vertical({ execute_command_button, clear_payload_button, clear_result_button });
    Component command_action_container = Container::Vertical({ command_pane_container, action_menu });

    Component action_status = Renderer([this] {
      if (controller.executing()) {
        return bgcolor(Color::Orange1, color(Color::Black, text("Executing...") | center));
      }
      return bgcolor(Color::Green, color(Color::White, text("Ready") | center));
    });

    Component action_pane = Renderer([&] {
      return vbox({ text("Action") | bold | center, separator(), action_menu->Render(), separator(), action_status->Render() });
    });
    // clang-format off
    Component command_pane = Renderer(command_action_container, [&] {
      return vbox({
        hbox({ command_menu_complete->Render(), separator(), payload_complete->Render() | flex }) | flex,
        separator(),
        hbox({ command_info->Render() | size(WIDTH, GREATER_THAN, max_command_info_len), separator(), action_pane->Render()}) | notflex
      });
    });
    // clang-format on

    Component command_result_pane = Renderer([this] {
      std::optional<CommandResult> command_result = controller.commandResult();
      Elements result_content;
      if (command_result.has_value()) {
        result_content.push_back(text(controller.executedCommand()) | bold);
        result_content.push_back(text(command_result->success ? "SUCCESS" : "FAILED") | bold);
        for (const std::string& message : command_result->message) {
          result_content.push_back(text(message));
        }
        if (!command_result->data.empty()) {
          result_content.push_back(separator());
          result_content.push_back(text(command_result->data) | dim);
        }
      } else {
        result_content.push_back(text("(empty)") | dim);
      }
      // clang-format off
      return vbox({
        text("Command Result") | bold | center,
        separator(),
        vbox(std::move(result_content)) | frame | vscroll_indicator
      });
      // clang-format on
    });

    // Overview row: Agent List and Endpoint List tables. The renderer consumes the
    // row data exposed by the controller (agentRows()/endpointRows()) so the view
    // never needs to change when the data source is wired in. Each table is a chain
    // of ftxui ResizableSplit components: every column is a vertical stack of its
    // cells, and the user can drag a column separator to resize that column.
    // Column widths are kept in the impl members so the user's chosen sizes persist
    // for the whole UI session.
    // clang-format off

    // Render one column: a bold, centered header with a horizontal separator under
    // it, then one body cell per data value. Body cells are left-aligned by default;
    // pass dim_bodies to render them dimmed (used for the "(empty)" marker row).
    auto make_col = [](const std::string& header, std::function<std::vector<std::string>()> body,
                       bool dim_bodies = false) -> Component {
      return Renderer([header, body = std::move(body), dim_bodies] {
        Elements el;
        el.push_back(text(header) | bold | center);
        el.push_back(separator());
        for (const std::string& v : body()) {
          Element t = text(v);
          if (dim_bodies) {
            t = std::move(t) | dim;
          }
          el.push_back(std::move(t));
        }
        return vbox(std::move(el));
      });
    };

    // Body cells for one column of a table. When the list is empty the first column
    // (is_marker_col) yields a single dim "(empty)" row and the others yield a blank,
    // so the table reads as intentionally empty.
    auto col_body = [&](const std::vector<OverviewRow>& rows, const std::string OverviewRow::* field,
                        bool is_marker_col) {
      std::vector<std::string> body;
      if (rows.empty()) {
        body.push_back(is_marker_col ? std::string("(empty)") : std::string());
      } else {
        body.reserve(rows.size());
        for (const auto& row : rows) {
          body.push_back(row.*field);
        }
      }
      return body;
    };

    const std::vector<OverviewRow>& agent_rows = controller.agentRows();
    const bool agent_empty = agent_rows.empty();
    // Agent List table columns: Name | Type | Status | Report.
    Component agent_name_col   = make_col("Name",   [&] { return col_body(agent_rows, &OverviewRow::name,   true); }, agent_empty);
    Component agent_type_col   = make_col("Type",   [&] { return col_body(agent_rows, &OverviewRow::type,   false); }, agent_empty);
    Component agent_status_col = make_col("Status", [&] { return col_body(agent_rows, &OverviewRow::status, false); }, agent_empty);
    Component agent_report_col = make_col("Report", [&] { return col_body(agent_rows, &OverviewRow::report, false); }, agent_empty);
    Component agent_split = ResizableSplitLeft(
        ResizableSplitLeft(
            agent_name_col,
            agent_type_col,
            &agent_name_w),
        ResizableSplitRight(
            agent_report_col,
            agent_status_col,
            &agent_report_w),
        &agent_name_type_w);

    const std::vector<OverviewRow>& endpoint_rows = controller.endpointRows();
    const bool endpoint_empty = endpoint_rows.empty();
    // Endpoint List table columns: Name | Type | Status (Status fills the rest).
    Component ep_name_col   = make_col("Name",   [&] { return col_body(endpoint_rows, &OverviewRow::name,   true); }, endpoint_empty);
    Component ep_type_col   = make_col("Type",   [&] { return col_body(endpoint_rows, &OverviewRow::type,   false); }, endpoint_empty);
    Component ep_status_col = make_col("Status", [&] { return col_body(endpoint_rows, &OverviewRow::status, false); }, endpoint_empty);
    Component endpoint_split = ResizableSplitRight(
        ResizableSplitRight(ep_status_col, ep_type_col, &ep_status_w),
        ep_name_col,
        &ep_type_status_w);
    // clang-format on

    // Build each table pane as a component that routes events to its inner column
    // splits, wrapped in a titled, bordered panel. No flex here: the outer
    // overview split sizes the agent pane (fixed) and the endpoint pane (fills).
    Component agent_pane = Renderer(agent_split, [&] {
      return vbox(text("Agent List") | bold | center, separator(), agent_split->Render()) | border;
    });
    Component endpoint_pane = Renderer(endpoint_split, [&] {
      return vbox(text("Endpoint List") | bold | center, separator(), endpoint_split->Render()) | border;
    });

    // Overview split keeps the agent/endpoint boundary draggable; the agent pane
    // has a fixed width and the endpoint pane fills the rest.
    Component overview_split = ResizableSplitLeft(agent_pane, endpoint_pane, &overview_agent_width);

    // The overview splits are interactive children of the layout so their
    // separators receive mouse (drag) events, while rendering stays in the custom
    // vbox layout below.
    Component layout = Renderer(Container::Vertical({ command_pane, overview_split }), [&] {
      // clang-format off
      return vbox({
                 text("Mavlink Hub UI") | bold | center | border,
                 hbox({ command_pane->Render() | border, command_result_pane->Render() | flex | border }) | flex,
                 overview_split->Render(),
                 text("Application Status: " + app_status) | border | notflex,
             }) | border;
      // clang-format on
    });

    std::vector<Component> tab_boxes{ command_menu, payload_input, action_menu };

    auto focus_next_box = [&](int direction) {
      int focused_index = -1;
      for (std::size_t i = 0; i < tab_boxes.size(); ++i) {
        if (tab_boxes[i]->Focused()) {
          focused_index = static_cast<int>(i);
          break;
        }
      }
      if (focused_index < 0) {
        focused_index = 0;
      }
      const std::size_t next =
          (static_cast<std::size_t>(focused_index) + static_cast<std::size_t>(direction) + tab_boxes.size()) % tab_boxes.size();
      tab_boxes[next]->TakeFocus();
    };

    auto layout_with_nav = CatchEvent(layout, [&](Event event) {
      if (event == Event::Tab) {
        focus_next_box(+1);
        return true;
      }
      if (event == Event::TabReverse) {
        focus_next_box(-1);
        return true;
      }
      return false;
    });

    auto app = App::Fullscreen();
    app.Loop(layout_with_nav);
  }

  CliUiHandler::CliUiHandler(IAppService& appsrv) : d(std::make_unique<CliUiHandlerImpl>(appsrv))
  {
  }
  CliUiHandler::~CliUiHandler()
  {
    stop();
  }
  CliUiHandler::CliUiHandler(CliUiHandler&&) noexcept = default;
  CliUiHandler& CliUiHandler::operator=(CliUiHandler&&) noexcept = default;

  void CliUiHandler::start()
  {
    if (d->running) {
      return;
    }
    d->running = true;

    // When a command finishes on the worker thread, ask the UI loop to
    // redraw so the result/status pane reflects the new state without
    // requiring any user input.
    d->controller.setOnUpdate([] {
      if (ftxui::App::Active() != nullptr) {
        ftxui::App::Active()->Post([] {
          if (ftxui::App::Active() != nullptr) {
            ftxui::App::Active()->RequestAnimationFrame();
          }
        });
      }
    });

    d->ui_thread = std::thread(&CliUiHandler::CliUiHandlerImpl::generateUserInterface, d.get());
  }

  void CliUiHandler::stop()
  {
    if (!d->running) {
      return;
    }

    if (d->controller.executing()) {
      std::string waiting_for = "a command";
      const int committed_command = d->controller.committedCommand();
      const std::vector<CommandDescriptor>& descriptors = d->controller.commandDescriptors();
      if (committed_command >= 0 && committed_command < static_cast<int>(descriptors.size())) {
        waiting_for = std::string(descriptors[static_cast<std::size_t>(committed_command)].name);
      }
      d->app_status = "Exiting - waiting for: " + waiting_for;
      if (ftxui::App::Active() != nullptr) {
        ftxui::App::Active()->RequestAnimationFrame();
      }
      d->controller.joinExecution();
    }

    if (ftxui::App::Active() != nullptr) {
      ftxui::App::Active()->Exit();
    }
    if (d->ui_thread.joinable()) {
      d->ui_thread.join();
    }
    d->running = false;
  }

  OperationResult CliUiHandler::getResult()
  {
    OperationResult result;
    return result;
  }

} // namespace pendarlab::app::mavlink_hub
