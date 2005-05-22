/**
 * \brief Base class for dialogs in Inkscape.  This class provides certain
 *        common behaviors and styles wanted of all dialogs in the application.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_DIALOG_H
#define INKSCAPE_DIALOG_H

#include <gtkmm/dialog.h>

namespace Inkscape { class Selection; }
class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Dialog : public Gtk::Dialog {
public:
    Dialog(BaseObjectType *gobj); // fixme: remove this

    Dialog(const char *prefs_path, int verb_num, const char *apply_label = NULL);

    virtual ~Dialog();

    virtual void onDestroy();

    /** Hide and show dialogs */
    virtual void   onHideDialogs();
    virtual void   onHideF12();
    virtual void   onShowDialogs();
    virtual void   onShowF12();

    void           transientize();

    void           update_position();

    const char           *_prefs_path;

protected:
    Dialog( bool flag ); // fixme: remove this

    SPDesktop      *_desktop;
    bool           _user_hidden;

    virtual void   on_response(int response_id);
    virtual void   _apply();
    virtual void   _close();

    static bool windowKeyPress( GtkWidget *widget, GdkEventKey *event );
    static void Dialog::hideCallback(GtkObject *object, gpointer dlgPtr);
    static void Dialog::unhideCallback(GtkObject *object, gpointer dlgPtr);

    Inkscape::Selection*   _getSelection();
    void           _setDesktop(SPDesktop *desktop);

private:
    Dialog(); // no constructor without params

    Dialog(Dialog const &d);            // no copy
    Dialog& operator=(Dialog const &d); // no assign
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_DIALOG_H

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
