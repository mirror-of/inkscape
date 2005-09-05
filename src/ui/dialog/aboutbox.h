/**
 * \brief About Widget - Adds the "about" doc to the Gnome::UI::About Class
 *
 * The standard Gnome::UI::About class doesn't include a place to stuff
 * a renderable View that holds the classic Inkscape "about.svg".
 *
 * Author:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2005 Kees Cook
 *
 * Released under GNU GPL v2+.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_ABOUTBOX_H
#define INKSCAPE_UI_DIALOG_ABOUTBOX_H

#include <gtkmm.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

#define INKSCAPE_ABOUT_CREDITS  1
#define INKSCAPE_ABOUT_LICENSE  2
#define INKSCAPE_ABOUT_CLOSE    3

class AboutBoxChild: public Gtk::Dialog
{
public:
    AboutBoxChild::AboutBoxChild(Gtk::Window& parent, gchar * title)
        : Gtk::Dialog(title,parent) {};
protected:
    virtual void         on_response(int response_id);
    Gtk::ScrolledWindow& make_scrolled_text(Glib::ustring& contents);
};

class LicenseBox: public AboutBoxChild
{
public:
    LicenseBox(Gtk::Window& parent, Glib::ustring& text);
//protected:
//    Gtk::ScrolledWindow  _scrolled;
//    Gtk::TextView        _textview;
};

class CreditsBox: public AboutBoxChild
{
public:
    CreditsBox(Gtk::Window& parent,
               std::vector<Glib::ustring>& authors,
               std::vector<Glib::ustring>& translators);
protected:
    Gtk::Notebook  _notebook;
private:
    void flatten_vector(std::vector<Glib::ustring>& list,
                        Glib::ustring& string);
};

class AboutBox: public Gtk::Dialog
{
public:
    AboutBox(Gtk::Widget& about_svg_view, gint width, gint height);
protected:
    virtual void    on_response(int response_id);
    
    void            show_credits(void);
    void            show_license(void);

    LicenseBox *    _license;
    CreditsBox *    _credits;

/*
    Gtk::Dialog *  _about;
*/
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_ABOUTBOX_H

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
