#include "showreport.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/flexbox_config.hpp"
#include "programoptions.hpp"
#include "testrunner.hpp"
#include "virtualgames.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

#include <vector>
namespace ui {
void start(ProgramOptions::Options const &opt)
/*std::vector<TestRunner> &results*/ {
  using namespace ftxui;
  // Start display things on the screen usign fxtui

  std::string input_text;
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto button_quit = Button("Quit", screen.ExitLoopClosure());

  auto header = Renderer(button_quit, [&] {
    return vbox(
        {hbox({text(std::format("Results from: {}", opt.program_to_test)) |
                   hcenter | vcenter | bold,
               button_quit->Render() | xflex}),
         separator()});
  });
  auto input_random = Input(&input_text, "Write something here...");

  auto component = Renderer(input_random, [&] {
    return hbox({text("Write something:"), input_random->Render()});
  });

  auto component2 = Button("Wow", [] {});
  int slider_value = 0;
  auto component3 = Slider("Slide me", &slider_value, 0, 100);

  int tab_selected = 0;
  std::vector<std::string> tab_header_strings = {"Comp1", "Comp2", "Comp3"};
  auto tab_header = Toggle(tab_header_strings, &tab_selected);
  auto tabs =
      Container::Tab({component, component2, component3}, &tab_selected);

  screen.Loop(Container::Vertical({header, tab_header, tabs}));

  // screen.Loop(component);
}
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

} // namespace ui
