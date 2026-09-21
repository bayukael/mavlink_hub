#include "app/ui_handler/CliUiHandler.h"

#include "common/types/OperationResult.h"

#include <algorithm>
#include <chrono>
#include <ftxui.hpp>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

// TODO: start a thread to run the handler

namespace pendarlab::app::mavlink_hub
{
  struct CliUiHandler::CliUiHandlerImpl {
    CliUiHandlerImpl(IAppService& appsrv);
    IAppService& app_service;
    OperationResult run_result;

    std::vector<CommandDescriptor> command_descriptors;
    std::vector<std::string> command_entries;
    std::vector<std::string> action_entries{ "Execute command", "Clear payload", "Clear result" };
    int selected_command = 0;
    int committed_command = -1;
    int selected_action = 0;
    std::string payload;
    std::string app_status = "Running";

    const std::string command_info_selected_label = " Selected Command: ";
    const std::string command_info_payload_hint_label = "          Payload: ";
    int max_command_info_len = 0;

    const std::string action_execute_command_label = "Execute Command";
    const std::string action_clear_payload_label = "Clear Payload";
    const std::string action_clear_result_label = "Clear Result";
    int max_action_label_len = 0;

    std::optional<CommandResult> command_result;
    std::mutex result_mutex;
    bool executing = false;
    std::thread execute_thread;

    bool running = false;
    std::thread ui_thread;

    void generateUserInterface();
    void executeCurrentCommand();
    void executeCommandAsync(const UserCommand& cmd);
    void clearPayload();
    void clearResult();
  };

  CliUiHandler::CliUiHandlerImpl::CliUiHandlerImpl(IAppService& appsrv) : app_service(appsrv)
  {
    int max_command_info_label_len = 0;
    int max_command_name_len = 0;

    command_descriptors = app_service.getCommandDescriptors();
    for (const CommandDescriptor& descriptor : command_descriptors) {
      command_entries.push_back(std::string(descriptor.name));
      max_command_name_len = std::max(max_command_name_len, static_cast<int>(descriptor.name.size()));
    }

    max_command_info_label_len = std::max(command_info_selected_label.size(), command_info_payload_hint_label.size());
    max_command_info_len = max_command_name_len + max_command_info_label_len;

    max_action_label_len = std::max(max_action_label_len, static_cast<int>(action_execute_command_label.size()));
    max_action_label_len = std::max(max_action_label_len, static_cast<int>(action_clear_payload_label.size()));
    max_action_label_len = std::max(max_action_label_len, static_cast<int>(action_clear_result_label.size()));
  }

  void CliUiHandler::CliUiHandlerImpl::executeCurrentCommand()
  {
    if (executing) {
      return;
    }
    if (committed_command < 0 || committed_command >= static_cast<int>(command_descriptors.size())) {
      return;
    }

    UserCommand cmd;
    cmd.cmd_type = command_descriptors[static_cast<std::size_t>(committed_command)].type;
    cmd.payload = payload;

    executeCommandAsync(cmd);
  }

  void CliUiHandler::CliUiHandlerImpl::executeCommandAsync(const UserCommand& cmd)
  {
    if (execute_thread.joinable()) {
      execute_thread.join();
    }

    executing = true;

    execute_thread = std::thread([this, cmd]() {
      CommandResult result = app_service.executeCommand(cmd);

      if(cmd.cmd_type == UserCommandType::APPLY_CURRENT_PLAN){
        std::this_thread::sleep_for(std::chrono::seconds(5));
      }

      if (ftxui::App::Active() != nullptr) {
        ftxui::App::Active()->Post([this, result = std::move(result)]() mutable {
          std::lock_guard<std::mutex> lock(result_mutex);
          command_result = std::move(result);
          executing = false;
          if (ftxui::App::Active() != nullptr) {
            ftxui::App::Active()->RequestAnimationFrame();
          }
        });
      } else {
        std::lock_guard<std::mutex> lock(result_mutex);
        command_result = std::move(result);
        executing = false;
      }
    });
  }

  void CliUiHandler::CliUiHandlerImpl::clearPayload()
  {
    payload.clear();
  }

  void CliUiHandler::CliUiHandlerImpl::clearResult()
  {
    std::lock_guard<std::mutex> lock(result_mutex);
    command_result.reset();
  }

  void CliUiHandler::CliUiHandlerImpl::generateUserInterface()
  {
    using namespace ftxui;
    MenuOption command_options;
    command_options.on_enter = [this] { committed_command = selected_command; };
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
      if (committed_command >= 0 && committed_command < static_cast<int>(command_descriptors.size())) {
        const CommandDescriptor& descriptor = command_descriptors[static_cast<std::size_t>(committed_command)];
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
        "Execute Command", [&] { executeCurrentCommand(); }, action_button_options);
    Component clear_payload_button = Button(
        "Clear Payload", [&] { clearPayload(); }, action_button_options);
    Component clear_result_button = Button(
        "Clear Result", [&] { clearResult(); }, action_button_options);

    Component command_pane_container = Container::Horizontal({ command_menu_complete, payload_complete });
    Component action_menu = Container::Vertical({ execute_command_button, clear_payload_button, clear_result_button });
    Component command_action_container = Container::Vertical({ command_pane_container, action_menu });

    Component action_status = Renderer([this] {
      if (executing) {
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

    Component command_result_pane = Renderer([&] {
      std::lock_guard<std::mutex> lock(result_mutex);
      Elements result_content;
      if (command_result.has_value()) {
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

    d->ui_thread = std::thread(&CliUiHandler::CliUiHandlerImpl::generateUserInterface, d.get());
  }

  void CliUiHandler::stop()
  {
    if (!d->running) {
      return;
    }

    if (d->execute_thread.joinable()) {
      std::string waiting_for = "a command";
      if (d->committed_command >= 0 && d->committed_command < static_cast<int>(d->command_descriptors.size())) {
        waiting_for = std::string(d->command_descriptors[static_cast<std::size_t>(d->committed_command)].name);
      }
      d->app_status = "Exiting - waiting for: " + waiting_for;
      if (ftxui::App::Active() != nullptr) {
        ftxui::App::Active()->RequestAnimationFrame();
      }
      d->execute_thread.join();
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