#include "dialogs/dockable.h"
#include "dialogs/docker.h"

#include "prefs-utils.h"

#include <iostream> //TODO : remove
void Dockable::present()
  {
    if ( !_pDocker)
      {
	Docker *pD = new Docker();
        pD->dock(*this); //assume _pDocker is NULL
        _pDocker = pD;
      }
    _pDocker->present(*this);
}

Dockable::~Dockable()
{
 
}
void Dockable::savePosition()
{
 if(_pDocker) 
    {
      int x, y, w, h;
      _pDocker->get_geometry(x, y, w, h);
      prefs_set_int_attribute (_prefs_path.c_str(), "x", x);
      prefs_set_int_attribute (_prefs_path.c_str(), "y", y);
      prefs_set_int_attribute (_prefs_path.c_str(), "w", w);
      prefs_set_int_attribute (_prefs_path.c_str(), "h", h);
    }
}

bool Dockable::hide( GdkEventAny *event)
{
  savePosition();
  return false;
}


void Dockable::get_prefered_geometry(int &x, int &y, int &w, int &h)
{
  if (_pDocker ) //Already docked, return the current docker geometry
    {
      _pDocker->get_geometry(x, y, w, h);
    }
  else //Not docked, get from prefs file
    {
      x = prefs_get_int_attribute (_prefs_path.c_str(), "x", 0);
      y = prefs_get_int_attribute (_prefs_path.c_str(), "y", 0);
      w = prefs_get_int_attribute (_prefs_path.c_str(), "w", 0);
      h = prefs_get_int_attribute (_prefs_path.c_str(), "h", 0);
    }
  
}
