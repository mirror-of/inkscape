// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * @file
 * Inkscape toolbar definitions and general utility functions.
 * Each tool should have its own xxx-toolbar implementation file
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *   Jabiertxo Arraiza <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2015 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm/box.h>
#include <gtkmm/action.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/toolitem.h>
#include <glibmm/i18n.h>

#include "desktop-style.h"
#include "desktop.h"
#include "document-undo.h"
#include "inkscape.h"
#include "verbs.h"

#include "ink-action.h"

#include "helper/action.h"
#include "helper/verb-action.h"

#include "include/gtkmm_version.h"

#include "io/resource.h"

#include "object/sp-namedview.h"

#include "ui/icon-names.h"
#include "ui/tools-switch.h"
#include "ui/uxmanager.h"
#include "ui/widget/button.h"
#include "ui/widget/spinbutton.h"
#include "ui/widget/unit-tracker.h"

#include "widgets/spw-utilities.h"
#include "widgets/widget-sizes.h"

#include "xml/attribute-record.h"
#include "xml/node-event-vector.h"

#include "ui/toolbar/aux-toolbox.h"
#include "ui/toolbar/snap-toolbar.h"

#include "toolbox.h"

#include "ui/tools/tool-base.h"

//#define DEBUG_TEXT

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::Tools::ToolBase;

using Inkscape::IO::Resource::get_filename;
using Inkscape::IO::Resource::UIS;

typedef void (*SetupFunction)(Gtk::Bin *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, ToolBase *eventcontext, Gtk::Bin *toolbox);

enum BarId {
    BAR_TOOL = 0,
    BAR_COMMANDS,
    BAR_SNAP,
};

#define BAR_ID_KEY "BarIdValue"
#define HANDLE_POS_MARK "x-inkscape-pos"

GtkIconSize ToolboxFactory::prefToSize( Glib::ustring const &path, int base ) {
    static GtkIconSize sizeChoices[] = {
        GTK_ICON_SIZE_LARGE_TOOLBAR,
        GTK_ICON_SIZE_SMALL_TOOLBAR,
        GTK_ICON_SIZE_DND,
        GTK_ICON_SIZE_DIALOG
    };
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int index = prefs->getIntLimited( path, base, 0, G_N_ELEMENTS(sizeChoices) );
    return sizeChoices[index];
}

Gtk::IconSize ToolboxFactory::prefToSize_mm(Glib::ustring const &path, int base)
{
    static Gtk::IconSize sizeChoices[] = { Gtk::ICON_SIZE_LARGE_TOOLBAR, Gtk::ICON_SIZE_SMALL_TOOLBAR,
                                           Gtk::ICON_SIZE_DND, Gtk::ICON_SIZE_DIALOG };
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int index = prefs->getIntLimited(path, base, 0, G_N_ELEMENTS(sizeChoices));
    return sizeChoices[index];
}

static struct {
    gchar const *type_name;
    gchar const *data_name;
    sp_verb_t verb;
    sp_verb_t doubleclick_verb;
} const tools[] = {
	{ "/tools/select",   "select_tool",    SP_VERB_CONTEXT_SELECT,  SP_VERB_CONTEXT_SELECT_PREFS},
	{ "/tools/nodes",     "node_tool",      SP_VERB_CONTEXT_NODE, SP_VERB_CONTEXT_NODE_PREFS },
	{ "/tools/tweak",    "tweak_tool",     SP_VERB_CONTEXT_TWEAK, SP_VERB_CONTEXT_TWEAK_PREFS },
	{ "/tools/spray",    "spray_tool",     SP_VERB_CONTEXT_SPRAY, SP_VERB_CONTEXT_SPRAY_PREFS },
	{ "/tools/zoom",     "zoom_tool",      SP_VERB_CONTEXT_ZOOM, SP_VERB_CONTEXT_ZOOM_PREFS },
	{ "/tools/measure",  "measure_tool",   SP_VERB_CONTEXT_MEASURE, SP_VERB_CONTEXT_MEASURE_PREFS },
	{ "/tools/shapes/rect",     "rect_tool",      SP_VERB_CONTEXT_RECT, SP_VERB_CONTEXT_RECT_PREFS },
	{ "/tools/shapes/3dbox",      "3dbox_tool",     SP_VERB_CONTEXT_3DBOX, SP_VERB_CONTEXT_3DBOX_PREFS },
	{ "/tools/shapes/arc",      "arc_tool",       SP_VERB_CONTEXT_ARC, SP_VERB_CONTEXT_ARC_PREFS },
	{ "/tools/shapes/star",     "star_tool",      SP_VERB_CONTEXT_STAR, SP_VERB_CONTEXT_STAR_PREFS },
	{ "/tools/shapes/spiral",   "spiral_tool",    SP_VERB_CONTEXT_SPIRAL, SP_VERB_CONTEXT_SPIRAL_PREFS },
	{ "/tools/freehand/pencil",   "pencil_tool",    SP_VERB_CONTEXT_PENCIL, SP_VERB_CONTEXT_PENCIL_PREFS },
	{ "/tools/freehand/pen",      "pen_tool",       SP_VERB_CONTEXT_PEN, SP_VERB_CONTEXT_PEN_PREFS },
	{ "/tools/calligraphic", "dyna_draw_tool", SP_VERB_CONTEXT_CALLIGRAPHIC, SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS },
	{ "/tools/lpetool",  "lpetool_tool",   SP_VERB_CONTEXT_LPETOOL, SP_VERB_CONTEXT_LPETOOL_PREFS },
	{ "/tools/eraser",   "eraser_tool",    SP_VERB_CONTEXT_ERASER, SP_VERB_CONTEXT_ERASER_PREFS },
	{ "/tools/paintbucket",    "paintbucket_tool",     SP_VERB_CONTEXT_PAINTBUCKET, SP_VERB_CONTEXT_PAINTBUCKET_PREFS },
	{ "/tools/text",     "text_tool",      SP_VERB_CONTEXT_TEXT, SP_VERB_CONTEXT_TEXT_PREFS },
	{ "/tools/connector","connector_tool", SP_VERB_CONTEXT_CONNECTOR, SP_VERB_CONTEXT_CONNECTOR_PREFS },
	{ "/tools/gradient", "gradient_tool",  SP_VERB_CONTEXT_GRADIENT, SP_VERB_CONTEXT_GRADIENT_PREFS },
	{ "/tools/mesh",     "mesh_tool",      SP_VERB_CONTEXT_MESH, SP_VERB_CONTEXT_MESH_PREFS },
	{ "/tools/dropper",  "dropper_tool",   SP_VERB_CONTEXT_DROPPER, SP_VERB_CONTEXT_DROPPER_PREFS },
	{ nullptr, nullptr, 0, 0 }
};

static Glib::RefPtr<Gtk::ActionGroup> create_or_fetch_actions( SPDesktop* desktop );

static void setup_snap_toolbox(Gtk::Bin *toolbox, SPDesktop *desktop);

static void setup_tool_toolbox(Gtk::Bin *toolbox, SPDesktop *desktop);
static void update_tool_toolbox(SPDesktop *desktop, ToolBase *eventcontext, Gtk::Bin *toolbox);

static void setup_commands_toolbox(Gtk::Bin *toolbox, SPDesktop *desktop);
static void update_commands_toolbox(SPDesktop *desktop, ToolBase *eventcontext, Gtk::Bin *toolbox);

static void trigger_sp_action( GtkAction* /*act*/, gpointer user_data )
{
    SPAction* targetAction = SP_ACTION(user_data);
    if ( targetAction ) {
        sp_action_perform( targetAction, nullptr );
    }
}

static GtkAction* create_action_for_verb( Inkscape::Verb* verb, Inkscape::UI::View::View* view, GtkIconSize size )
{
    GtkAction* act = nullptr;

    SPAction* targetAction = verb->get_action(Inkscape::ActionContext(view));
    InkAction* inky = ink_action_new( verb->get_id(), _(verb->get_name()), verb->get_tip(), verb->get_image(), size  );
    act = GTK_ACTION(inky);
    gtk_action_set_sensitive( act, targetAction->sensitive );

    g_signal_connect( G_OBJECT(inky), "activate", G_CALLBACK(trigger_sp_action), targetAction );

    // FIXME: memory leak: this is not unrefed anywhere
    g_object_ref(G_OBJECT(targetAction));
    g_object_set_data_full(G_OBJECT(inky), "SPAction", (void*) targetAction, (GDestroyNotify) &g_object_unref);
    targetAction->signal_set_sensitive.connect(
        sigc::bind<0>(
            sigc::ptr_fun(&gtk_action_set_sensitive),
            GTK_ACTION(inky)));

    return act;
}

static std::map<SPDesktop*, Glib::RefPtr<Gtk::ActionGroup> > groups;

static void desktopDestructHandler(SPDesktop *desktop)
{
    auto it = groups.find(desktop);
    if (it != groups.end())
    {
        groups.erase(it);
    }
}

static Glib::RefPtr<Gtk::ActionGroup> create_or_fetch_actions( SPDesktop* desktop )
{
    Inkscape::UI::View::View *view = desktop;
    gint verbsToUse[] = {
        // disabled until we have icons for them:
        //find
        //SP_VERB_EDIT_TILE,
        //SP_VERB_EDIT_UNTILE,
        SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
        SP_VERB_DIALOG_DISPLAY,
        SP_VERB_DIALOG_FILL_STROKE,
        SP_VERB_DIALOG_NAMEDVIEW,
        SP_VERB_DIALOG_TEXT,
        SP_VERB_DIALOG_XML_EDITOR,
        SP_VERB_DIALOG_SELECTORS,
        SP_VERB_DIALOG_LAYERS,
        SP_VERB_EDIT_CLONE,
        SP_VERB_EDIT_COPY,
        SP_VERB_EDIT_CUT,
        SP_VERB_EDIT_DUPLICATE,
        SP_VERB_EDIT_PASTE,
        SP_VERB_EDIT_REDO,
        SP_VERB_EDIT_UNDO,
        SP_VERB_EDIT_UNLINK_CLONE,
        //SP_VERB_FILE_EXPORT,
        SP_VERB_DIALOG_EXPORT,
        SP_VERB_FILE_IMPORT,
        SP_VERB_FILE_NEW,
        SP_VERB_FILE_OPEN,
        SP_VERB_FILE_PRINT,
        SP_VERB_FILE_SAVE,
        SP_VERB_OBJECT_TO_CURVE,
        SP_VERB_SELECTION_GROUP,
        SP_VERB_SELECTION_OUTLINE,
        SP_VERB_SELECTION_UNGROUP,
        SP_VERB_ZOOM_1_1,
        SP_VERB_ZOOM_1_2,
        SP_VERB_ZOOM_2_1,
        SP_VERB_ZOOM_DRAWING,
        SP_VERB_ZOOM_IN,
        SP_VERB_ZOOM_NEXT,
        SP_VERB_ZOOM_OUT,
        SP_VERB_ZOOM_PAGE,
        SP_VERB_ZOOM_PAGE_WIDTH,
        SP_VERB_ZOOM_PREV,
        SP_VERB_ZOOM_SELECTION,
        SP_VERB_ZOOM_CENTER_PAGE
    };

    GtkIconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/small");
    Glib::RefPtr<Gtk::ActionGroup> mainActions;
    if (desktop == nullptr)
    {
        return mainActions;
    }

    if ( groups.find(desktop) != groups.end() ) {
        mainActions = groups[desktop];
    }

    if ( !mainActions ) {
        mainActions = Gtk::ActionGroup::create("main");
        groups[desktop] = mainActions;
        desktop->connectDestroy(&desktopDestructHandler);
    }

    for (int i : verbsToUse) {
        Inkscape::Verb* verb = Inkscape::Verb::get(i);
        if ( verb ) {
            if (!mainActions->get_action(verb->get_id())) {
                GtkAction* act = create_action_for_verb( verb, view, toolboxSize );
                mainActions->add(Glib::wrap(act));
            }
        }
    }

    if ( !mainActions->get_action("ToolZoom") ) {
        for ( guint i = 0; i < G_N_ELEMENTS(tools) && tools[i].type_name; i++ ) {
            Glib::RefPtr<VerbAction> va = VerbAction::create(Inkscape::Verb::get(tools[i].verb), Inkscape::Verb::get(tools[i].doubleclick_verb), view);
            if ( va ) {
                mainActions->add(va);
                if ( i == 0 ) {
                    va->set_active(true);
                }
            } else {
                // This creates a blank action using the data_name, this can replace
                // tools that have been disabled by compile time options.
                Glib::RefPtr<Gtk::Action> act = Gtk::Action::create(Glib::ustring(tools[i].data_name));
                act->set_sensitive(false);
                mainActions->add(act);
            }
        }
    }

    return mainActions;
}


static Gtk::EventBox * toolboxNewCommon(Gtk::Box * tb, BarId id, Gtk::PositionType /*handlePos*/)
{
    tb->set_data("desktop", nullptr);

    tb->set_sensitive(false);

    auto hb = Gtk::make_managed<Gtk::EventBox>(); // A simple, neutral container.
    hb->set_name("ToolboxCommon");

    hb->add(*tb);
    tb->show();

    sigc::connection* conn = new sigc::connection;
    hb->set_data("event_context_connection", conn);

    gpointer val = GINT_TO_POINTER(id);
    hb->set_data(BAR_ID_KEY, val);

    return hb;
}

Gtk::EventBox * ToolboxFactory::createToolToolbox()
{
    auto tb = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 0);
    tb->set_name("ToolToolbox");
    tb->set_homogeneous(false);

    return toolboxNewCommon(tb, BAR_TOOL, Gtk::POS_TOP);
}

//####################################
//# Commands Bar
//####################################

Gtk::EventBox * ToolboxFactory::createCommandsToolbox()
{
    auto tb = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 0);
    tb->set_name("CommandsToolbox");
    tb->set_homogeneous(false);

    return toolboxNewCommon(tb, BAR_COMMANDS, Gtk::POS_LEFT);
}

Gtk::EventBox * ToolboxFactory::createSnapToolbox()
{
    auto tb = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 0);
    tb->set_name("SnapToolbox");
    tb->set_homogeneous(false);

    return toolboxNewCommon(tb, BAR_SNAP, Gtk::POS_LEFT);
}

void ToolboxFactory::setToolboxDesktop(Gtk::EventBox *toolbox, SPDesktop *desktop)
{
    auto aux_toolbox = dynamic_cast<Toolbar::AuxToolbox *>(toolbox);

    if (aux_toolbox) {
        aux_toolbox->set_desktop(desktop);
    }
    else {
        auto conn = static_cast<sigc::connection*>(toolbox->get_data("event_context_connection"));

        BarId id = static_cast<BarId>(GPOINTER_TO_INT(toolbox->get_data(BAR_ID_KEY)));

        SetupFunction setup_func = nullptr;
        UpdateFunction update_func = nullptr;

        switch (id) {
            case BAR_TOOL:
                setup_func = setup_tool_toolbox;
                update_func = update_tool_toolbox;
                break;

            case BAR_COMMANDS:
                setup_func = setup_commands_toolbox;
                update_func = update_commands_toolbox;
                break;

            case BAR_SNAP:
                setup_func = setup_snap_toolbox;
                update_func = updateSnapToolbox;
                break;

            default:
                g_warning("Unexpected toolbox id encountered.");
        }

        SPDesktop *old_desktop = static_cast<SPDesktop*>(toolbox->get_data("desktop"));

        if (old_desktop) {
            auto children = toolbox->get_children();
            for ( auto i:children ) {
                gtk_container_remove(GTK_CONTAINER(toolbox->gobj()), i->gobj());
            }
        }

        toolbox->set_data("desktop", (gpointer)desktop);

        if (desktop && setup_func && update_func) {
            toolbox->set_sensitive(true);
            setup_func(toolbox, desktop);
            update_func(desktop, desktop->event_context, toolbox);
            *conn = desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(update_func), toolbox));
        } else {
            toolbox->set_sensitive(false);
        }
    }
} // end of sp_toolbox_set_desktop()


static void setupToolboxCommon( Gtk::Bin  *toolbox,
                                SPDesktop *desktop,
                                gchar const *ui_file,
                                gchar const* toolbarName,
                                gchar const* sizePref )
{
    auto mainActions = create_or_fetch_actions(desktop);
    auto prefs = Inkscape::Preferences::get();
    auto mgr = Gtk::UIManager::create();
    auto orientation = Gtk::ORIENTATION_HORIZONTAL;

    mgr->insert_action_group(mainActions, 0);

    auto filename = get_filename(UIS, ui_file);
    try {
        mgr->add_ui_from_file(filename);
    } catch (Glib::Error &err) {
        g_warning("Failed to load %s: %s", filename.c_str(), err.what().c_str());
        return;
    }

    auto toolBar = dynamic_cast<Gtk::Toolbar *>(mgr->get_widget(toolbarName));
    if ( prefs->getBool("/toolbox/icononly", true) ) {
        toolBar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
    }

    auto toolboxSize = ToolboxFactory::prefToSize(sizePref);
    toolBar->set_icon_size(static_cast<Gtk::IconSize>(toolboxSize));

    auto pos = static_cast<Gtk::PositionType>(GPOINTER_TO_INT(toolbox->get_data(HANDLE_POS_MARK)));
    orientation = ((pos == Gtk::POS_LEFT) || (pos == Gtk::POS_RIGHT)) ? Gtk::ORIENTATION_HORIZONTAL : Gtk::ORIENTATION_VERTICAL;
    gtk_orientable_set_orientation(GTK_ORIENTABLE(toolBar->gobj()), static_cast<GtkOrientation>(orientation));
    toolBar->set_show_arrow(true);

    toolBar->set_data("desktop", nullptr);

    auto child = toolbox->get_child();
    if (child) {
        gtk_container_remove(GTK_CONTAINER(toolbox->gobj()), child->gobj());
    }

    toolbox->add(*toolBar);
}

#define noDUMP_DETAILS 1

void ToolboxFactory::setOrientation(Gtk::EventBox *toolbox, Gtk::Orientation orientation)
{
    auto pos = (orientation == Gtk::ORIENTATION_HORIZONTAL) ? Gtk::POS_LEFT : Gtk::POS_TOP;

    auto child = toolbox->get_child();
    if (child) {
        auto child_box = dynamic_cast<Gtk::Box *>(child);
        auto child_toolbar = dynamic_cast<Gtk::Toolbar *>(child);

        if (child_box) {
            auto children = child_box->get_children();
            if (!children.empty()) {
                for (auto child2:children) {
                    auto child2_container = dynamic_cast<Gtk::Container *>(child2);

                    if (child2_container) {
                        auto children2 = child2_container->get_children();
                        if (!children2.empty()) {
                            for (auto child3:children2) {
                                auto child3_toolbar = dynamic_cast<Gtk::Toolbar *>(child3);
                                if (child3_toolbar) {
                                    gtk_orientable_set_orientation(GTK_ORIENTABLE(child3_toolbar->gobj()),
                                                                   static_cast<GtkOrientation>(orientation));
                                }
                            }
                        }
                    }

                    auto child2_toolbar = dynamic_cast<Gtk::Toolbar *>(child2);

                    if (child2_toolbar) {
                        gtk_orientable_set_orientation(GTK_ORIENTABLE(child2_toolbar->gobj()),
                                                       static_cast<GtkOrientation>(orientation));
                    } else {
                        g_message("need to add dynamic switch");
                    }
                }
            } else {
                // The call is being made before the toolbox proper has been setup.
                toolbox->set_data(HANDLE_POS_MARK, GINT_TO_POINTER(pos));
            }
        } else if (child_toolbar) {
            gtk_orientable_set_orientation(GTK_ORIENTABLE(child_toolbar->gobj()),
                                           static_cast<GtkOrientation>(orientation));
        }
    }
}

void setup_tool_toolbox(Gtk::Bin *toolbox, SPDesktop *desktop)
{
    setupToolboxCommon(toolbox, desktop, "toolbar-tool.ui", "/ui/ToolToolbar", "/toolbox/tools/small");
}

void update_tool_toolbox( SPDesktop *desktop, ToolBase *eventcontext, Gtk::Bin * /*toolbox*/ )
{
    gchar const *const tname = ( eventcontext
                                 ? eventcontext->getPrefsPath().c_str() //g_type_name(G_OBJECT_TYPE(eventcontext))
                                 : nullptr );
    auto mainActions = create_or_fetch_actions( desktop );

    for (int i = 0 ; tools[i].type_name ; i++ ) {
        auto act = mainActions->get_action( Inkscape::Verb::get(tools[i].verb)->get_id() );
        if ( act ) {
            bool setActive = tname && !strcmp(tname, tools[i].type_name);
            auto verbAct = Glib::RefPtr<VerbAction>::cast_dynamic(act);
            if ( verbAct ) {
                verbAct->set_active(setActive);
            }
        }
    }
}


void setup_commands_toolbox(Gtk::Bin *toolbox, SPDesktop *desktop)
{
    setupToolboxCommon(toolbox, desktop, "toolbar-commands.ui", "/ui/CommandsToolbar", "/toolbox/small");
}

void update_commands_toolbox(SPDesktop * /*desktop*/, ToolBase * /*eventcontext*/, Gtk::Bin * /*toolbox*/)
{
}

void setup_snap_toolbox(Gtk::Bin *toolbox, SPDesktop *desktop)
{
    Glib::ustring sizePref("/toolbox/secondary");
    auto toolBar = Inkscape::UI::Toolbar::SnapToolbar::create(desktop);
    auto prefs = Inkscape::Preferences::get();

    if ( prefs->getBool("/toolbox/icononly", true) ) {
        toolBar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
    }

    auto toolboxSize = ToolboxFactory::prefToSize(sizePref.c_str());
    toolBar->set_icon_size(static_cast<Gtk::IconSize>(toolboxSize));

    auto pos = static_cast<Gtk::PositionType>(GPOINTER_TO_INT(toolbox->get_data(HANDLE_POS_MARK)));
    auto orientation = ((pos == Gtk::POS_LEFT) || (pos == Gtk::POS_RIGHT)) ? Gtk::ORIENTATION_HORIZONTAL : Gtk::ORIENTATION_VERTICAL;
    gtk_orientable_set_orientation(GTK_ORIENTABLE(toolBar->gobj()), static_cast<GtkOrientation>(orientation));
    toolBar->set_show_arrow(true);

    auto child = toolbox->get_child();
    if (child) {
        gtk_container_remove(GTK_CONTAINER(toolbox->gobj()), child->gobj());
    }

    toolbox->add(*toolBar);
}

void ToolboxFactory::updateSnapToolbox(SPDesktop *desktop, ToolBase * /*eventcontext*/, Gtk::Bin *toolbox)
{
    auto tb = dynamic_cast<Inkscape::UI::Toolbar::SnapToolbar*>(toolbox->get_child());

    if (!tb) {
        return;
    }

    Inkscape::UI::Toolbar::SnapToolbar::update(tb);
}


#define MODE_LABEL_WIDTH 70


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
