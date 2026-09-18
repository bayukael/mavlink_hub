#include "app/ui_handler/CliUiHandler.h"

#include "common/types/OperationResult.h"

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

    std::optional<CommandResult> command_result;
    std::mutex result_mutex;
    bool executing = false;
    std::thread execute_thread;

    bool running = false;
    std::thread ui_thread;

    void generateUserInterface();
    void executeCurrentCommand();
    void executeCommandAsync(const UserCommand& cmd);
    std::string actionStatusText();
  };

  CliUiHandler::CliUiHandlerImpl::CliUiHandlerImpl(IAppService& appsrv) : app_service(appsrv)
  {
    command_descriptors = app_service.getCommandDescriptors();
    for (const CommandDescriptor& descriptor : command_descriptors) {
      command_entries.push_back(std::string(descriptor.name));
    }
  }

  std::string CliUiHandler::CliUiHandlerImpl::actionStatusText()
  {
    if (executing) {
      return "Busy - a command is running";
    }
    if (committed_command < 0 || committed_command >= static_cast<int>(command_descriptors.size())) {
      return "No command selected";
    }
    return "Ready to execute command";
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
    executing = true;

    execute_thread = std::thread([this, cmd]() {
      CommandResult result = app_service.executeCommand(cmd);

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

  void CliUiHandler::CliUiHandlerImpl::generateUserInterface()
  {
    using namespace ftxui;

    MenuOption command_options;
    command_options.on_enter = [this] { committed_command = selected_command; };

    Component command_menu = Menu(&command_entries, &selected_command, command_options);
    Component payload_input = Input(&payload, "payload (JSON)");

    MenuOption action_options;
    action_options.on_enter = [this] {
      switch (selected_action) {
        case 0: executeCurrentCommand(); break;
        case 1: payload.clear(); break;
        case 2: {
          std::lock_guard<std::mutex> lock(result_mutex);
          command_result.reset();
          break;
        }
        default: break;
      }
    };

    Component action_menu = Menu(&action_entries, &selected_action, action_options);

    Component result_pane = Renderer([this] {
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
      return vbox(text("Command Result") | bold | center, separator(), vbox(std::move(result_content)) | frame | vscroll_indicator) |
             border;
    });

    // clang-format off
    Component command_box = Renderer(command_menu, [&] {
      return vbox(
        text("Command Lists") | bold | center, separator(),
        hbox(command_menu->Render(), filler()) | vscroll_indicator | frame
      ) | border;
    });
    Component payload_box = Renderer(payload_input, [&] {
      return vbox(
        text("Command Payload") | bold | center,
        separator(),
        payload_input->Render()
      ) | border;
    });
    Component action_box = Renderer(action_menu, [&] {
      return vbox(
        text("Action") | bold | center,
        separator(),
        action_menu->Render()
      ) | border;
    });
    // clang-format on

    Component selected_command_box = Renderer([this] {
      std::string selected_name = "none";
      std::string payload_hint = "-";
      if (committed_command >= 0 && committed_command < static_cast<int>(command_descriptors.size())) {
        const CommandDescriptor& descriptor = command_descriptors[static_cast<std::size_t>(committed_command)];
        selected_name = std::string(descriptor.name);
        payload_hint = descriptor.requires_payload ? "required" : "not required";
      }
      return vbox(text("Selected Command") | bold | center, separator(),
                  vbox({
                      text(selected_name),
                      text("payload: " + payload_hint) | dim,
                  })) |
             border;
    });

    Component action_status_box = Renderer([this] {
      return vbox(text("Action Status") | bold | center, separator(),
                  vbox({
                      text(actionStatusText()) | dim,
                  })) |
             border;
    });

    Component third_column = Container::Vertical({ action_box }) | Renderer([this, selected_command_box, action_status_box](Element inner) {
                               return vbox({
                                          selected_command_box->Render(),
                                          inner,
                                          action_status_box->Render(),
                                      }) |
                                      size(WIDTH, EQUAL, 30) | flex_shrink_factor(0);
                             });

    Component top_row = Container::Horizontal({
        command_box | size(WIDTH, EQUAL, 32) | flex_shrink_factor(0),
        payload_box | flex_factor(1, 1),
        third_column,
        result_pane | flex_factor(2, 1),
    });

    auto layout = top_row | Renderer([this](Element inner) {
                    return vbox({
                               text("MAVLink Hub - Command Execution UI") | bold | center | border,
                               inner | flex,
                               text("Application Status: " + app_status) | center | border,
                           }) |
                           border;
                  });

    std::vector<Component> tab_boxes{ command_box, payload_box, action_box };

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