// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Check for data loss when closing a document window.
 *
 * Copyright (C) 2004-2021 Authors
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

/* Authors:
 *   MenTaLguY
 *   Link Mauve
 *   Thomas Holder
 *   Tavmjong Bah
 */

#include "document-check.h"

#include <glibmm/i18n.h>  // Internationalization
#include <gtkmm.h>

#include "inkscape-window.h"
#include "object/sp-namedview.h"

#include "file.h"
#include "extension/system.h" // Inkscape::Extension::FILE...

/** Check if closing document associated with window will cause data loss, and if so opens a dialog
 *  that gives user options to save or ignore.
 *
 *  Returns true if document should remain open.
 */
bool
document_check_for_data_loss(InkscapeWindow* window)
{
    auto document = window->get_document();

    if (document->isModifiedSinceSave()) {
        // Document has been modified!

        Glib::ustring message = g_markup_printf_escaped(
            _("<span weight=\"bold\" size=\"larger\">Save changes to document \"%s\" before closing?</span>\n\n"
              "If you close without saving, your changes will be discarded."),
            document->getDocumentName());

        Gtk::MessageDialog dialog =
            Gtk::MessageDialog(*window, message, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
        dialog.property_destroy_with_parent() = true;

        // Don't allow text to be selected (via tabbing).
        Gtk::Container *ma = dialog.get_message_area();
        std::vector<Gtk::Widget*> ma_labels = ma->get_children();
        ma_labels[0]->set_can_focus(false);

        dialog.add_button(_("Close _without saving"), Gtk::RESPONSE_NO);
        dialog.add_button(_("_Cancel"),               Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("_Save"),                 Gtk::RESPONSE_YES);
        dialog.set_default_response(Gtk::RESPONSE_YES);

        int response = dialog.run();

        switch (response) {
            case GTK_RESPONSE_YES:
            {
                // Save document
                sp_namedview_document_from_window(window->get_desktop()); // Save window geometry in document.
                if (!sp_file_save_document(*window, document)) {
                    // Save dialog cancelled or save failed.
                    return true;
                }
                break;
            }
            case GTK_RESPONSE_NO:
                break;
            default: // cancel pressed, or dialog was closed
                return true;
                break;
        }
    }

    // Check for data loss due to saving in lossy format.
    bool allow_data_loss = false;
    while (document->getReprRoot()->attribute("inkscape:dataloss") != nullptr && allow_data_loss == false) {
        // This loop catches if the user saves to a lossy format when in the loop. 

        Glib::ustring message = g_markup_printf_escaped(
            _("<span weight=\"bold\" size=\"larger\">The file \"%s\" was saved with a format that may cause data loss!</span>\n\n"
              "Do you want to save this file as Inkscape SVG?"),
            document->getDocumentName() ? document->getDocumentName() : "Unnamed");

        Gtk::MessageDialog dialog =
            Gtk::MessageDialog(*window, message, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
        dialog.property_destroy_with_parent() = true;

        // Don't allow text to be selected (via tabbing).
        Gtk::Container *ma = dialog.get_message_area();
        std::vector<Gtk::Widget*> ma_labels = ma->get_children();
        ma_labels[0]->set_can_focus(false);

        dialog.add_button(_("Close _without saving"), Gtk::RESPONSE_NO);
        dialog.add_button(_("_Cancel"),               Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("_Save as Inkscape SVG"), Gtk::RESPONSE_YES);
        dialog.set_default_response(Gtk::RESPONSE_YES);

        int response = dialog.run();

        switch (response) {
            case GTK_RESPONSE_YES:
            {
                if (!sp_file_save_dialog(*window, document, Inkscape::Extension::FILE_SAVE_METHOD_INKSCAPE_SVG)) {
                    // Save dialog cancelled or save failed.
                    return TRUE;
                }

                break;
            }
            case GTK_RESPONSE_NO:
                allow_data_loss = true;
                break;
            default: // cancel pressed, or dialog was closed
                return true;
                break;
        }
    }

    return false;
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
