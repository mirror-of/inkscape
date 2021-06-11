#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-edit.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "selection-chemistry.h"

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

void 
undo(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_undo(dt, dt->getDocument());
}

void 
redo(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_redo(dt, dt->getDocument());
}

void 
cut(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->cut();
}

void 
copy(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->copy();
}

void 
paste(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_selection_paste(dt, false);
}

void 
paste_in_place(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_selection_paste(dt, true);
}

void 
paste_style(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteStyle();
}

void 
paste_size(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSize(true,true);
}

void 
paste_width(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSize(true, false);
}

void 
paste_height(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSize(false, true);
}

void 
paste_size_separately(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSizeSeparately(true, true);
}

void 
paste_width_separately(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSizeSeparately(true, false);
}

void 
paste_height_separately(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSizeSeparately(false, true);
}

void 
duplicate(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->duplicate();
}

std::vector<std::vector<Glib::ustring>> raw_data_edit =
{
    // clang-format off
    {"win.object-to-pattern",          N_("Objects to Pattern"),           "Edit",  N_("Convert selection to a rectangle with tiled pattern fill")                                    },
    {"win.pattern-to-object",          N_("Pattern to Objects"),           "Edit",  N_("Extract objects from a tiled pattern fill")                                    },
    {"win.object-to-marker",           N_("Objects to Marker"),            "Edit",  N_("Convert selection to a line marker")                                    },
    {"win.object-to-guides",           N_("Objects to Guides"),            "Edit",  N_("Convert selected objects to a collection of guidelines aligned with their edges")                                    },
    {"win.undo",                       N_("_Undo"),                        "Edit",  N_("Undo last action")                                    },
    {"win.redo",                       N_("_Redo"),                        "Edit",  N_("Do again the last undone action")                                    },
    {"win.cut",                       N_("Cu_t"),                        "Edit",  N_("Cut selection to clipboard")                                    },
    {"win.copy",                       N_("_Copy"),                        "Edit",  N_("Copy selection to clipboard")                                    },
    {"win.paste",                       N_("_Paste"),                        "Edit",  N_("Paste objects from clipboard to mouse point, or paste text")                                    },
    {"win.paste-in-place",                       N_("Paste _In Place"),                        "Edit",  N_("Paste objects from clipboard to mouse point, or paste text")                                    },
    {"win.paste-style",                       N_("Paste _Style"),                        "Edit",  N_("Apply the style of the copied object to selection")                                    },
    {"win.paste-size",                       N_("Paste Si_ze"),                        "Edit",  N_("Scale selection to match the size of the copied object")                                    },
    {"win.paste-width",                       N_("Paste _Width"),                        "Edit",  N_("Scale selection horizontally to match the width of the copied object")                                    },
    {"win.paste-height",                       N_("Paste _Height"),                        "Edit",  N_("Scale selection vertically to match the height of the copied object")                                    },
    {"win.paste-size-separately",                       N_("Paste Size Separately"),                        "Edit",  N_("Scale each selected object to match the size of the copied object")                                    },
    {"win.paste-width-separately",                       N_("Paste Width Separately"),                        "Edit",  N_("Scale each selected object horizontally to match the width of the copied object")                                    },
    {"win.paste-height-separately",                       N_("Paste Height Separately"),                        "Edit",  N_("Scale each selected object vertically to match the height of the copied object")                                    },
    {"win.duplicate",                       N_("Duplic_ate"),                        "Edit",  N_("Duplicate Selected Objects")                                    }
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
    win->add_action( "undo",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&undo), win));
    win->add_action( "redo",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&redo), win));
    win->add_action( "cut",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&cut), win));
    win->add_action( "copy",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&copy), win));
    win->add_action( "paste",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste), win));
    win->add_action( "paste-in-place",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_in_place), win));
    win->add_action( "paste-style",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_style), win));
    win->add_action( "paste-size",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_size), win));
    win->add_action( "paste-width",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_width), win));
    win->add_action( "paste-height",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_height), win));
    win->add_action( "paste-size-separately",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_size_separately), win));
    win->add_action( "paste-width-separately",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_width_separately), win));
    win->add_action( "paste-height-separately",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_height_separately), win));
    win->add_action( "duplicate",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&duplicate), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_edit);
}