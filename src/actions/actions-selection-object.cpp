#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-selection-object.h"
#include "actions-helper.h"
#include "inkscape-application.h"

#include "inkscape.h"             // Inkscape::Application
#include "selection.h"            // Selection

#include "object/sp-root.h"       // select_all: document->getRoot();
#include "object/sp-item-group.h" // select_all

void
select_object_group(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->group();
}

void
select_object_ungroup(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->ungroup();
}

void
select_object_ungroup_pop(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->popFromGroup();
}

void
selection_top(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->raiseToTop();
}

void
selection_raise(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->raise();
}

void
selection_lower(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->lower();
}

void
selection_bottom(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->lowerToBottom();
}

// SHOULD REALLY BE DOC ACTIONS
std::vector<std::vector<Glib::ustring>> raw_data_selection_object =
{
    /* Group */
    {"app.select-object-group",            N_("Group"),                                 "Select",   N_("Group selected objects")       },
    {"app.select-object-ungroup",          N_("Ungroup"),                               "Select",   N_("Ungroup selected objects")       },
    {"app.select-object-ungroup-pop",      N_("Pop Selected Objects out of Group"),     "Select",   N_("Pop selected objects out of group")       },
    /* Rise and Lower */
    {"app.selection-top",                  N_("Raise to Top"),                          "Select",   N_("Raise selection to top")       },
    {"app.selection-raise",                N_("Raise"),                                 "Select",   N_("Raise selection one step")       },
    {"app.selection-lower",                N_("Lower"),                                 "Select",   N_("Lower selection one step")       },
    {"app.selection-bottom",               N_("Lower to Bottom"),                       "Select",   N_("Lower selection to bottom")       }
    // clang-format on
};

void
add_actions_selection_object(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();

    // clang-format off
    /* Group */
    gapp->add_action(               "select-object-group",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_object_group),           app)        );
    gapp->add_action(               "select-object-ungroup",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_object_ungroup),         app)        );
    gapp->add_action(               "select-object-ungroup-pop",    sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_object_ungroup_pop),     app)        );
    /* Rise and Lower */
    gapp->add_action(               "selection-top",                sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_top),                 app)        );
    gapp->add_action(               "selection-raise",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_raise),               app)        );
    gapp->add_action(               "selection-lower",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_lower),               app)        );
    gapp->add_action(               "selection-bottom",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_bottom),              app)        );
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_selection_object);
}