#include "showreport.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "programoptions.hpp"
#include "testrunner.hpp"
#include "virtualgames.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
namespace ui {
void start(ProgramOptions::Options const &opt)
/*std::vector<TestRunner> &results*/ {
  // Start display things on the screen usign fxtui
  //
  std::string input_text;
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto component = ftxui::Renderer([&]() {
    return ftxui::vbox(
        ftxui::text(std::format("Results from: {}", opt.program_to_test)),
        ftxui::separatorDouble(),
        ftxui::hbox(ftxui::text("Input: "),
                    ftxui::Input(&input_text, "Test me...")) |
            ftxui::border);
  });

  screen.Loop(component);
}
} // namespace ui
