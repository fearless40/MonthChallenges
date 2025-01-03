#include "commandline.hpp"
#include "generatetest.hpp"
#include "runtests.hpp"
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

    std::cout << "0 in base 26 " << RowCol{0, 0}.as_excel_fmt() << '\n';
    std::cout << "1 in base 26 " << RowCol{0, 1}.as_excel_fmt() << '\n';
    std::cout << "25 in base 26 " << RowCol{0, 25}.as_excel_fmt() << '\n';
    std::cout << "26 in base 26 " << RowCol{0, 26}.as_excel_fmt() << '\n';
    std::cout << "702 in base 26 " << RowCol{0, 702}.as_excel_fmt() << '\n';
    std::cout << "703 in base 26 " << RowCol{0, 703}.as_excel_fmt() << '\n' << '\n';

    for (std::uint16_t i = 0; i < 200; i += 25)
    {
        std::string colFormat = RowCol{i, i}.as_excel_fmt();
        std::cout << i << " in base 26 " << colFormat << " back to number format "
                  << RowCol::from_string(colFormat).as_colrow_fmt() << '\n';
    }

    return 0;

    auto options = CommandLine::parse(argc, argv);
    if (!options)
        return 0;

    auto opt = options.value();

    switch (options.value().mode)
    {
    case CommandLine::RunMode::Generate:
        std::cout << opt.testFile << '\n';
        return generate_tests_cmd_line(opt.testFile, opt.tests, opt.huge, opt.overwrite);

    case CommandLine::RunMode::Run:
        std::cout << "Testing reading and then writing config file out..." << '\n';
        return main_run_tests(opt.testFile, opt.testProgram);

    default:
        break;
    }

    return 0;
}