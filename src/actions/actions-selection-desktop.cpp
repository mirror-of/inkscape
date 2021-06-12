#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-selection-desktop.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "document-undo.h"

void 
selection_make_bitmap_copy(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->createBitmapCopy();
}

std::vector<std::vector<Glib::ustring>> raw_selection_dekstop_data =
{
    // clang-format off
    {"win.selection-make-bitmap-copy",                       N_("Make a _Bitmap Copy"),     "Selection Desktop",  N_("Export selection to a bitmap and insert it into document")                                    }
    // clang-format on
};

void
add_actions_select_desktop(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "selection-make-bitmap-copy",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&selection_make_bitmap_copy), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_selection_dekstop_data);
}