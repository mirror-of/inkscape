#ifndef SEEN_SP_ICON_H
#define SEEN_SP_ICON_H

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

#include <gtk/gtkwidget.h>

struct SPIcon {
    GtkWidget widget;

    GtkIconSize lsize;
    int psize;
    gchar *name;

    GdkPixbuf *pb;
    GdkPixbuf *pb_faded;
};

struct SPIconClass {
    GtkWidgetClass parent_class;
};

GType sp_icon_get_type (void);

GtkWidget *sp_icon_new( GtkIconSize size, const gchar *name );
GtkWidget *sp_icon_new_scaled( GtkIconSize size, const gchar *name );


#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>
#include <map>

class PixBufFactory {
  //Singleton class handling pixbufs from icons.svg
public :
  static PixBufFactory &get();
  class ID {
  public :
    ID(Glib::ustring id, GtkIconSize size , unsigned int psize );
    Glib::ustring id() const {return _id;}
    int size() const {return _size;}
    int psize() const {return _psize;}
  private :
    Glib::ustring _id;
    GtkIconSize _size;
    unsigned int _psize;
  };


  const Glib::RefPtr<Gdk::Pixbuf> getIcon(const Glib::ustring &oid);
  const Glib::RefPtr<Gdk::Pixbuf> getIcon(const Glib::ustring &oid, GtkIconSize size);
  const Glib::RefPtr<Gdk::Pixbuf> getIcon(const ID &id);

private :
  PixBufFactory ();
  struct cmpID
  {
    bool operator()(const ID &i1, const ID &i2) const
    {
      int cmp = i1.id().compare(i2.id());
      if (cmp == 0)
      {
          if (i1.size() == i2.size()) return i1.psize() < i2.psize() ;
          return i1.size() < i2.size();
      }
      return cmp < 0;
    }
  };
  std::map<ID, Glib::RefPtr<Gdk::Pixbuf>, cmpID> _map;
};

#endif // SEEN_SP_ICON_H
