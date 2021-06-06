#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-edit.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"

void 
object_to_pattern(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->tile();
}

void 
pattern_to_object(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->untile();
}

void 
object_to_marker(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->toMarker();
}

void 
object_to_guides(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->toGuides();
}

std::vector<std::vector<Glib::ustring>> raw_data_edit =
{
    // clang-format off
    {"win.object-to-pattern",          N_("Objects to Pattern"),           "Edit",  N_("Convert selection to a rectangle with tiled pattern fill")                                    },
    {"win.pattern-to-object",          N_("Pattern to Objects"),           "Edit",  N_("Extract objects from a tiled pattern fill")                                    },
    {"win.object-to-marker",           N_("Objects to Marker"),            "Edit",  N_("Convert selection to a line marker")                                    },
    {"win.object-to-guides",           N_("Objects to Guides"),            "Edit",  N_("Convert selected objects to a collection of guidelines aligned with their edges")                                    }
    // clang-format on
};

void
add_actions_edit(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "object-to-pattern",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&object_to_pattern), win));
    win->add_action( "pattern-to-object",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&pattern_to_object), win));
    win->add_action( "object-to-marker",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&object_to_marker), win));
    win->add_action( "object-to-guides",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&object_to_guides), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_edit);
}