#ifndef __SP_ICON_H__
#define __SP_ICON_H__

/*
 * Generic icon widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>



#define SP_TYPE_ICON (sp_icon_get_type ())
#define SP_ICON(o) (GTK_CHECK_CAST ((o), SP_TYPE_ICON, SPIcon))
#define SP_IS_ICON(o) (GTK_CHECK_TYPE ((o), SP_TYPE_ICON))

#define SP_ICON_SIZE_BORDER 8
#define SP_ICON_SIZE_BUTTON 16
#define SP_ICON_SIZE_MENU 12
#define SP_ICON_SIZE_TITLEBAR 12
#define SP_ICON_SIZE_NOTEBOOK 20

#include <gtk/gtkwidget.h>

struct SPIcon {
	GtkWidget widget;

	int size;

	GdkPixbuf *pb;
};

struct SPIconClass {
	GtkWidgetClass parent_class;
};

GType sp_icon_get_type (void);

GtkWidget *sp_icon_new (unsigned int size, const gchar *name);
GtkWidget *sp_icon_new_scaled (unsigned int size, const gchar *name);
GtkWidget *sp_icon_new_from_data (unsigned int size, const guchar *px);

/* This is unrelated, but can as well be here */

guchar *sp_icon_image_load (const gchar *name, unsigned int size, unsigned int scale);
guchar *sp_icon_image_load_gtk (GtkWidget *widget, const gchar *name, unsigned int size, unsigned int scale);


#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>
#include <map>

class PixBufFactory {
  //Singleton class handling pixbufs from icons.svg
public :
  static PixBufFactory &get();
  class ID {
  public :
    ID(Glib::ustring id, unsigned int size , unsigned int scale ):
      _id(id), _size(size), _scale(scale) {}
    Glib::ustring id() const {return _id;}
    int size() const {return _size;}
    int scale() const {return _scale;}
  private :
    Glib::ustring _id;
    unsigned int _size;
    unsigned int _scale;
  };


  const Glib::RefPtr<Gdk::Pixbuf> getFromSVG(const Glib::ustring &oid) {
    ID id (oid, 20, 20);
    return getFromSVG(id);
  }
  const Glib::RefPtr<Gdk::Pixbuf> getFromSVG(const ID &id);

private :
  PixBufFactory ();
  struct cmpID
  {
    bool operator()(const ID &i1, const ID &i2) const
    {
      int cmp = i1.id().compare(i2.id());
      if (cmp == 0)
	{
	  if (i1.size() == i2.size()) return i1.scale() < i2.scale() ;
	  return i1.size() < i2.size();
	}
      return cmp < 0;
    }
  };
  std::map<ID, Glib::RefPtr<Gdk::Pixbuf>, cmpID> _map;
};

#endif
