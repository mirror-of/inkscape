#ifndef __DOCKABLE_H__
#define __DOCKABLE_H__
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

#include <glibmm/ustring.h>

class Docker;
#include <gtkmm/container.h>


class Dockable {
public : 
  Dockable(Glib::ustring title, Glib::ustring prefs_path) 
    : _title(title), _prefs_path(prefs_path), _pDocker(0), _page_num(-1) {}
  virtual ~Dockable();
  Glib::ustring get_title() const {return _title;}
  virtual Gtk::Container & get_main_widget() = 0;
  void set_page_num(int page) {_page_num = page;}
  int get_page_num() {return _page_num;}
  void present();
  bool hide(GdkEventAny *event);
  void get_prefered_geometry(int &x, int &y, int &w, int &h);
private :
  void savePosition();
  Glib::ustring _title, _prefs_path;
  Docker *_pDocker;
  int _page_num; 
  

};
#endif //__DOCKABLE_H__
