// SPDX-License-Identifier: GPL-2.0-or-later

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

#include "dialog-base.h"

#include <glibmm/i18n.h>
#include <glibmm/main.h>
#include <glibmm/refptr.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/notebook.h>
#include <iostream>

#include "desktop.h"
#include "ui/dialog/dialog-data.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog-events.h"
// get_latin_keyval
#include "ui/tools/tool-base.h"
#include "widgets/spw-utilities.h"
#include "ui/widget/canvas.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * DialogBase constructor.
 *
 * @param prefs_path characteristic path to load/save dialog position.
 * @param dialog_type is the "type" string for the dialog.
 */
DialogBase::DialogBase(gchar const *prefs_path, Glib::ustring dialog_type)
    : Gtk::Box(Gtk::ORIENTATION_VERTICAL)
    , desktop(nullptr)
    , document(nullptr)
    , selection(nullptr)
    , _name("DialogBase")
    , _prefs_path(prefs_path)
    , _dialog_type(dialog_type)
    , _app(InkscapeApplication::instance())
{
    // Derive a pretty display name for the dialog based on the verbs name.
    // TODO: This seems fragile. Should verbs have a proper display name?
    auto it = dialog_data.find(dialog_type);
    if (it != dialog_data.end()) {

        // get translated verb name
        _name = _(it->second.label.c_str());

        // remove ellipsis and mnemonics
        int pos = _name.find("...", 0);
        if (pos >= 0 && pos < _name.length() - 2) {
            _name.erase(pos, 3);
        }
        pos = _name.find("…", 0);
        if (pos >= 0 && pos < _name.length()) {
            _name.erase(pos, 1);
        }
        pos = _name.find("_", 0);
        if (pos >= 0 && pos < _name.length()) {
            _name.erase(pos, 1);
        }
    }

    set_name(_name); // Essential for dialog functionality
    property_margin().set_value(1); // Essential for dialog UI
    ensure_size();
}

bool DialogBase::on_key_press_event(GdkEventKey* key_event) {
    switch (Inkscape::UI::Tools::get_latin_keyval(key_event)) {
        case GDK_KEY_Escape:
            defocus_dialog();
            return true;
    }

    return parent_type::on_key_press_event(key_event);
}


/**
 * Highlight notebook where dialog already exists.
 */
void DialogBase::blink()
{
    Gtk::Notebook *notebook = dynamic_cast<Gtk::Notebook *>(get_parent());
    if (notebook && notebook->get_is_drawable()) {
        // Switch notebook to this dialog.
        notebook->set_current_page(notebook->page_num(*this));
        notebook->get_style_context()->add_class("blink");

        // Add timer to turn off blink.
        sigc::slot<bool> slot = sigc::mem_fun(*this, &DialogBase::blink_off);
        sigc::connection connection = Glib::signal_timeout().connect(slot, 1000); // msec
    }
}

void DialogBase::focus_dialog() {
    if (auto window = dynamic_cast<Gtk::Window*>(get_toplevel())) {
        window->present();
    }

    // widget that had focus, if any
    if (auto child = get_focus_child()) {
        child->grab_focus();
    }
    else {
        // find first focusable widget
        if (auto child = sp_find_focusable_widget(this)) {
            child->grab_focus();
        }
    }
}

void DialogBase::defocus_dialog() {
    if (auto wnd = dynamic_cast<Gtk::Window*>(get_toplevel())) {
        // defocus floating dialog:
        sp_dialog_defocus_cpp(wnd);

        // for docked dialogs, move focus to canvas
        if (auto desktop = getDesktop()) {
            desktop->getCanvas()->grab_focus();
        }
    }
}

/**
 * Callback to reset the dialog highlight.
 */
bool DialogBase::blink_off()
{
    Gtk::Notebook *notebook = dynamic_cast<Gtk::Notebook *>(get_parent());
    if (notebook && notebook->get_is_drawable()) {
        notebook->get_style_context()->remove_class("blink");
    }
    return false;
}

/**
 * Called when the desktop might have changed for this dialog.
 */
void DialogBase::setDesktop(SPDesktop *new_desktop)
{
    if (desktop != new_desktop) {
        unsetDesktop();
        desktop = new_desktop;

        if (desktop) {
            _doc_replaced = desktop->connectDocumentReplaced(sigc::hide<0>(sigc::mem_fun(*this, &DialogBase::setDocument)));
            _desktop_destroyed = desktop->connectDestroy( sigc::mem_fun(*this, &DialogBase::desktopDestroyed));
            if (desktop->selection) {
                selection = desktop->selection;
                _select_changed = selection->connectChanged(sigc::mem_fun(*this, &DialogBase::selectionChanged));
                _select_modified = selection->connectModified(sigc::mem_fun(*this, &DialogBase::selectionModified));
            }
        }
        if (desktop) {
            this->setDocument(desktop->getDocument());
        } else {
            this->setDocument(nullptr);
        }
        desktopReplaced();
    }
}

/**
 * Called to destruct desktops, must not call virtuals
 */
void DialogBase::unsetDesktop()
{
    desktop = nullptr;
    document = nullptr;
    selection = nullptr;
    _desktop_destroyed.disconnect();
    _doc_replaced.disconnect();
    _select_changed.disconnect();
    _select_modified.disconnect();
}

void DialogBase::desktopDestroyed(SPDesktop* old_desktop)
{
    if (old_desktop == desktop && desktop) {
        unsetDesktop();
    }
}

/**
 * Called when the document might have changed, called from setDesktop too.
 */
void DialogBase::setDocument(SPDocument *new_document)
{
    if (document != new_document) {
        document = new_document;
        documentReplaced();
    }
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
