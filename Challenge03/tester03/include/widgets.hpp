#include "baseconv.hpp"
#include <cstddef>
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
#include <iterator>
#include <string>

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


class SelectableCell : public ComponentBase {

private:
      std::size_t row_id;
      std::size_t user_value; 
      std::size_t *current_selected_id;
      std::string cell_text; 
      Box box;

public:
      SelectableCell(std::size_t id, const std::string text, std::size_t user_val, 
                     std::size_t *current_selection)
          : row_id(id), cell_text(text), user_value(user_val),
            current_selected_id(current_selection) {}

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
         if( event.is_mouse() ) return OnMouse(event); 


         // if( event == Event::Return) {
         //  *current_selected_id = row_id;
         //  TakeFocus();
         //  return true;
      //}       
        return false;
      }
     

      bool OnMouse( Event event ) { 
         bool mouse_hover = box.Contain(event.mouse().x, event.mouse().y) &&
                           CaptureMouse(event);

         if ((event.mouse().button == Mouse::Left &&
             event.mouse().motion == Mouse::Pressed && mouse_hover) )
         {
            TakeFocus();
            *current_selected_id = row_id; 
            return true;
         }

         return false;
      }
};

   
class SelectableTable : public ComponentBase { 
  private:
    std::size_t m_total_rows;
    std::size_t m_current_selected_row;

    Components m_cells;
    Component m_rows;
    std::vector<std::string> m_headers;

      Box m_box;


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
              Make<SelectableCell>(row_id, data[(cols_count * row_id) + col_id],0,
                                   &m_current_selected_row);
          m_cells.push_back(cell);
          row->Add(cell);
        }
        Add(row);
        // m_rows->Add(row);
      }
      // log << "Finished with constructor." << std::endl;
    }

    void set_data( std::vector<std::string> const & data ) { 

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
      auto elements =  table.Render() | vscroll_indicator |
             focusPosition(0, m_current_selected_row * 2) | frame | flex_shrink;

      elements |= reflect(m_box); 
      return elements; 
    }

    void advance_active_row( int amount ) { 
      
      m_current_selected_row = (std::size_t) std::max(0, std::min( (int)m_total_rows-1, (int)m_current_selected_row + amount)); 

      auto child =  ChildAt(m_current_selected_row);
      if( child && child->Focusable() ) { 
         SetActiveChild(child);
      }

   }


    bool OnEvent(Event e) override {
      for( auto & child : children_ ) { 
         if(child->Focusable() ) {
            auto val = child->OnEvent(e);
            if(val) return true; 
         }
      }
if( e.is_mouse() ) return OnMouseEvent(e);


      if( !Active() || !ActiveChild() ) return false;

      if (e == Event::ArrowDown) {
         advance_active_row(1);
         return true;
      }

      else if (e == Event::ArrowUp) {
         advance_active_row(-1);
         return true;
      }
      else if (e ==Event::PageDown) {
         advance_active_row(5);
         return true;
      }
      else if (e == Event::PageUp) { 
         advance_active_row(-5);
         return true;
      }

      // else
      //   return m_rows->OnEvent(e);
      return false;
    }

    bool OnMouseEvent(Event e) { 

     if( !m_box.Contain( e.mouse().x, e.mouse().y )  ) return false; 

      if( e.mouse().button == Mouse::WheelUp ) { 
         advance_active_row(-1);     
         return true;
      }
      else if( e.mouse().button == Mouse::WheelDown ) {
         advance_active_row(1);
         return true; 
      }

     return false; 
      // return m_rows->OnEvent(e);

   }

    bool Focusable() const final { return true; }

    const std::size_t get_selected_row() { return m_current_selected_row; }
  };



class GameBoard : public ComponentBase { 
public:
   struct DisplayPoint { 
      Color color {255,255,255};
      Color bgColor { 0,0,0 };
      std::string value;
   };



   int x_selected {0}; 
   int y_selected {0};
   std::size_t m_row_count {0};

   std::size_t m_col_count {0}; 

   bool show_borders;
   Color border_color;
   bool bold_col_headers;
   bool bold_row_headers;

   void set_board( std::vector<DisplayPoint> && data, std::size_t col_size ) { 
      m_col_count = col_size;
      m_row_count = data.size() / col_size; 

      // Reset ftxui data
      m_rows = nullptr; 
      m_rows = Container::Vertical({}, &x_selected); 
      m_values.clear(); 

      for( std::size_t row = 0; row < m_row_count; ++row) { 
         auto row_component = Container::Horizontal({}, &y_selected); 
         for(std::size_t col = 0; col < m_col_count; ++col) { 
            m_values.push_back( Renderer([cell = data[row*m_col_count + col]](bool focused) { 
               if( focused ) 
                  return text(cell.value) | color( cell.color) | bgcolor(cell.bgColor) | bold; 
         
               else 
                  return text(cell.value) | color( cell.color) | bgcolor( cell.bgColor ); 
            }));
            row_component->Add(m_values.back());
         }
         m_rows->Add(row_component);
      }
   }



   Element OnRender() final { 

      if( m_rows == nullptr)  return text("Empty board."); 
         
         std::vector<Elements> table; 

         table.push_back(Elements{});
         table[0].push_back(text(" "));
      for( std::size_t col = 0; col < m_col_count; ++col) { 
         table[0].push_back( text(base26::to_string( col ))); 
      }

      for( std::size_t row =0; row < m_row_count; ++row) { 
         Elements row_component; 
         row_component.push_back( text( std::to_string(row)) | bold); 
         for( std::size_t col = 0; col < m_col_count; ++col ) { 
            row_component.push_back( m_values[row*m_col_count+col]->Render() );
         }
         table.push_back(std::move( row_component )); 
      }

      auto table_ren = Table( table ); 

      table_ren.SelectAll().Separator(LIGHT) ;
      

      table_ren.SelectRow(0).BorderBottom(BorderStyle::DOUBLE);
      table_ren.SelectColumn(0).BorderRight(DOUBLE); 

      return table_ren.Render(); 
     
      std::vector<Elements> ele; 
      
      const std::size_t row_elements = 2*m_row_count+1; 
      const std::size_t col_elements = 2*m_col_count+1; 

      ele.reserve(row_elements); 
      
      for( std::size_t y=0 ; y < row_elements; ++y) { 
         Elements row; 
         row.reserve((2* m_col_count) + 1); 
         for( std::size_t x=0; x < col_elements; ++x) { 
            if( y % 2 == 0 ) { // Border  
                              if (y == 2 ) { // Doubler border 
                  row.push_back( separatorCharacter("=") | automerge );
                  contiune;
               } 
               if( y == 0 ) { // Skip outer Border
                  row.push_back(emptyElement());
                  continue;
               }
               row.push_back(separatorCharacter("-") | automerge); 
            }
            if( x%2 == 0 ) { //Vertical border 
               if( x 

      // std::vector<Elements> table; 
      // const auto grid_size = 3;
      //
      // const auto cell_height = 3;
      // const auto cell_width = 2;
      //
      // auto sizeit = []( Element && el ) -> auto{ 
      //    return el   | size(WIDTH, EQUAL, grid_size + 1) | size(HEIGHT, EQUAL, grid_size) | hcenter | vcenter;
      //  };
      //
      // auto lc = Color(90,90,90);
      //
      //    table.push_back(Elements{});
      //    table[0].push_back(sizeit(text(" ")));
      // for( std::size_t col = 0; col < m_col_count; ++col) { 
      //    table[0].push_back( text("│\n│\n│") | flex_shrink | color(lc) );
      //    table[0].push_back( sizeit(text(base26::to_string( col ))) | bold ); 
      // }
      //
      // for( std::size_t row =0; row < m_row_count; ++row) { 
      //    {
      //       Elements divider; 
      //       auto div = row == 0 ? "═" : "─";
      //
      //       for( std::size_t col = 0; col < m_col_count; ++col ) { 
      //          if( col == 0 ) div = " "; 
      //          else               if( col == 1 ) div = "║";
      //          else 
      //       div = row == 0 ? "═" : "─";
      //          divider.push_back( text(div) | color(lc) | flex_shrink);
      //       }
      //       table.push_back(std::move(divider));
      //    }
      //    Elements row_component; 
      //    row_component.push_back( text( std::to_string(row+1)) | bold); 
      //    row_component.push_back( text("║") | flex_shrink | color(lc) );
      //    for( std::size_t col = 0; col < m_col_count; ++col ) { 
      //       row_component.push_back( sizeit(m_values[row*m_col_count+col]->Render()) );
      //       row_component.push_back( text("│") | flex_shrink | color(lc) );
      //    }
      //    table.push_back(std::move( row_component )); 
      // }
      //
      // return gridbox(table); 
      //



   }

   bool Focusable() const final { return true;} 

   bool OnEvent(Event e) override { 
      return m_rows->OnEvent(e);
   }

private: 
   Component m_rows; 
   Components m_values; 
   std::vector<DisplayPoint> m_display; 

};
}// namespace Widgets
