#include "RowCol.hpp"
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/string.hpp>

namespace Widgets {
using namespace ftxui;

class DynamicTab : public ComponentBase {
private:
  struct TabMap {
    std::size_t id;
    ftxui::Component tab_button;
    ftxui::Component tab_content;
  };

  std::size_t selected;
  int fake_selected;
  size_t nextID;

  Component vert_c;
  ftxui::Component tabs_c;
  ftxui::Component container_c;
  std::vector<TabMap> tab_ID; // map of ID to position

public:
  DynamicTab() : selected(0), nextID(0) {
    tabs_c = ftxui::Container::Horizontal({});

    container_c = ftxui::Container::Tab({}, &fake_selected);
    vert_c = Container::Vertical({tabs_c, container_c});
  }

  void set_active_tab(std::size_t tabID) {
    auto found = std::ranges::find(tab_ID, tabID, &TabMap::id);
    if (found != tab_ID.end()) {
      container_c->SetActiveChild(found->tab_content);
    }
  }

  void close_tab(std::size_t tabID) {
    if (container_c->ChildCount() == 1)
      return;

    if (auto pairfind = std::ranges::find(tab_ID, tabID, &TabMap::id);
        pairfind != tab_ID.end()) {
      pairfind->tab_button->Detach();
      pairfind->tab_content->Detach();

      auto before = pairfind - 1;
      container_c->SetActiveChild(before->tab_content);
      tabs_c->SetActiveChild(before->tab_button);

      tab_ID.erase(pairfind);
    }
  }

  void add_tab(std::string label, bool show_close, ftxui::Component comp) {
    auto tbutton = TabButton(label, nextID, show_close);

    tabs_c->Add(tbutton);
    container_c->Add(comp);
    tab_ID.emplace_back(nextID, tbutton, comp);
    set_active_tab(nextID);

    ++nextID;
  }

  ftxui::Component TabButton(std::string const &label, std::size_t tabID,
                             bool show_close) {
    auto transform = [](const ftxui::EntryState &s) {
      auto element = ftxui::text(s.label);
      if (s.focused)
        element |= ftxui::bold;
      return element;
    };

    ftxui::ButtonOption label_btn_option;
    label_btn_option.label = label + (show_close ? "  " : "");
    label_btn_option.transform = transform;
    label_btn_option.on_click =
        std::bind(&DynamicTab::set_active_tab, this, tabID);

    ftxui::ButtonOption close_btn_option;
    close_btn_option.label = "X";
    close_btn_option.transform = transform;
    close_btn_option.on_click = std::bind(&DynamicTab::close_tab, this, tabID);

    if (show_close) {
      auto container = ftxui::Container::Horizontal(
          {ftxui::Button(label_btn_option), ftxui::Button(close_btn_option)});

      return ftxui::Renderer(container, [container] {
        return ftxui::hbox(container->Render(), ftxui::separator());
      });

    } else {
      auto container =
          ftxui::Container::Horizontal({ftxui::Button(label_btn_option)});

      return ftxui::Renderer(container, [container] {
        return ftxui::hbox(container->Render(), ftxui::separator());
      });
    }
  }

  Element OnRender() final {
    // clang-format off
    return Renderer(vert_c, [this]{ 
         return vbox({
         ftxui::separator(),
         tabs_c->Render(),
         separatorDouble(),
         container_c->Render()
      });
      })->Render();
      //clang-format on
  }

   bool Focusable() const final { return true; }

   bool OnEvent(Event e) override { 
      return vert_c->OnEvent(e);
   }
};

  class SelectableTable : public ComponentBase {
  private:
    std::size_t m_total_rows;
    std::size_t m_current_selected_row;

    Components m_cells;
    Component m_rows;
    std::vector<std::string> m_headers;

    struct SelectableCell : ComponentBase {
      SelectableCell(std::size_t id, const std::string text,
                     std::size_t *current_selection)
          : row_id(id), cell_text(text),
            current_selected_id(current_selection) {}

      std::size_t row_id;
      std::string cell_text;
      std::size_t *current_selected_id;
      Box box;

      Element OnRender() final {
        Element element;

        if (*current_selected_id == row_id)
          element =
              text(cell_text) | ftxui::bgcolor(Color(40, 200, 80)) | focus;
        else
          element = text(cell_text);

        element |= reflect(box);
        const int change = 10;
        box.x_min -= change;
        box.x_max += change;
        box.y_min -= change;
        box.y_max += change;
        return element;
      }

      bool Focusable() const final { return true; }

      bool OnEvent(Event event) final {
        bool mouse_hover = box.Contain(event.mouse().x, event.mouse().y) &&
                           CaptureMouse(event);

        if ((event.mouse().button == Mouse::Left &&
             event.mouse().motion == Mouse::Pressed && mouse_hover) ||
            event == Event::Return) {
          *current_selected_id = row_id;
          TakeFocus();
          return true;
        }
        return false;
      }
    };

  public:
    int left_size;
    SelectableTable(std::vector<std::string> &&header,
                    std::vector<std::string> const &data)
        : m_headers(header), m_total_rows(0), left_size(40),
          m_current_selected_row(0) {
      m_rows = Container::Vertical({});
      m_total_rows = data.size() / m_headers.size();
      m_cells.reserve(data.size());
      std::size_t cols_count = m_headers.size();

      // log << "Header Size: " << m_headers.size() << '\n';
      // log << "Data Size: " << data.size() << '\n';
      // log << "Number of rows: " << m_total_rows << std::endl;

      for (std::size_t row_id = 0; row_id < m_total_rows; ++row_id) {
        Component row = Container::Horizontal({});
        for (std::size_t col_id = 0; col_id < cols_count; ++col_id) {
          auto cell =
              Make<SelectableCell>(row_id, data[(cols_count * row_id) + col_id],
                                   &m_current_selected_row);
          m_cells.push_back(cell);
          row->Add(cell);
        }
        m_rows->Add(row);
      }
      // log << "Finished with constructor." << std::endl;
    }

    Element OnRender() override {

      std::vector<Elements> table_elements;
      Elements headers;
      // Header of table
      std::transform(m_headers.begin(), m_headers.end(),
                     std::back_inserter(headers),
                     [](auto const &value) { return text(value); });

      // log << "On render: after header manipulation." << std::endl;
      table_elements.push_back(std::move(headers));

      const auto col_count = m_headers.size();

      for (std::size_t row = 0; row < m_total_rows; ++row) {
        Elements row_values;
        for (std::size_t col = 0; col < col_count; ++col) {
          row_values.push_back(m_cells[(row * col_count) + col]->Render());
        }
        table_elements.push_back(std::move(row_values));
      }
      // log << "After making table of elements." << std::endl;

      auto table = Table(table_elements);
      table.SelectAll().Border(ftxui::LIGHT);
      table.SelectAll().Separator(ftxui::LIGHT);
      return table.Render() | vscroll_indicator |
             focusPosition(0, m_current_selected_row * 2) | frame;
    }

    bool OnEvent(Event e) override {
      if (e == Event::ArrowDown) {
        if (m_current_selected_row < m_total_rows) {
          ++m_current_selected_row;
        }
        return true;
      }

      else if (e == Event::ArrowUp) {
        if (m_current_selected_row != 0)
          --m_current_selected_row;
        return true;
      }

      else
        return m_rows->OnEvent(e);
    }

    bool Focusable() const final { return true; }

    const std::size_t get_selected_row() { return m_current_selected_row; }
  };



class GameBoard : ComponentBase { 
public:
   struct DisplayPoint { 
      battleship::RowCol pos;
      Color color; 
   };

   
   std::size_t m_row_count;

   std::size_t m_col_count;

   bool show_borders;
   Color border_color;
   bool bold_col_headers;
   bool bold_row_headers;


   Element OnRender() final { 

   
   }

private: 
   Component m_rows; 
   std::vector<DisplayPoint> m_display; 

};
}// namespace Widgets
