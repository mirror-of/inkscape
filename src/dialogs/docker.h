#ifndef __DOCKER_H__
#define __DOCKER_H__

/**
 * \brief  Dialog handling Dockables
 *
 * Authors:
 *   Aubanel MONNIER <aubi@libertysurf.fr>
 *
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include "helper/sp-intl.h"

#include <gtkmm/window.h>
#include <gtkmm/notebook.h>
#include <gtkmm/menu.h>

class Dockable;

class Docker {
public : 
  // TODO : fetch and set geometry
  Docker( Gtk::Window &desktopWindow);
  Docker( );
  virtual ~Docker();
  
  void dock ( Dockable & dockable) ;
  void un_dock ( Dockable & dockable) {} //TODO
  void present() {_window.present();}
  void present( Dockable & dockable);
  void get_geometry(int &x, int &y, int &w, int &h);
  void geometry_request(Dockable &dockable);
  private :
  class Menu{
  public :
    Menu(Docker &docker) ;
    void popup(){_popup.popup(0,0);}
    void add_align_dockable();
    void add(Dockable &dockable);
  private:
    Docker &_docker;
    Gtk::MenuItem _removeItem, _addItem, _align, _fonts;
    Gtk::Menu _popup, _removePopup, _addPopup;
    std::map<Dockable *, Gtk::MenuItem *> _items;
  };
  void init();
  bool on_click(GdkEventButton* e);

  //Members
  Gtk::Window _window;
  Menu _menu;    
  Gtk::Notebook _notebook;
  std::list<Dockable *> _docked;
     
 };


#endif //__DOCKER_H__
 
