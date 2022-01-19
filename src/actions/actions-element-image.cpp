// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for use with <image>.
 *
 * Copyright (C) 2022 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include "actions-element-image.h"

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <gtkmm.h>  // OK, we lied. We pop-up an message dialog if external editor not found and if we have a GUI.
#include <glibmm/i18n.h>

#include "inkscape-application.h"
#include "inkscape-window.h"
#include "preferences.h"

#include "selection.h"            // Selection
#include "object/sp-image.h"

Glib::ustring image_get_editor_name(bool is_svg)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Glib::ustring editor;
    if (is_svg) {
        editor = prefs->getString("/options/svgeditor/value", "inkscape");
    } else {
        editor = prefs->getString("/options/bitmapeditor/value", "gimp");
    }
    return editor;
}

// Note that edits are external to Inkscape and thus we cannot undo them!
void image_edit(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    if (selection->isEmpty()) {
        // Nothing to do.
        return;
    }

    auto document = selection->document();

    for (auto item : selection->items()) {
        auto image = dynamic_cast<SPImage *>(item);
        if (image) {

            Inkscape::XML::Node *node = item->getRepr();
            const gchar *href = node->attribute("xlink:href");
            if (!href) {
                std::cerr << "image_edit: no xlink:href" << std::endl;
                continue;
            }

            if (strncmp (href, "data", 4) == 0) {
                std::cerr << "image_edit: cannot edit embedded image" << std::endl;
                continue;
            }

            // Find filename.
            std::string filename = href;
            if (strncmp (href, "file", 4) == 0) {
                filename = Glib::filename_from_uri(href);
            }

            if (Glib::path_is_absolute(filename)) {
                // Do nothing
            } else if (document->getDocumentBase()) {
                filename = Glib::build_filename(document->getDocumentBase(), filename);
            } else {
                filename = Glib::build_filename(Glib::get_current_dir(), filename);
            }

            // Bitmap or SVG?
            bool is_svg = false;
            if (filename.substr(filename.find_last_of(".") + 1) == "SVG" ||
                filename.substr(filename.find_last_of(".") + 1) == "svg") {
                is_svg = true;
            }

            // Get editor.
            auto editor = image_get_editor_name(is_svg);

#ifdef _WIN32
            // Parsing is done according to Unix shell rules, need to enclose editor path by single
            // quotes (everything before file extension).
            int            index = editor.find(".exe");
            if (index < 0) index = editor.find(".bat");
            if (index < 0) index = editor.find(".com");
            if (index < 0) index = editor.length();

            editor.insert(index, "'");
            editor.insert(0, "'");
#endif
            Glib::ustring command = editor + " '" + filename + "'";

            GError* error = nullptr;
            g_spawn_command_line_async(command.c_str(), &error);
            if (error) {
                Glib::ustring message = _("Failed to edit external image.\n<small>Note: Path to editor can be set in Preferences dialog.</small>");
                Glib::ustring message2 = Glib::ustring::compose(_("System error message: %1"), error->message);
                auto window = app->get_active_window();
                if (window) {
                    Gtk::MessageDialog dialog(*window, message, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
                    dialog.property_destroy_with_parent() = true;
                    dialog.set_name("SetEditorDialog");
                    dialog.set_title(_("External Edit Image:"));
                    dialog.set_secondary_text(message2);
                    dialog.run();
                } else {
                    std::cerr << "image_edit: " << message << std::endl;
                }
                g_error_free(error);
                error = nullptr;
            }
        }
    }
}

std::vector<std::vector<Glib::ustring>> raw_data_element_image =
{
    // clang-format off
    {"app.element-image-edit",          N_("Edit externally"),   "Image",    N_("Edit image externally (image must be selected and not embeded).")    },
    // clang-format on
};

void
add_actions_element_image(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action(                "element-image-edit",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&image_edit),      app));
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_element_image);
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
