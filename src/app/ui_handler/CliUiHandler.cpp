#include "app/ui_handler/CliUiHandler.h"

#include "common/types/OperationResult.h"

#include <ftxui.hpp>
#include <memory>
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
    int committed_command = 0;
    int selected_action = 0;
    std::string payload;

    bool running = false;
    std::thread ui_thread;
  };

  CliUiHandler::CliUiHandlerImpl::CliUiHandlerImpl(IAppService& appsrv) : app_service(appsrv)
  {
    command_descriptors = app_service.getCommandDescriptors();
    for (const CommandDescriptor& descriptor : command_descriptors) {
      command_entries.push_back(std::string(descriptor.name));
    }
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

    d->ui_thread = std::thread([this]() {
      using namespace ftxui;

      MenuOption command_options;
      command_options.on_enter = [this] { d->committed_command = d->selected_command; };

      auto command_menu = Menu(&d->command_entries, &d->selected_command, command_options);
      auto payload_input = Input(&d->payload, "payload (JSON)");

      auto action_menu = Menu(&d->action_entries, &d->selected_action);

      auto result_pane = Renderer([] {
        return window(
            text("Command Result") | bold,
            vbox({
                text("(empty)") | dim,
            }) |
                frame | vscroll_indicator);
      });

      auto command_box = command_menu | Renderer([](Element inner) { return window(text("Command Lists") | bold, inner); });
      auto payload_box = payload_input | Renderer([](Element inner) { return window(text("Command Payload") | bold, inner); });
      auto action_box = action_menu | Renderer([](Element inner) { return window(text("Action") | bold, inner); });

      auto selected_command_box = Renderer([this] {
        std::string selected_name = "?";
        std::string payload_hint = "?";
        if (d->committed_command >= 0 && d->committed_command < static_cast<int>(d->command_descriptors.size())) {
          const CommandDescriptor& descriptor = d->command_descriptors[static_cast<std::size_t>(d->committed_command)];
          selected_name = std::string(descriptor.name);
          payload_hint = descriptor.requires_payload ? "required" : "not required";
        }
        return window(
            text("Selected Command") | bold,
            vbox({
                text(selected_name),
                text("payload: " + payload_hint) | dim,
            }));
      });

      auto third_column =
          Container::Vertical({ action_box }) | Renderer([this, selected_command_box](Element inner) {
            return vbox({
                       selected_command_box->Render() | flex_shrink_factor(0),
                       separator(),
                       inner | flex,
                   }) |
                   size(WIDTH, EQUAL, 30) | flex_shrink_factor(0);
          });

      auto top_row = Container::Horizontal({
          command_box | size(WIDTH, EQUAL, 32) | flex_shrink_factor(0),
          payload_box | flex_factor(1, 1),
          third_column,
          result_pane | flex_factor(2, 1),
      });

      auto layout = top_row | Renderer([](Element inner) {
                      return vbox({
                                 text("MAVLink Hub - Command Execution UI") | bold | center | border,
                                 inner | flex,
                             }) |
                             border;
                    });

      auto app = App::Fullscreen();
      app.Loop(layout);
    });
  }

  void CliUiHandler::stop()
  {
    if (!d->running) {
      return;
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