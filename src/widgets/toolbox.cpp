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

#include <gtkmm.h>
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
#include "ui/uxmanager.h"
#include "ui/widget/button.h"
#include "ui/widget/spinbutton.h"
#include "ui/widget/style-swatch.h"
#include "ui/widget/unit-tracker.h"

#include "widgets/spw-utilities.h"
#include "widgets/widget-sizes.h"

#include "xml/attribute-record.h"
#include "xml/node-event-vector.h"

#include "ui/toolbar/arc-toolbar.h"
#include "ui/toolbar/box3d-toolbar.h"
#include "ui/toolbar/calligraphy-toolbar.h"
#include "ui/toolbar/connector-toolbar.h"
#include "ui/toolbar/dropper-toolbar.h"
#include "ui/toolbar/eraser-toolbar.h"
#include "ui/toolbar/gradient-toolbar.h"
#include "ui/toolbar/lpe-toolbar.h"
#include "ui/toolbar/mesh-toolbar.h"
#include "ui/toolbar/measure-toolbar.h"
#include "ui/toolbar/node-toolbar.h"
#include "ui/toolbar/rect-toolbar.h"
#include "ui/toolbar/paintbucket-toolbar.h"
#include "ui/toolbar/pencil-toolbar.h"
#include "ui/toolbar/select-toolbar.h"
//#include "ui/toolbar/snap-toolbar.h"
#include "ui/toolbar/spray-toolbar.h"
#include "ui/toolbar/spiral-toolbar.h"
#include "ui/toolbar/star-toolbar.h"
#include "ui/toolbar/tweak-toolbar.h"
#include "ui/toolbar/text-toolbar.h"
#include "ui/toolbar/zoom-toolbar.h"

#include "toolbox.h"

#include "ui/tools/tool-base.h"

//#define DEBUG_TEXT

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::Tools::ToolBase;

using Inkscape::IO::Resource::get_filename;
using Inkscape::IO::Resource::UIS;

typedef void (*SetupFunction)(GtkWidget *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

enum BarId {
    BAR_TOOL = 0,
    BAR_AUX,
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
    Glib::ustring const tool_name;
    GtkWidget *(*create_func)(SPDesktop *desktop);
    gchar const *swatch_tip;
} const aux_toolboxes[] = {
    // If you change the tool_name for Measure or Text here, change it also in desktop-widget.cpp.
    // clang-format off
    { "/tools/select",          "Select",       Inkscape::UI::Toolbar::SelectToolbar::create,        nullptr},
    { "/tools/nodes",           "Node",         Inkscape::UI::Toolbar::NodeToolbar::create,          nullptr},
    { "/tools/shapes/rect",     "Rect",         Inkscape::UI::Toolbar::RectToolbar::create,          N_("Style of new rectangles")},
    { "/tools/shapes/arc",      "Arc",          Inkscape::UI::Toolbar::ArcToolbar::create,           N_("Style of new ellipses")},
    { "/tools/shapes/star",     "Star",         Inkscape::UI::Toolbar::StarToolbar::create,          N_("Style of new stars")},
    { "/tools/shapes/3dbox",    "3DBox",        Inkscape::UI::Toolbar::Box3DToolbar::create,         N_("Style of new 3D boxes")},
    { "/tools/shapes/spiral",   "Spiral",       Inkscape::UI::Toolbar::SpiralToolbar::create,        N_("Style of new spirals")},
    { "/tools/freehand/pencil", "Pencil",       Inkscape::UI::Toolbar::PencilToolbar::create_pencil, N_("Style of new paths created by Pencil")},
    { "/tools/freehand/pen",    "Pen",          Inkscape::UI::Toolbar::PencilToolbar::create_pen,    N_("Style of new paths created by Pen")},
    { "/tools/calligraphic",    "Calligraphic", Inkscape::UI::Toolbar::CalligraphyToolbar::create,   N_("Style of new calligraphic strokes")},
    { "/tools/text",            "Text",         Inkscape::UI::Toolbar::TextToolbar::create,          nullptr},
    { "/tools/gradient",        "Gradient",     Inkscape::UI::Toolbar::GradientToolbar::create,      nullptr},
    { "/tools/mesh",            "Mesh",         Inkscape::UI::Toolbar::MeshToolbar::create,          nullptr},
    { "/tools/zoom",            "Zoom",         Inkscape::UI::Toolbar::ZoomToolbar::create,          nullptr},
    { "/tools/measure",         "Measure",      Inkscape::UI::Toolbar::MeasureToolbar::create,       nullptr},
    { "/tools/dropper",         "Dropper",      Inkscape::UI::Toolbar::DropperToolbar::create,       nullptr},
    { "/tools/tweak",           "Tweak",        Inkscape::UI::Toolbar::TweakToolbar::create,         N_("Color/opacity used for color tweaking")},
    { "/tools/spray",           "Spray",        Inkscape::UI::Toolbar::SprayToolbar::create,         nullptr},
    { "/tools/connector",       "Connector",    Inkscape::UI::Toolbar::ConnectorToolbar::create,     nullptr},
    { "/tools/paintbucket",     "Paintbucket",  Inkscape::UI::Toolbar::PaintbucketToolbar::create,   N_("Style of Paint Bucket fill objects")},
    { "/tools/eraser",          "Eraser",       Inkscape::UI::Toolbar::EraserToolbar::create,        _("TBD")},
    { "/tools/lpetool",         "LPETool",      Inkscape::UI::Toolbar::LPEToolbar::create,           _("TBD")},
    { nullptr,                  "",             nullptr,                                             nullptr }
    // clang-format on
};


static Glib::RefPtr<Gtk::ActionGroup> create_or_fetch_actions( SPDesktop* desktop );

static void setup_aux_toolbox(GtkWidget *toolbox, SPDesktop *desktop);
static void update_aux_toolbox(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

static void setup_commands_toolbox(GtkWidget *toolbox, SPDesktop *desktop);
static void update_commands_toolbox(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

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
    std::map<SPDesktop*, Glib::RefPtr<Gtk::ActionGroup> >::iterator it = groups.find(desktop);
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
        SP_VERB_DIALOG_PREFERENCES,
        SP_VERB_DIALOG_FILL_STROKE,
        SP_VERB_DIALOG_DOCPROPERTIES,
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
        //SP_VERB_OBJECT_TO_CURVE,
        SP_VERB_SELECTION_GROUP,
        //SP_VERB_SELECTION_OUTLINE,
        SP_VERB_SELECTION_UNGROUP,
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

    return mainActions;
}


static GtkWidget* toolboxNewCommon( GtkWidget* tb, BarId id, GtkPositionType /*handlePos*/ )
{
    g_object_set_data(G_OBJECT(tb), "desktop", nullptr);

    gtk_widget_set_sensitive(tb, TRUE);

    GtkWidget *hb = gtk_event_box_new(); // A simple, neutral container.
    gtk_widget_set_name(hb, "ToolboxCommon");

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    gpointer val = GINT_TO_POINTER(id);
    g_object_set_data(G_OBJECT(hb), BAR_ID_KEY, val);

    return hb;
}

GtkWidget *ToolboxFactory::createToolToolbox()
{
    auto tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(tb, "ToolToolbox");
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);

    Glib::ustring tool_toolbar_builder_file = get_filename(UIS, "toolbar-tool.ui");
    auto builder = Gtk::Builder::create();
    try
    {
        builder->add_from_file(tool_toolbar_builder_file);
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "ToolboxFactor::createToolToolbox: " << tool_toolbar_builder_file << " file not read! " << ex.what() << std::endl;
    }

    Gtk::Toolbar* toolbar = nullptr;
    builder->get_widget("tool-toolbar", toolbar);
    if (!toolbar) {
        std::cerr << "InkscapeWindow: Failed to load tool toolbar!" << std::endl;
    } else {
        gtk_box_pack_start(GTK_BOX(tb), GTK_WIDGET(toolbar->gobj()), false, false, 0);

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if ( prefs->getBool("/toolbox/icononly", true) ) {
            toolbar->set_toolbar_style( Gtk::TOOLBAR_ICONS );
        }

        // TODO: Change preference path!
        GtkIconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/tools/small", 1);
        toolbar->set_icon_size (static_cast<Gtk::IconSize>(toolboxSize));
    }

    return toolboxNewCommon( tb, BAR_TOOL, GTK_POS_LEFT );
}

GtkWidget *ToolboxFactory::createAuxToolbox()
{
    auto tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(tb, "AuxToolbox");
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);

    return toolboxNewCommon( tb, BAR_AUX, GTK_POS_LEFT );
}

//####################################
//# Commands Bar
//####################################

GtkWidget *ToolboxFactory::createCommandsToolbox()
{
    auto tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(tb, "CommandsToolbox");
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);

    return toolboxNewCommon( tb, BAR_COMMANDS, GTK_POS_LEFT );
}

GtkWidget *ToolboxFactory::createSnapToolbox()
{
    auto tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(tb, "SnapToolbox");
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);

    Glib::ustring snap_toolbar_builder_file = get_filename(UIS, "toolbar-snap.ui");
    auto builder = Gtk::Builder::create();
    try
    {
        builder->add_from_file(snap_toolbar_builder_file);
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "ToolboxFactor::createSnapToolbox: " << snap_toolbar_builder_file << " file not read! " << ex.what() << std::endl;
    }

    Gtk::Toolbar* toolbar = nullptr;
    builder->get_widget("snap-toolbar", toolbar);
    if (!toolbar) {
        std::cerr << "InkscapeWindow: Failed to load snap toolbar!" << std::endl;
    } else {
        gtk_box_pack_start(GTK_BOX(tb), GTK_WIDGET(toolbar->gobj()), false, false, 0);

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if ( prefs->getBool("/toolbox/icononly", true) ) {
            toolbar->set_toolbar_style( Gtk::TOOLBAR_ICONS );
        }

        GtkIconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);
        toolbar->set_icon_size (static_cast<Gtk::IconSize>(toolboxSize));
    }

    return toolboxNewCommon( tb, BAR_SNAP, GTK_POS_LEFT );
}

void ToolboxFactory::setToolboxDesktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    sigc::connection *conn = static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox),
                                                                              "event_context_connection"));

    BarId id = static_cast<BarId>( GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toolbox), BAR_ID_KEY)) );

    SetupFunction setup_func = nullptr;
    UpdateFunction update_func = nullptr;

    switch (id) {
        case BAR_TOOL:
            setup_func = nullptr; // setup_tool_toolbox;
            update_func = nullptr; // update_tool_toolbox;
            break;

        case BAR_AUX:
            toolbox = gtk_bin_get_child(GTK_BIN(toolbox));
            setup_func = setup_aux_toolbox;
            update_func = update_aux_toolbox;
            break;

        case BAR_COMMANDS:
            setup_func = setup_commands_toolbox;
            update_func = update_commands_toolbox;
            break;

        case BAR_SNAP:
            setup_func = nullptr;
            update_func = nullptr;
            break;
        default:
            g_warning("Unexpected toolbox id encountered.");
    }

    gpointer ptr = g_object_get_data(G_OBJECT(toolbox), "desktop");
    SPDesktop *old_desktop = static_cast<SPDesktop*>(ptr);

    if (old_desktop) {
        std::vector<Gtk::Widget*> children = Glib::wrap(GTK_CONTAINER(toolbox))->get_children();
        for ( auto i:children ) {
            gtk_container_remove( GTK_CONTAINER(toolbox), i->gobj() );
        }
    }

    g_object_set_data(G_OBJECT(toolbox), "desktop", (gpointer)desktop);

    if (desktop && setup_func && update_func) {
        gtk_widget_set_sensitive(toolbox, TRUE);
        setup_func(toolbox, desktop);
        update_func(desktop, desktop->event_context, toolbox);
        *conn = desktop->connectEventContextChanged(sigc::bind (sigc::ptr_fun(update_func), toolbox));
    } else {
        gtk_widget_set_sensitive(toolbox, TRUE);
    }

} // end of sp_toolbox_set_desktop()


static void setupToolboxCommon( GtkWidget *toolbox,
                                SPDesktop *desktop,
                                gchar const *ui_file,
                                gchar const* toolbarName,
                                gchar const* sizePref )
{
    Glib::RefPtr<Gtk::ActionGroup> mainActions = create_or_fetch_actions( desktop );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    GtkUIManager* mgr = gtk_ui_manager_new();
    GError* err = nullptr;

    GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

    gtk_ui_manager_insert_action_group( mgr, mainActions->gobj(), 0 );

    Glib::ustring filename = get_filename(UIS, ui_file);
    gtk_ui_manager_add_ui_from_file( mgr, filename.c_str(), &err );
    if(err) {
        g_warning("Failed to load %s: %s", filename.c_str(), err->message);
        g_error_free(err);
        return;
    }

    GtkWidget* toolBar = gtk_ui_manager_get_widget( mgr, toolbarName );
    if ( prefs->getBool("/toolbox/icononly", true) ) {
        gtk_toolbar_set_style( GTK_TOOLBAR(toolBar), GTK_TOOLBAR_ICONS );
    }

    GtkIconSize toolboxSize = ToolboxFactory::prefToSize(sizePref);
    gtk_toolbar_set_icon_size( GTK_TOOLBAR(toolBar), static_cast<GtkIconSize>(toolboxSize) );

    GtkPositionType pos = static_cast<GtkPositionType>(GPOINTER_TO_INT(g_object_get_data( G_OBJECT(toolbox), HANDLE_POS_MARK )));
    orientation = ((pos == GTK_POS_LEFT) || (pos == GTK_POS_RIGHT)) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    gtk_orientable_set_orientation (GTK_ORIENTABLE(toolBar), orientation);
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolBar), TRUE);

    g_object_set_data(G_OBJECT(toolBar), "desktop", nullptr);

    GtkWidget* child = gtk_bin_get_child(GTK_BIN(toolbox));
    if ( child ) {
        gtk_container_remove( GTK_CONTAINER(toolbox), child );
    }

    gtk_container_add( GTK_CONTAINER(toolbox), toolBar );
}

#define noDUMP_DETAILS 1

void ToolboxFactory::setOrientation(GtkWidget* toolbox, GtkOrientation orientation)
{
#if DUMP_DETAILS
    g_message("Set orientation for %p to be %d", toolbox, orientation);
    GType type = G_OBJECT_TYPE(toolbox);
    g_message("        [%s]", g_type_name(type));
    g_message("             %p", g_object_get_data(G_OBJECT(toolbox), BAR_ID_KEY));
#endif

    GtkPositionType pos = (orientation == GTK_ORIENTATION_HORIZONTAL) ? GTK_POS_LEFT : GTK_POS_TOP;

    if (GTK_IS_BIN(toolbox)) {
#if DUMP_DETAILS
        g_message("            is a BIN");
#endif // DUMP_DETAILS
        GtkWidget* child = gtk_bin_get_child(GTK_BIN(toolbox));
        if (child) {
#if DUMP_DETAILS
            GType type2 = G_OBJECT_TYPE(child);
            g_message("            child    [%s]", g_type_name(type2));
#endif // DUMP_DETAILS

            if (GTK_IS_BOX(child)) {
#if DUMP_DETAILS
                g_message("                is a BOX");
#endif // DUMP_DETAILS

                std::vector<Gtk::Widget*> children = Glib::wrap(GTK_CONTAINER(child))->get_children();
                if (!children.empty()) {
                    for (auto curr:children) {
                        GtkWidget* child2 = curr->gobj();
#if DUMP_DETAILS
                        GType type3 = G_OBJECT_TYPE(child2);
                        g_message("                child2   [%s]", g_type_name(type3));
#endif // DUMP_DETAILS

                        if (GTK_IS_CONTAINER(child2)) {
                            std::vector<Gtk::Widget*> children2 = Glib::wrap(GTK_CONTAINER(child2))->get_children();
                            if (!children2.empty()) {
                                for (auto curr2:children2) {
                                    GtkWidget* child3 = curr2->gobj();
#if DUMP_DETAILS
                                    GType type4 = G_OBJECT_TYPE(child3);
                                    g_message("                    child3   [%s]", g_type_name(type4));
#endif // DUMP_DETAILS
                                    if (GTK_IS_TOOLBAR(child3)) {
                                        GtkToolbar* childBar = GTK_TOOLBAR(child3);
                                        gtk_orientable_set_orientation(GTK_ORIENTABLE(childBar), orientation);
                                    }
                                }
                            }
                        }


                        if (GTK_IS_TOOLBAR(child2)) {
                            GtkToolbar* childBar = GTK_TOOLBAR(child2);
                            gtk_orientable_set_orientation(GTK_ORIENTABLE(childBar), orientation);
                        } else {
                            g_message("need to add dynamic switch");
                        }
                    }
                } else {
                    // The call is being made before the toolbox proper has been setup.
                    g_object_set_data(G_OBJECT(toolbox), HANDLE_POS_MARK, GINT_TO_POINTER(pos));
                }
            } else if (GTK_IS_TOOLBAR(child)) {
                GtkToolbar* toolbar = GTK_TOOLBAR(child);
                gtk_orientable_set_orientation( GTK_ORIENTABLE(toolbar), orientation );
            }
        }
    }
}

/**
 * \brief Generate the auxiliary toolbox
 *
 * \details This is the one that appears below the main menu, and contains
 *          tool-specific toolbars.  Each toolbar is created here, using
 *          its "create" method.
 *
 *          The actual method used for each toolbar is specified in the
 *          "aux_toolboxes" array, defined above.
 */
void setup_aux_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // Loop through all the toolboxes and create them using either
    // their "create" methods.
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        if (aux_toolboxes[i].create_func) {
            GtkWidget *sub_toolbox = aux_toolboxes[i].create_func(desktop);
            gtk_widget_set_name( sub_toolbox, "SubToolBox" );

            auto holder = gtk_grid_new();
            gtk_grid_attach(GTK_GRID(holder), sub_toolbox, 0, 0, 1, 1);

            // This part is just for styling
            if ( prefs->getBool( "/toolbox/icononly", true) ) {
                gtk_toolbar_set_style( GTK_TOOLBAR(sub_toolbox), GTK_TOOLBAR_ICONS );
            }

            GtkIconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/small");
            gtk_toolbar_set_icon_size( GTK_TOOLBAR(sub_toolbox), static_cast<GtkIconSize>(toolboxSize) );
            gtk_widget_set_hexpand(sub_toolbox, TRUE);

            // Add a swatch widget if swatch tooltip is defined.
            if ( aux_toolboxes[i].swatch_tip) {
                auto swatch = new Inkscape::UI::Widget::StyleSwatch( nullptr, _(aux_toolboxes[i].swatch_tip) );
                swatch->setDesktop( desktop );
                swatch->setToolName(aux_toolboxes[i].tool_name);
                // swatch->setClickVerb( aux_toolboxes[i].swatch_verb_id );
                swatch->setWatchedTool( aux_toolboxes[i].type_name, true );
                swatch->set_margin_start(AUX_BETWEEN_BUTTON_GROUPS);
                swatch->set_margin_end(AUX_BETWEEN_BUTTON_GROUPS);
                swatch->set_margin_top(AUX_SPACING);
                swatch->set_margin_bottom(AUX_SPACING);

                auto swatch_ = GTK_WIDGET( swatch->gobj() );
                gtk_grid_attach( GTK_GRID(holder), swatch_, 1, 0, 1, 1);
            }

            // Add the new toolbar into the toolbox (i.e., make it the visible toolbar)
            // and also store a pointer to it inside the toolbox.  This allows the
            // active toolbar to be changed.
            gtk_container_add(GTK_CONTAINER(toolbox), holder);
            Glib::ustring ui_name = aux_toolboxes[i].tool_name + "Toolbar";  // If you change "Toolbar" here, change it also in desktop-widget.cpp.
            gtk_widget_set_name( holder, ui_name.c_str() );

            // TODO: We could make the toolbox a custom subclass of GtkEventBox
            //       so that we can store a list of toolbars, rather than using
            //       GObject data
            g_object_set_data(G_OBJECT(toolbox), aux_toolboxes[i].tool_name.c_str(), holder);
            gtk_widget_show(sub_toolbox);
            gtk_widget_show(holder);
        } else if (aux_toolboxes[i].swatch_tip) {
            g_warning("Could not create toolbox %s", aux_toolboxes[i].tool_name.c_str());
        }
    }
}

void update_aux_toolbox(SPDesktop * /*desktop*/, ToolBase *eventcontext, GtkWidget *toolbox)
{
    gchar const *tname = ( eventcontext
                           ? eventcontext->getPrefsPath().c_str() //g_type_name(G_OBJECT_TYPE(eventcontext))
                           : nullptr );
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        GtkWidget *sub_toolbox = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), aux_toolboxes[i].tool_name.c_str()));
        if (tname && !strcmp(tname, aux_toolboxes[i].type_name)) {
            gtk_widget_show_now(sub_toolbox);
            g_object_set_data(G_OBJECT(toolbox), "shows", sub_toolbox);
        } else {
            gtk_widget_hide(sub_toolbox);
        }
        //FIX issue #Inkscape686
        GtkAllocation allocation;
        gtk_widget_get_allocation(sub_toolbox, &allocation);
        gtk_widget_size_allocate(sub_toolbox, &allocation);
    }
    //FIX issue #Inkscape125
    GtkAllocation allocation;
    gtk_widget_get_allocation(toolbox, &allocation);
    gtk_widget_size_allocate(toolbox, &allocation);  
}

void setup_commands_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    setupToolboxCommon(toolbox, desktop, "toolbar-commands.ui", "/ui/CommandsToolbar", "/toolbox/small");
}

void update_commands_toolbox(SPDesktop * /*desktop*/, ToolBase * /*eventcontext*/, GtkWidget * /*toolbox*/)
{
}

Glib::ustring ToolboxFactory::getToolboxName(GtkWidget* toolbox)
{
    Glib::ustring name;
    BarId id = static_cast<BarId>( GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toolbox), BAR_ID_KEY)) );
    switch(id) {
        case BAR_TOOL:
            name = "ToolToolbar";
            break;
        case BAR_AUX:
            name = "AuxToolbar";
            break;
        case BAR_COMMANDS:
            name = "CommandsToolbar";
            break;
        case BAR_SNAP:
            name = "SnapToolbar";
            break;
    }

    return name;
}

void ToolboxFactory::showAuxToolbox(GtkWidget *toolbox_toplevel)
{
    gtk_widget_show(toolbox_toplevel);
    GtkWidget *toolbox = gtk_bin_get_child(GTK_BIN(toolbox_toplevel));

    GtkWidget *shown_toolbox = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), "shows"));
    if (!shown_toolbox) {
        return;
    }
    gtk_widget_show(toolbox);
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
