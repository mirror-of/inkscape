// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef INK_DIALOG_BASE_H
#define INK_DIALOG_BASE_H

/** @file
 * @brief A base class for all dialogs.
 *
 * Authors: see git history
 *   Tavmjong Bah
 *
 * Copyright (c) 2018 Tavmjong Bah, Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glibmm/ustring.h>
#include <gtkmm/box.h>

#include "inkscape-application.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * DialogBase is the base class for the dialog system.
 *
 * Each dialog has a reference to the application, in order to update it's inner focus
 * (be it of the active desktop, document, selection, etc.) in the update() method.
 *
 * DialogsBase derived classes' instances live in DialogNotebook classes and are managed by
 * DialogContainer classes. DialogContainer instances can have at most one type of dialog,
 * differentiated by the associated type.
 */
class DialogBase : public Gtk::Box
{
    using parent_type = Gtk::Box;

public:
    DialogBase(gchar const *prefs_path = nullptr, Glib::ustring dialog_type = "");
    ~DialogBase() override {
        ensure_size();
        unsetDesktop();
    };

    /**
     * The update() method is essential to Gtk state management. DialogBase implementations get updated whenever
     * a new focus event happens if they are in a DialogWindow or if they are in the currently focused window.
     *
     * DO NOT use update to keep SPDesktop, SPDocument or Selection states, use the virtual functions below.
     */
    virtual void update() {}

    // Public for future use, say if the desktop is smartly set when docking dialogs.
    void setDesktop(SPDesktop *new_desktop);

    void on_map() override
    {
        // Update asks the dialogs if they need their Gtk widgets updated.
        update();
        // Set the desktop on_map, although we might want to be smarter about this.
        setDesktop(dynamic_cast<SPDesktop *>(_app->get_active_view()));
        parent_type::on_map();
    }
    /*
     * Often the dialog won't request the right size until the window has
     * been pushed to resize all it's children. We do this on dialog creation
     * and destruction.
     */
    void ensure_size()
    {
        if (desktop) {
            desktop->getToplevel()->resize_children();
        }
    }

    // Getters and setters
    Glib::ustring get_name() { return _name; };
    const Glib::ustring& getPrefsPath() const { return _prefs_path; }
    Glib::ustring const &get_type() const { return _dialog_type; }

    void blink();
    // find focusable widget to grab focus
    void focus_dialog();
    // return focus back to canvas
    void defocus_dialog();

    // Too many dialogs have unprotected calls to ask for this data
    SPDesktop *getDesktop() const { return desktop; }
protected:
    InkscapeApplication *getApp() const { return _app; }
    SPDocument *getDocument() const { return document; }
    Selection *getSelection() const { return selection; }

    Glib::ustring _name;             // Gtk widget name (must be set!)
    Glib::ustring const _prefs_path; // Stores characteristic path for loading/saving the dialog position.
    Glib::ustring const _dialog_type; // Type of dialog (we could just use _pref_path?).
private:
    bool blink_off(); // timer callback
    bool on_key_press_event(GdkEventKey* key_event) override;

    void unsetDesktop();
    void desktopDestroyed(SPDesktop* old_desktop);
    void setDocument(SPDocument *new_document);
    /**
     * Called when the desktop has certainly changed. It may have changed to nullptr
     * when destructing the dialog, so the override should expect nullptr too.
     */
    virtual void desktopReplaced() {}
    virtual void documentReplaced() {}
    virtual void selectionChanged(Inkscape::Selection *selection) {};
    virtual void selectionModified(Inkscape::Selection *selection, guint flags) {};

    sigc::connection _desktop_destroyed;
    sigc::connection _doc_replaced;
    sigc::connection _select_changed;
    sigc::connection _select_modified;

    InkscapeApplication *_app; // Used for state management
    SPDesktop *desktop;
    SPDocument *document;
    Selection *selection;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INK_DIALOG_BASE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
