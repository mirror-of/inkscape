#include "dialogs/docker.h"
#include  "dialogs/dockable.h"
#include "dialogs/align.h"
#include "dialogs/dialog-events.h"
#include "inkscape.h"
#include "desktop.h"
#include "display/sp-canvas.h"

#include <gtk/gtk.h> // temporary

#include <iostream> // TODO : delete
//Docker class
//TODO : the docker is instanciated by a dialog but never deleted.
//This is a LEAK TM
// TODO : fetch and set geometry
Docker::Docker( Gtk::Window &desktopWindow) : 
  _menu(*this)
{
  init();
};

Docker::Docker( ) 
  : _menu(*this) 
{
  init();
}

Docker::~Docker()
{
}
void Docker::dock ( Dockable & dockable) 
{
  geometry_request(dockable); 
  _docked.push_back(&dockable);
  _menu.add(dockable);
  dockable.set_page_num(_notebook.get_n_pages());
  _notebook.append_page(dockable.get_main_widget(), dockable.get_title());

  _window.signal_delete_event().connect(sigc::mem_fun(dockable, &Dockable::hide));

  dockable.get_main_widget().show();
  present(dockable);
}


//bool Docker::on_delete_event(GdkEventAny *event)
//{
//  for_each_dockable(&Dockable::save_geometry);
//  Gtk::Widget
//}
void Docker::present( Dockable & dockable)
{
  _notebook.set_current_page(dockable.get_page_num());
  _window.set_title(dockable.get_title());
  present();
}

void Docker::init()
{
  _window.set_title("Docker");
  _window.set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);	
  _window.set_skip_taskbar_hint(true);
  _window.set_skip_pager_hint(true);

    { 
    // This block is a much simplified version of the code used in all other dialogs for
    // saving/restoring geometry, transientising, passing events to the aplication, and
    // hiding/unhiding on F12. This code fits badly into gtkmm so it had to be abridged and
    // mutilated somewhat. This block should be removed when the same functionality is made
    // available to all gtkmm dialogs via a base class.
        GtkWidget *dlg = GTK_WIDGET(_window.gobj());

        gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);

        sp_transientize (dlg);
                           
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );
    }


  _window.add(_notebook);
  _notebook.show();
  _notebook.signal_button_press_event().connect(sigc::mem_fun(*this, &Docker::on_click));        
  _notebook.set_border_width(4);
  
}
bool Docker::on_click(GdkEventButton* e)
{ 
  _menu.popup();
  return true;
}
void Docker::get_geometry(int &x, int &y, int &w, int &h)
{
  _window.get_size(w, h);
  _window.get_position(x, y);
}

void Docker::geometry_request(Dockable &dockable)
{
  int x, y, w, h;
  dockable.get_prefered_geometry(x, y, w, h);
  if (_docked.empty())
    {
      _window.move(x, y);
    }
  else
    {
      int xc, yc, wc, hc;
      get_geometry(xc, yc, wc, hc);
      if (wc > w) w=wc;
      if (hc > h) h=hc;

    }
  _window.resize(w, h);
}

//class Docker::Menu
//used for the right mouse button click contextual menu
Docker::Menu::Menu( Docker &docker) : 
  _docker(docker), 
  _removeItem("Remove tab"), 
  _addItem("Add tab"), 
  _align("Align"), 
  _fonts("Fonts")
{
  _popup.attach(_removeItem, 0, 1, 0, 1);            
  _popup.attach(_addItem, 0, 1, 1, 2);            
  _removeItem.set_submenu(_removePopup);            
  _addItem.set_submenu(_addPopup);            
  _popup.show();
  _removeItem.show();
  _addItem.show();
  _align.show();
  _fonts.show();
  _addPopup.attach(_align, 0, 1, 0, 1);
  _align.signal_activate().connect(sigc::mem_fun(*this, &Menu::add_align_dockable));
  _addPopup.attach(_fonts, 0, 1, 1, 2);
}

void Docker::Menu::add_align_dockable()
{
  //  _docker.dock(* new DialogAlign());
}
void Docker::Menu::add(Dockable &dockable)
{
  Gtk::MenuItem *pItem = new Gtk::MenuItem(dockable.get_title());
  _items[&dockable] = pItem;
  pItem->show();
  _removePopup.attach(*pItem, 0, 1, 0, 1);
}
