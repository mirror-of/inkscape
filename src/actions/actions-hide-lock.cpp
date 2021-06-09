#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-hide-lock.h"
#include "inkscape-application.h"
#include "document-undo.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "selection-chemistry.h"

void 
hide_lock_unhide_all(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    SPDocument *doc = dt->getDocument();
    unhide_all(dt);
    Inkscape::DocumentUndo::done(doc, _("Unhide all objects in the current layer"), nullptr);
}

void
hide_lock_unlock_all(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    SPDocument *doc = dt->getDocument();
    unlock_all(dt);
    Inkscape::DocumentUndo::done(doc, _("Unlock all objects in the current layer"), nullptr);
}

std::vector<std::vector<Glib::ustring>> raw_data_hide_lock =
{
    // clang-format off
    {"win.unhide-all",            N_("Unhide All"),             "Hide and Lock",  N_("Unhide all objects in the current layer")                                    },
    {"win.unlock-all",            N_("Unlock All"),             "Hide and Lock",  N_("Unlock all objects in the current layer")                                    }
    // clang-format on
};

void
add_actions_hide_lock(InkscapeWindow* win)
{

    // clang-format off
    win->add_action( "unhide-all",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&hide_lock_unhide_all), win));
    win->add_action( "unlock-all",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&hide_lock_unlock_all), win));
    // clang-format on


    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_hide_lock: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_hide_lock);
}