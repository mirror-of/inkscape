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

class Selection;
class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Dialog : public Gtk::Dialog {
public:
    Dialog();
    Dialog(BaseObjectType *gobj);
    virtual ~Dialog();

    virtual void onDestroy();

    virtual Glib::ustring  getName() const = 0;
    virtual Glib::ustring  getDesc() const = 0;

    /** Hide and show dialogs */
    virtual void   onHideDialogs();
    virtual void   onHideF12();
    virtual void   onShowDialogs();
    virtual void   onShowF12();

    void           transientize();

protected:
    SPDesktop      *_desktop;
    bool           _user_hidden;

    virtual void   on_response(int response_id);
    virtual void   _apply();
    virtual void   _close();

    Selection*     _getSelection();
    void           _setDesktop(SPDesktop *desktop);

private:
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
