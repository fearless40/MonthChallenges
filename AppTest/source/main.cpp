#include "commandline.hpp"
#include "generatetest.hpp"
#include <iostream>

/*
#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Renderer, Button, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for operator|, Element, text, bold, border, center, color
#include "ftxui/screen/color.hpp"                 // for Color, Color::Red
*/

int main(int argc, char *argv[])
{
    /*    using namespace ftxui;


        auto screen = ScreenInteractive::FullscreenPrimaryScreen();

        // 1. Example of focusable renderer:
        auto renderer_focusable = Renderer([](bool focused) {
            if (focused)
                return text("FOCUSABLE RENDERER()") | center | bold | border;
            else
                return text(" Focusable renderer() ") | center | border;
        });

        // 2. Examples of a non focusable renderer.
        auto renderer_non_focusable = Renderer([&] {
            return text("~~~~~ Non Focusable renderer() ~~~~~"); //
        });

        // 3. Renderer can wrap other components to redefine their Render() function.
        auto button = Button("Wrapped quit button", screen.ExitLoopClosure());
        auto renderer_wrap = Renderer(button, [&] {
            if (button->Focused())
                return button->Render() | bold | color(Color::Red);
            else
                return button->Render();
        });

        // Let's renderer everyone:
        screen.Loop(Container::Vertical({
            renderer_focusable,
            renderer_non_focusable,
            renderer_wrap,
        }));
    */

    auto options = CommandLine::parse(argc, argv);
    if (!options)
        return 0;

    auto opt = options.value();

    switch (options.value().mode)
    {
    case CommandLine::RunMode::Generate:
        return generate_tests_cmd_line(opt.testFile, opt.tests, opt.huge, opt.overwrite);

    default:
        break;
    }

    return 0;
}