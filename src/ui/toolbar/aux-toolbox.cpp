// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Inkscape Auxilliary toolbar
 * Each tool should have its own xxx-toolbar implementation file
 *
 * @authors Inkscape Authors
 * Copyright (C) 1999-2010 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "aux-toolbox.h"

#include <glibmm/i18n.h>

#include <gtkmm/box.h>
#include <gtkmm/grid.h>

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
#include "ui/toolbar/spray-toolbar.h"
#include "ui/toolbar/spiral-toolbar.h"
#include "ui/toolbar/star-toolbar.h"
#include "ui/toolbar/tweak-toolbar.h"
#include "ui/toolbar/text-toolbar.h"
#include "ui/toolbar/zoom-toolbar.h"

#include "ui/tools/tool-base.h"

#include "ui/widget/style-swatch.h"

#include "widgets/toolbox.h"
#include "widgets/widget-sizes.h"

#include "verbs.h"

static struct {
    gchar const *type_name;
    gchar const *data_name;
    Inkscape::UI::Toolbar::Toolbar *(*create_func)(SPDesktop *desktop);
    gchar const *ui_name;
    gint swatch_verb_id;
    gchar const *swatch_tool;
    gchar const *swatch_tip;
} const aux_toolboxes[] = {
    { "/tools/select",          "select_toolbox",      Inkscape::UI::Toolbar::SelectToolbar::create,        "SelectToolbar",
      SP_VERB_INVALID,                    nullptr,                  nullptr},
    { "/tools/nodes",           "node_toolbox",        Inkscape::UI::Toolbar::NodeToolbar::create,          "NodeToolbar",
      SP_VERB_INVALID,                    nullptr,                  nullptr},
    { "/tools/tweak",           "tweak_toolbox",       Inkscape::UI::Toolbar::TweakToolbar::create,         "TweakToolbar",
      SP_VERB_CONTEXT_TWEAK_PREFS,        "/tools/tweak",           N_("Color/opacity used for color tweaking")},
    { "/tools/spray",           "spray_toolbox",       Inkscape::UI::Toolbar::SprayToolbar::create,         "SprayToolbar",
      SP_VERB_INVALID,                    nullptr,                  nullptr},
    { "/tools/zoom",            "zoom_toolbox",        Inkscape::UI::Toolbar::ZoomToolbar::create,          "ZoomToolbar",
      SP_VERB_INVALID,                    nullptr,                  nullptr},
    // If you change MeasureToolbar here, change it also in desktop-widget.cpp
    { "/tools/measure",         "measure_toolbox",     Inkscape::UI::Toolbar::MeasureToolbar::create,       "MeasureToolbar",
      SP_VERB_INVALID,                    nullptr,                  nullptr},
    { "/tools/shapes/star",     "star_toolbox",        Inkscape::UI::Toolbar::StarToolbar::create,          "StarToolbar",
      SP_VERB_CONTEXT_STAR_PREFS,         "/tools/shapes/star",     N_("Style of new stars")},
    { "/tools/shapes/rect",     "rect_toolbox",        Inkscape::UI::Toolbar::RectToolbar::create,          "RectToolbar",
      SP_VERB_CONTEXT_RECT_PREFS,         "/tools/shapes/rect",     N_("Style of new rectangles")},
    { "/tools/shapes/3dbox",    "3dbox_toolbox",       Inkscape::UI::Toolbar::Box3DToolbar::create,         "3DBoxToolbar",
      SP_VERB_CONTEXT_3DBOX_PREFS,        "/tools/shapes/3dbox",    N_("Style of new 3D boxes")},
    { "/tools/shapes/arc",      "arc_toolbox",         Inkscape::UI::Toolbar::ArcToolbar::create,           "ArcToolbar",
      SP_VERB_CONTEXT_ARC_PREFS,          "/tools/shapes/arc",      N_("Style of new ellipses")},
    { "/tools/shapes/spiral",   "spiral_toolbox",      Inkscape::UI::Toolbar::SpiralToolbar::create,        "SpiralToolbar",
      SP_VERB_CONTEXT_SPIRAL_PREFS,       "/tools/shapes/spiral",   N_("Style of new spirals")},
    { "/tools/freehand/pencil", "pencil_toolbox",      Inkscape::UI::Toolbar::PencilToolbar::create_pencil, "PencilToolbar",
      SP_VERB_CONTEXT_PENCIL_PREFS,       "/tools/freehand/pencil", N_("Style of new paths created by Pencil")},
    { "/tools/freehand/pen",    "pen_toolbox",         Inkscape::UI::Toolbar::PencilToolbar::create_pen,    "PenToolbar",
      SP_VERB_CONTEXT_PEN_PREFS,          "/tools/freehand/pen",    N_("Style of new paths created by Pen")},
    { "/tools/calligraphic",    "calligraphy_toolbox", Inkscape::UI::Toolbar::CalligraphyToolbar::create,   "CalligraphyToolbar",
      SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS, "/tools/calligraphic",    N_("Style of new calligraphic strokes")},
    { "/tools/eraser",          "eraser_toolbox",      Inkscape::UI::Toolbar::EraserToolbar::create,        "EraserToolbar",
      SP_VERB_CONTEXT_ERASER_PREFS,       "/tools/eraser",           _("TBD")},
    { "/tools/lpetool",         "lpetool_toolbox",     Inkscape::UI::Toolbar::LPEToolbar::create,           "LPEToolToolbar",
      SP_VERB_CONTEXT_LPETOOL_PREFS,      "/tools/lpetool",          _("TBD")},
    // If you change TextToolbar here, change it also in desktop-widget.cpp
    { "/tools/text",            "text_toolbox",        Inkscape::UI::Toolbar::TextToolbar::create,          "TextToolbar",
      SP_VERB_INVALID,                    nullptr,                   nullptr},
    { "/tools/dropper",         "dropper_toolbox",     Inkscape::UI::Toolbar::DropperToolbar::create,       "DropperToolbar",
      SP_VERB_INVALID,                    nullptr,                   nullptr},
    { "/tools/connector",       "connector_toolbox",   Inkscape::UI::Toolbar::ConnectorToolbar::create,     "ConnectorToolbar",
      SP_VERB_INVALID,                    nullptr,                   nullptr},
    { "/tools/gradient",        "gradient_toolbox",    Inkscape::UI::Toolbar::GradientToolbar::create,      "GradientToolbar",
      SP_VERB_INVALID,                    nullptr,                   nullptr},
    { "/tools/mesh",            "mesh_toolbox",        Inkscape::UI::Toolbar::MeshToolbar::create,          "MeshToolbar",
      SP_VERB_INVALID,                    nullptr,                   nullptr},
    { "/tools/paintbucket",     "paintbucket_toolbox", Inkscape::UI::Toolbar::PaintbucketToolbar::create,   "PaintbucketToolbar",
      SP_VERB_CONTEXT_PAINTBUCKET_PREFS, "/tools/paintbucket",       N_("Style of Paint Bucket fill objects")},
    { nullptr,                  nullptr,               nullptr,                                             nullptr,
        SP_VERB_INVALID,                 nullptr,                    nullptr }
};

namespace Inkscape {
namespace UI {
namespace Toolbar {

AuxToolbox::AuxToolbox()
{
    set_name("AuxToolbox");

    _box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 0);
    _box->set_homogeneous(false);
    add(*_box);
    set_sensitive(false);
    show_all();
}

void
AuxToolbox::setup(SPDesktop *desktop)
{
    _desktop = desktop;
    auto prefs = Inkscape::Preferences::get();

    // Loop through all the toolbars that can be shown here, create them and
    // store each one in a different cell of the main box
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        if (aux_toolboxes[i].create_func) {
            auto sub_toolbox = aux_toolboxes[i].create_func(desktop);

            // We use a Gtk::Grid here so that we can show a tool-specific
            // swatch next to the toolbar
            auto holder = Gtk::make_managed<Gtk::Grid>();
            holder->attach(*sub_toolbox, 0, 0, 1, 1);

            // This part is just for styling
            if ( prefs->getBool( "/toolbox/icononly", true) ) {
                sub_toolbox->set_toolbar_style(Gtk::TOOLBAR_ICONS);
            }

            auto toolboxSize = ToolboxFactory::prefToSize("/toolbox/small");
            sub_toolbox->set_icon_size(static_cast<Gtk::IconSize>(toolboxSize));
            sub_toolbox->set_hexpand(true);

            // Add a swatch widget if one was specified
            if ( aux_toolboxes[i].swatch_verb_id != SP_VERB_INVALID ) {
                auto swatch = Gtk::make_managed<Inkscape::UI::Widget::StyleSwatch>(nullptr, _(aux_toolboxes[i].swatch_tip));
                swatch->setDesktop(desktop);
                swatch->setClickVerb(aux_toolboxes[i].swatch_verb_id);
                swatch->setWatchedTool(aux_toolboxes[i].swatch_tool, true);
                swatch->set_margin_start(AUX_BETWEEN_BUTTON_GROUPS);
                swatch->set_margin_end(AUX_BETWEEN_BUTTON_GROUPS);
                swatch->set_margin_top(AUX_SPACING);
                swatch->set_margin_bottom(AUX_SPACING);

                holder->attach(*swatch, 1, 0, 1, 1);
            }

            // Add the new toolbar into the toolbox
            // and also store a pointer to it inside the toolbox.  This allows the
            // active toolbar to be changed.
            _box->add(*holder);
            holder->set_name(aux_toolboxes[i].ui_name);
            _toolbar_map[aux_toolboxes[i].data_name] = holder;
            sub_toolbox->show();
            holder->show_all();
        } else if (aux_toolboxes[i].swatch_verb_id != SP_VERB_NONE) {
            g_warning("Could not create toolbox %s", aux_toolboxes[i].ui_name);
        }
    }
}

void
AuxToolbox::update(SPDesktop * /*desktop*/, Tools::ToolBase *eventcontext)
{
    gchar const *tname = ( eventcontext
            ? eventcontext->getPrefsPath().c_str() //g_type_name(G_OBJECT_TYPE(eventcontext))
            : nullptr );
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        auto sub_toolbox = _toolbar_map[aux_toolboxes[i].data_name];
        if (tname && !strcmp(tname, aux_toolboxes[i].type_name)) {
            sub_toolbox->show_all();
            _shows = sub_toolbox;
        } else {
            sub_toolbox->hide();
        }
        //FIX issue #Inkscape686
        auto allocation = sub_toolbox->get_allocation();
        sub_toolbox->size_allocate(allocation);
    }
    //FIX issue #Inkscape125
    auto allocation = get_allocation();
    size_allocate(allocation);
}

void
AuxToolbox::set_desktop(decltype(_desktop) desktop)
{
    auto old_desktop = _desktop;

    // purge all existing toolbars
    if (old_desktop) {
        auto children = get_children();
        for (auto i:children) {
            gtk_container_remove(GTK_CONTAINER(gobj()), i->gobj());
        }
    }

    _desktop = desktop;

    if (desktop) {
        set_sensitive(true);
        setup(desktop);
        update(desktop, desktop->event_context);
        _event_context_connection = desktop->connectEventContextChanged(sigc::mem_fun(*this, &AuxToolbox::update));
    } else {
        set_sensitive(false);
    }
}

/**
 * Shows the currently selected tool-specific toolbar
 */
void
AuxToolbox::show_aux_toolbox()
{
    show();

    if (_shows) {
        _shows->show_all();
    }
}

} // namespace Toolbar
} // namespace UI
} // namespace Inkscape

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
