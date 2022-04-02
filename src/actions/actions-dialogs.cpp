// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for switching tools.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>
#include <map>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "config.h"  // #ifdef WITH_GSPELL

#include "actions-dialogs.h"

#include "inkscape-application.h"
#include "inkscape-window.h"

#include "ui/dialog/dialog-container.h"
#include "ui/dialog/dialog-data.h"
#include "ui/icon-names.h"

// Note the "AttrDialog" is now part of the "XMLDialog" and the "Style" dialog is part of the "Selectors" dialog.
// Also note that the "AttrDialog" does not correspond to SP_VERB_DIALOG_ATTR!!!!! (That would be the "ObjectAttributes" dialog.)

std::vector<std::vector<Glib::ustring>> raw_data_dialogs =
{
    // clang-format off
    {"win.dialog-open('AlignDistribute')",    N_("Open Align and Distribute"), "Dialog",  N_("Align and distribute objects")                                                           },
    {"win.dialog-open('CloneTiler')",         N_("Open Clone Tiler"),          "Dialog",  N_("Create multiple clones of selected object, arranging them into a pattern or scattering") },
    {"win.dialog-open('DocumentProperties')", N_("Open Document Properties"),  "Dialog",  N_("Edit properties of this document (to be saved with the document)")                       },
    {"win.dialog-open('Export')",             N_("Open Export"),               "Dialog",  N_("Export this document or a selection as a PNG image")                                     },
    {"win.dialog-open('FillStroke')",         N_("Open Fill and Stroke"),          "Dialog",  N_("Edit objects' colors, gradients, arrowheads, and other fill and stroke properties...")   },
    {"win.dialog-open('FilterEffects')",      N_("Open Filter Effects"),       "Dialog",  N_("Manage, edit, and apply SVG filters")                                                    },
    {"win.dialog-open('Find')",               N_("Open Find"),                 "Dialog",  N_("Find objects in document")                                                               },
    {"win.dialog-open('Glyphs')",             N_("Open Glyphs"),               "Dialog",  N_("Select Unicode characters from a palette")                                               },
    {"win.dialog-open('IconPreview')",        N_("Open Icon Preview"),         "Dialog",  N_("Preview Icon")                                                                           },
    {"win.dialog-open('Input')",              N_("Open Input"),                "Dialog",  N_("Configure extended input devices, such as a graphics tablet")                            },
    {"win.dialog-open('LivePathEffect')",     N_("Open Live Path Effect"),     "Dialog",  N_("Manage, edit, and apply path effects")                                                   },
    {"win.dialog-open('Memory')",             N_("Open Memory"),               "Dialog",  N_("View memory use")                                                                        },
    {"win.dialog-open('Messages')",           N_("Open Messages"),             "Dialog",  N_("View debug messages")                                                                    },
    {"win.dialog-open('ObjectAttributes')",   N_("Open Object Attributes"),    "Dialog",  N_("Edit the object attributes (context dependent)...")                                      },
    {"win.dialog-open('ObjectProperties')",   N_("Open Object Properties"),    "Dialog",  N_("Edit the ID, locked and visible status, and other object properties")                    },
    {"win.dialog-open('Objects')",            N_("Open Objects"),              "Dialog",  N_("View Objects")                                                                           },
    {"win.dialog-open('PaintServers')",       N_("Open Paint Servers"),        "Dialog",  N_("Select paint server from a collection")                                                  },
    {"win.dialog-open('Preferences')",        N_("Open Preferences"),          "Dialog",  N_("Edit global Inkscape preferences")                                                       },
    {"win.dialog-open('Selectors')",          N_("Open Selectors"),            "Dialog",  N_("View and edit CSS selectors and styles")                                                 },
    {"win.dialog-open('SVGFonts')",           N_("Open SVG Fonts"),            "Dialog",  N_("Edit SVG fonts")                                                                         },
    {"win.dialog-open('Swatches')",           N_("Open Swatches"),             "Dialog",  N_("Select colors from a swatches palette") /* TRANSLATORS: "Swatches" -> color samples */   },
    {"win.dialog-open('Symbols')",            N_("Open Symbols"),              "Dialog",  N_("Select symbol from a symbols palette")                                                   },
    {"win.dialog-open('Text')",               N_("Open Text"),                 "Dialog",  N_("View and select font family, font size and other text properties")                       },
    {"win.dialog-open('Trace')",              N_("Open Trace"),                "Dialog",  N_("Create one or more paths from a bitmap by tracing it")                                   },
    {"win.dialog-open('Transform')",          N_("Open Transform"),            "Dialog",  N_("Precisely control objects' transformations")                                             },
    {"win.dialog-open('UndoHistory')",        N_("Open Undo History"),         "Dialog",  N_("Undo History")                                                                           },
    {"win.dialog-open('XMLEditor')",          N_("Open XML Editor"),           "Dialog",  N_("View and edit the XML tree of the document")                                             },
    {"app.preferences",                       N_("Open Preferences"),          "Dialog",  N_("Edit global Inkscape preferences")                                                       },
#if WITH_GSPELL
    {"win.dialog-open('Spellcheck')",         N_("Open Spellcheck"),           "Dialog",  N_("Check spelling of text in document")                                                     },
#endif
#if DEBUG
    {"win.dialog-open('Prototype')",          N_("Open Prototype"),            "Dialog",  N_("Prototype Dialog")                                                                       },
#endif

    {"win.dialog-toggle",                     N_("Toggle all dialogs"),        "Dialog",  N_("Show or hide all dialogs")                                                               },
    // clang-format on
};

/**
 * Open dialog.
 */
void
dialog_open(const Glib::VariantBase& value, InkscapeWindow *win)
{
    Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring> >(value);
    auto dialog = s.get();

    auto dialog_it = dialog_data.find(dialog);
    if (dialog_it == dialog_data.end()) {
        std::cerr << "dialog_open: invalid dialog name: " << dialog << std::endl;
        return;
    }

    SPDesktop* dt = win->get_desktop();
    if (!dt) {
        std::cerr << "dialog_toggle: no desktop!" << std::endl;
        return;
    }

    Inkscape::UI::Dialog::DialogContainer *container = dt->getContainer();
    container->new_dialog(dialog);
}

/**
 * Toggle between showing and hiding dialogs.
 */
void
dialog_toggle(InkscapeWindow *win)
{
    SPDesktop* dt = win->get_desktop();
    if (!dt) {
        std::cerr << "dialog_toggle: no desktop!" << std::endl;
        return;
    }

    // Keep track of state?
    // auto action = win->lookup_action("dialog-toggle");
    // if (!action) {
    //     std::cerr << "dialog_toggle: action 'dialog-toggle' missing!" << std::endl;
    //     return;
    // }

    // auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    // if (!saction) {
    //     std::cerr << "dialog_toogle: action 'dialog_switch' not SimpleAction!" << std::endl;
    //     return;
    // }

    // saction->get_state();

    Inkscape::UI::Dialog::DialogContainer *container = dt->getContainer();
    container->toggle_dialogs();
}

void
add_actions_dialogs(InkscapeWindow* win)
{
    Glib::VariantType String(Glib::VARIANT_TYPE_STRING);

    // clang-format off
    win->add_action_with_parameter( "dialog-open",  String, sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&dialog_open),   win));
    win->add_action(                "dialog-toggle",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&dialog_toggle), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_dialog: no app!" << std::endl;
        return;
    }

    // macOS automatically uses app.preferences in the application menu
    auto *gapp = app->gio_app();
    gapp->add_action("preferences", [=]() { dialog_open(Glib::Variant<Glib::ustring>::create("Preferences"), win); });

    app->get_action_extra_data().add_data(raw_data_dialogs);
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
