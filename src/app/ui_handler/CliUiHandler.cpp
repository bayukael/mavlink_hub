#include "app/ui_handler/CliUiHandler.h"

#include "app/ui_handler/CliUiController.h"
#include "common/types/OperationResult.h"

#include <algorithm>
#include <chrono>
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

    Component layout = Renderer(command_pane, [&] {
      // clang-format off
      return vbox({
                 text("Mavlink Hub UI") | bold | center | border,
                 hbox({ command_pane->Render() | border, command_result_pane->Render() | flex | border }) | flex,
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
