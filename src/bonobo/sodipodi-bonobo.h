#ifndef INKSCAPE_BONOBO_H
#define INKSCAPE_BONOBO_H

#include <config.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>
#include <bonobo.h>

#ifndef INKSCAPE_BONOBO_C
extern CORBA_Environment ev;
extern CORBA_ORB orb;
extern gint sp_bonobo_objects;
#else
CORBA_Environment ev;
CORBA_ORB orb;
gint sp_bonobo_objects = 0;
#endif

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
