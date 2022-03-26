// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Desktop widget implementation
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"  // only include where actually required!
#endif

#include <2geom/rect.h>

#include "attributes.h"
#include "cms-system.h"
#include "conn-avoid-ref.h"
#include "desktop-events.h"
#include "desktop-widget.h"
#include "desktop.h"
#include "document-undo.h"
#include "ege-color-prof-tracker.h"
#include "enums.h"
#include "file.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "inkscape-version.h"

#include "display/control/canvas-axonomgrid.h"
#include "display/control/canvas-item-drawing.h"
#include "display/control/canvas-item-guideline.h"

#include "extension/db.h"

#include "object/sp-image.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"

#include "ui/shortcuts.h"
#include "ui/dialog/swatches.h"
#include "ui/icon-loader.h"
#include "ui/icon-names.h"
#include "ui/monitor.h"   // Monitor aspect ratio
#include "ui/dialog/dialog-container.h"
#include "ui/dialog/dialog-multipaned.h"
#include "ui/dialog/dialog-window.h"
#include "ui/tools/box3d-tool.h"
#include "ui/util.h"
#include "ui/widget/canvas.h"
#include "ui/widget/canvas-grid.h"
#include "ui/widget/combo-tool-item.h"
#include "ui/widget/ink-ruler.h"
#include "ui/widget/layer-selector.h"
#include "ui/widget/page-selector.h"
#include "ui/widget/selected-style.h"
#include "ui/widget/spin-button-tool-item.h"
#include "ui/widget/unit-tracker.h"
#include "ui/themes.h"

#include "util/units.h"

// We're in the "widgets" directory, so no need to explicitly prefix these:
#include "spw-utilities.h"
#include "toolbox.h"
#include "widget-sizes.h"

#include "ui/widget/color-palette.h"
#include "ui/widget/preview.h"
#include "ui/dialog/color-item.h"
#include "widgets/ege-paint-def.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::Dialog::DialogContainer;
using Inkscape::UI::Dialog::DialogMultipaned;
using Inkscape::UI::Dialog::DialogWindow;
using Inkscape::UI::Widget::UnitTracker;
using Inkscape::UI::ToolboxFactory;
using Inkscape::Util::unit_table;


//---------------------------------------------------------------------
/* SPDesktopWidget */

class CMSPrefWatcher {
public:
    CMSPrefWatcher() :
        _dpw(*this),
        _tracker(ege_color_prof_tracker_new(nullptr))
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        g_signal_connect( G_OBJECT(_tracker), "modified", G_CALLBACK(hook), this );
        prefs->addObserver(_dpw);
    }
    virtual ~CMSPrefWatcher() = default;

    //virtual void notify(PrefValue &);
    void add( SPDesktopWidget* dtw ) {
        _widget_list.push_back(dtw);
    }
    void remove( SPDesktopWidget* dtw ) {
        _widget_list.remove(dtw);
    }

private:
    static void hook(EgeColorProfTracker *tracker, gint b, CMSPrefWatcher *watcher);

    class DisplayProfileWatcher : public Inkscape::Preferences::Observer {
    public:
        DisplayProfileWatcher(CMSPrefWatcher &pw) : Observer("/options/displayprofile"), _pw(pw) {}
        void notify(Inkscape::Preferences::Entry const &/*val*/) override {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            _pw._setCmsSensitive(!prefs->getString("/options/displayprofile/uri").empty());
        }
    private:
        CMSPrefWatcher &_pw;
    };

    DisplayProfileWatcher _dpw;

    void _setCmsSensitive(bool value);

    std::list<SPDesktopWidget*> _widget_list;
    EgeColorProfTracker *_tracker;

    friend class DisplayProfileWatcher;
};

void CMSPrefWatcher::hook(EgeColorProfTracker * /*tracker*/, gint monitor, CMSPrefWatcher * /*watcher*/)
{
    unsigned char* buf = nullptr;
    guint len = 0;

    ege_color_prof_tracker_get_profile_for( monitor, reinterpret_cast<gpointer*>(&buf), &len );
    Glib::ustring id = Inkscape::CMSSystem::setDisplayPer( buf, len, monitor );
}

void CMSPrefWatcher::_setCmsSensitive(bool enabled)
{
    for ( auto dtw : _widget_list ) {
        auto cms_adj = dtw->get_canvas_grid()->GetCmsAdjust();
        if ( cms_adj->get_sensitive() != enabled ) {
            dtw->cms_adjust_set_sensitive(enabled);
        }
    }
}

static CMSPrefWatcher* watcher = nullptr;

bool SPDesktopWidget::SignalEvent(GdkEvent* event)
{
    /**
     * What we want to do here is pass the keyboard events *up* to the canvas
     * if the mouse is hovering over the canvas. IF and only if, the canvas
     * is not currently listening to the event stack because it's not in focus.
     */
    if (event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE) {
        if (!_canvas->is_focus() && _canvas_grid->mouse_inside) {
            return sp_desktop_root_handler(event, desktop);
        }
    }
    return false;
}

SPDesktopWidget::SPDesktopWidget(InkscapeWindow* inkscape_window)
    : window (inkscape_window)
{
    auto *const dtw = this;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    /* Main table */
    dtw->_vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    dtw->_vbox->set_name("DesktopMainTable");
    dtw->add(*dtw->_vbox);

    /* Status bar */
    dtw->_statusbar = Gtk::manage(new Gtk::Box());
    dtw->_statusbar->set_name("DesktopStatusBar");
    dtw->_vbox->pack_end(*dtw->_statusbar, false, true);

    /* Swatch Bar */
    dtw->_panels = Gtk::manage(new Inkscape::UI::Dialog::SwatchesPanel("/embedded/swatches"));
    dtw->_panels->set_vexpand(false);
    dtw->_vbox->pack_end(*dtw->_panels, false, true);

    /* DesktopHBox (Vertical toolboxes, canvas) */
    dtw->_hbox = Gtk::manage(new Gtk::Box());
    dtw->_hbox->set_name("DesktopHbox");

    dtw->_tbbox = Gtk::manage(new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL));
    dtw->_tbbox->set_name("ToolboxCanvasPaned");
    dtw->_hbox->pack_start(*dtw->_tbbox, true, true);

    dtw->_vbox->pack_end(*dtw->_hbox, true, true);

    dtw->_top_toolbars = Gtk::make_managed<Gtk::Grid>();
    dtw->_top_toolbars->set_name("TopToolbars");
    dtw->_vbox->pack_end(*dtw->_top_toolbars, false, true);

    /* Toolboxes */
    dtw->aux_toolbox = ToolboxFactory::createAuxToolbox();

    dtw->snap_toolbox = ToolboxFactory::createSnapToolbox();
    ToolboxFactory::setOrientation(dtw->snap_toolbox, GTK_ORIENTATION_HORIZONTAL);
    dtw->_top_toolbars->attach(*Glib::wrap(dtw->snap_toolbox), 1, 0, 1, 2);

    dtw->commands_toolbox = ToolboxFactory::createCommandsToolbox();
    auto cmd = Glib::wrap(dtw->commands_toolbox);
    dtw->_top_toolbars->attach(*cmd, 0, 0);

    dtw->_top_toolbars->attach(*Glib::wrap(dtw->aux_toolbox), 0, 1);

    dtw->tool_toolbox = ToolboxFactory::createToolToolbox();
    ToolboxFactory::setOrientation( dtw->tool_toolbox, GTK_ORIENTATION_VERTICAL );
    dtw->_tbbox->pack1(*Glib::wrap(dtw->tool_toolbox), false, true);
    auto tbox_width = prefs->getEntry("/toolbox/tools/width");
    if (tbox_width.isValid()) {
        _tbbox->set_position(tbox_width.getIntLimited(32, 8, 500));
    }

    auto set_visible_buttons = [=](GtkWidget* tb) {
        int buttons_before_separator = 0;
        Gtk::Widget* last_sep = nullptr;
        Gtk::FlowBox* last_box = nullptr;
        sp_traverse_widget_tree(Glib::wrap(tb), [&](Gtk::Widget* widget) {
            if (auto flowbox = dynamic_cast<Gtk::FlowBox*>(widget)) {
                flowbox->show();
                flowbox->set_no_show_all();
                flowbox->set_max_children_per_line(1);
                last_box = flowbox;
            }
            else if (auto btn = dynamic_cast<Gtk::Button*>(widget)) {
                auto name = sp_get_action_target(widget);
                auto show = prefs->getBool(ToolboxFactory::get_tool_visible_buttons_path(name), true);
                auto parent = btn->get_parent();
                if (show) {
                    parent->show();
                    ++buttons_before_separator;
                    // keep the max_children up to date improves display.
                    last_box->set_max_children_per_line(buttons_before_separator);
                    last_sep = nullptr;
                }
                else {
                    parent->hide();
                }
            }
            else if (auto sep = dynamic_cast<Gtk::Separator*>(widget)) {
                if (buttons_before_separator <= 0) {
                    sep->hide();
                }
                else {
                    sep->show();
                    buttons_before_separator = 0;
                    last_sep = sep;
                }
            }
            return false;
        });
        if (last_sep) {
            // hide trailing separator
            last_sep->hide();
        }
    };
    auto set_toolbar_prefs = [=]() {
        int min = ToolboxFactory::min_pixel_size;
        int max = ToolboxFactory::max_pixel_size;
        int s = prefs->getIntLimited(ToolboxFactory::tools_icon_size, min, min, max);
        ToolboxFactory::set_icon_size(tool_toolbox, s);
    };

    // watch for changes
    _tb_icon_sizes1 = prefs->createObserver(ToolboxFactory::tools_icon_size,    [=]() { set_toolbar_prefs(); });
    _tb_icon_sizes2 = prefs->createObserver(ToolboxFactory::ctrlbars_icon_size, [=]() { apply_ctrlbar_settings(); });
    _tb_visible_buttons = prefs->createObserver(ToolboxFactory::tools_visible_buttons, [=]() { set_visible_buttons(tool_toolbox); });

    // restore preferences
    set_toolbar_prefs();
    apply_ctrlbar_settings();
    set_visible_buttons(tool_toolbox);

    /* Canvas Grid (canvas, rulers, scrollbars, etc.) */
    // desktop widgets owns it
    _canvas_grid = new Inkscape::UI::Widget::CanvasGrid(this);

    /* Canvas */
    dtw->_canvas = _canvas_grid->GetCanvas();
    dtw->_canvas->set_cms_active(prefs->getBool("/options/displayprofile/enable"));

    /* Dialog Container */
    _container = Gtk::manage(new DialogContainer(inkscape_window));
    _columns = _container->get_columns();
    _columns->set_dropzone_sizes(2, -1);
    dtw->_tbbox->pack2(*_container, true, true);

    _canvas_grid->set_hexpand(true);
    _canvas_grid->set_vexpand(true);
    _columns->append(_canvas_grid);

    // --------------- Status Tool Bar ------------------//

    // Selected Style (Fill/Stroke/Opacity)
    dtw->_selected_style = Gtk::manage(new Inkscape::UI::Widget::SelectedStyle(true));
    dtw->_statusbar->pack_start(*dtw->_selected_style, false, false);
    _selected_style->show_all();
    _selected_style->set_no_show_all();

    // Layer Selector
    _layer_selector = Gtk::manage(new Inkscape::UI::Widget::LayerSelector(nullptr));
    // separate layer selector buttons from status text
    auto vseparator = Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_VERTICAL);
    vseparator->set_margin_end(6);
    vseparator->set_margin_top(6);
    vseparator->set_margin_bottom(6);
    _layer_selector->pack_end(*vseparator);
    _layer_selector->show_all();
    _layer_selector->set_no_show_all();
    dtw->_statusbar->pack_start(*_layer_selector, false, false, 1);

    // Select Status
    dtw->_select_status = Gtk::manage(new Gtk::Label());
    dtw->_select_status->set_name("SelectStatus");
    dtw->_select_status->set_ellipsize(Pango::ELLIPSIZE_END);
    dtw->_select_status->set_line_wrap(true);
    dtw->_select_status->set_lines(2);
    dtw->_select_status->set_halign(Gtk::ALIGN_START);
    dtw->_select_status->set_size_request(1, -1);

    // Display the initial welcome message in the statusbar
    dtw->_select_status->set_markup(_("<b>Welcome to Inkscape!</b> Use shape or freehand tools to create objects; use selector (arrow) to move or transform them."));

    dtw->_statusbar->pack_start(*dtw->_select_status, true, true);

    dtw->_zoom_status_box = Gtk::make_managed<Gtk::Box>();
    // Zoom status spinbutton ---------------
    auto zoom_adj = Gtk::Adjustment::create(100.0, log(SP_DESKTOP_ZOOM_MIN)/log(2), log(SP_DESKTOP_ZOOM_MAX)/log(2), 0.1);
    dtw->_zoom_status = Gtk::manage(new Inkscape::UI::Widget::SpinButton(zoom_adj));

    dtw->_zoom_status->set_defocus_widget(dtw->_canvas);
    dtw->_zoom_status->set_tooltip_text(_("Zoom"));
    dtw->_zoom_status->set_size_request(STATUS_ZOOM_WIDTH, -1);
    dtw->_zoom_status->set_width_chars(6);
    dtw->_zoom_status->set_numeric(false);
    dtw->_zoom_status->set_update_policy(Gtk::UPDATE_ALWAYS);

    // Callbacks
    dtw->_zoom_status_input_connection  = dtw->_zoom_status->signal_input().connect(sigc::mem_fun(dtw, &SPDesktopWidget::zoom_input));
    dtw->_zoom_status_output_connection = dtw->_zoom_status->signal_output().connect(sigc::mem_fun(dtw, &SPDesktopWidget::zoom_output));
    dtw->_zoom_status_value_changed_connection = dtw->_zoom_status->signal_value_changed().connect(sigc::mem_fun(dtw, &SPDesktopWidget::zoom_value_changed));
    dtw->_zoom_status_populate_popup_connection = dtw->_zoom_status->signal_populate_popup().connect(sigc::mem_fun(dtw, &SPDesktopWidget::zoom_populate_popup));

    // Style
    auto css_provider_spinbutton = Gtk::CssProvider::create();
    css_provider_spinbutton->load_from_data("* { padding-left: 2px; padding-right: 2px; padding-top: 0px; padding-bottom: 0px;}");  // Shouldn't this be in a style sheet? Used also by rotate.

    dtw->_zoom_status->set_name("ZoomStatus");
    auto context_zoom = dtw->_zoom_status->get_style_context();
    context_zoom->add_provider(css_provider_spinbutton, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    dtw->_rotation_status_box = Gtk::make_managed<Gtk::Box>();
    dtw->_rotation_status_box->set_margin_start(10);
    // Rotate status spinbutton ---------------
    auto rotation_adj = Gtk::Adjustment::create(0, -360.0, 360.0, 1.0);

    dtw->_rotation_status = Gtk::manage(new Inkscape::UI::Widget::SpinButton(rotation_adj));

    // FIXME: This is a bit of a hack, to avoid the ExpressionEvaluator struggling to parse the
    //        degree symbol.  It would be better to improve ExpressionEvaluator so it copes
    dtw->_rotation_status->set_dont_evaluate(true);

    dtw->_rotation_status->set_defocus_widget(dtw->_canvas);
    dtw->_rotation_status->set_tooltip_text(_("Rotation. (Also Ctrl+Shift+Scroll)"));
    dtw->_rotation_status->set_size_request(STATUS_ROTATION_WIDTH, -1);
    dtw->_rotation_status->set_width_chars(7);
    dtw->_rotation_status->set_numeric(false);
    dtw->_rotation_status->set_digits(2);
    dtw->_rotation_status->set_increments(1.0, 15.0);
    dtw->_rotation_status->set_update_policy(Gtk::UPDATE_ALWAYS);

    // Callbacks
    dtw->_rotation_status_output_connection = dtw->_rotation_status->signal_output().connect(sigc::mem_fun(dtw, &SPDesktopWidget::rotation_output));
    dtw->_rotation_status_value_changed_connection = dtw->_rotation_status->signal_value_changed().connect(sigc::mem_fun(dtw, &SPDesktopWidget::rotation_value_changed));
    dtw->_rotation_status_populate_popup_connection = dtw->_rotation_status->signal_populate_popup().connect(sigc::mem_fun(dtw, &SPDesktopWidget::rotation_populate_popup));

    // Style
    dtw->_rotation_status->set_name("RotationStatus");
    auto context_rotation = dtw->_rotation_status->get_style_context();
    context_rotation->add_provider(css_provider_spinbutton, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Cursor coordinates
    dtw->_coord_status = Gtk::manage(new Gtk::Grid());
    dtw->_coord_status->set_name("CoordinateAndZStatus");
    dtw->_coord_status->set_row_spacing(0);
    dtw->_coord_status->set_column_spacing(10);
    dtw->_coord_status->set_margin_end(10);
    auto sep = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL));
    sep->set_name("CoordinateSeparator");
    dtw->_coord_status->attach(*sep, 0, 0, 1, 2);

    dtw->_coord_status->set_tooltip_text(_("Cursor coordinates"));
    auto label_x = Gtk::manage(new Gtk::Label(_("X:")));
    auto label_y = Gtk::manage(new Gtk::Label(_("Y:")));
    label_x->set_halign(Gtk::ALIGN_START);
    label_y->set_halign(Gtk::ALIGN_START);
    dtw->_coord_status->attach(*label_x, 1, 0, 1, 1);
    dtw->_coord_status->attach(*label_y, 1, 1, 1, 1);
    dtw->_coord_status_x = Gtk::manage(new Gtk::Label());
    dtw->_coord_status_y = Gtk::manage(new Gtk::Label());
    dtw->_coord_status_x->set_name("CoordinateStatusX");
    dtw->_coord_status_y->set_name("CoordinateStatusY");
    dtw->_coord_status_x->set_markup("   0.00 ");
    dtw->_coord_status_y->set_markup("   0.00 ");

    // TRANSLATORS: Abbreviation for canvas zoom level
    auto label_z = Gtk::manage(new Gtk::Label(C_("canvas", "Z:")));
    label_z->set_name("ZLabel");
    // TRANSLATORS: Abbreviation for canvas rotation
    auto label_r = Gtk::manage(new Gtk::Label(C_("canvas", "R:")));
    label_r->set_name("RLabel");

    dtw->_coord_status_x->set_halign(Gtk::ALIGN_END);
    dtw->_coord_status_y->set_halign(Gtk::ALIGN_END);
    dtw->_coord_status->attach(*dtw->_coord_status_x, 2, 0, 1, 1);
    dtw->_coord_status->attach(*dtw->_coord_status_y, 2, 1, 1, 1);
    dtw->_coord_status->show_all();
    dtw->_coord_status->set_no_show_all();

    dtw->_zoom_status_box->pack_start(*label_z, true, true);
    dtw->_zoom_status_box->pack_end(*dtw->_zoom_status, true, true);
    dtw->_zoom_status_box->show_all();

    dtw->_rotation_status_box->pack_start(*label_r, true, true);
    dtw->_rotation_status_box->pack_end(*dtw->_rotation_status, true, true);
    dtw->_rotation_status_box->show_all();
    dtw->_rotation_status_box->set_no_show_all();

    dtw->_statusbar->pack_end(*dtw->_rotation_status_box, false, false);
    dtw->_statusbar->pack_end(*dtw->_zoom_status_box, false, false);
    dtw->_statusbar->pack_end(*dtw->_coord_status, false, false);

    update_statusbar_visibility();

    _statusbar_preferences_observer = prefs->createObserver("/statusbar/visibility", [=]() {
        update_statusbar_visibility();
    });

    // --------------- Color Management ---------------- //
    dtw->_tracker = ege_color_prof_tracker_new(GTK_WIDGET(_layer_selector->gobj()));
    bool fromDisplay = prefs->getBool( "/options/displayprofile/from_display");
    if ( fromDisplay ) {
        auto id = Inkscape::CMSSystem::getDisplayId(0);
        dtw->_canvas->set_cms_key(id);
        dtw->cms_adjust_set_sensitive(!id.empty());
    }
    g_signal_connect( G_OBJECT(dtw->_tracker), "changed", G_CALLBACK(SPDesktopWidget::color_profile_event), dtw );

    // ------------------ Finish Up -------------------- //
    dtw->_vbox->show_all();
    dtw->_canvas_grid->ShowCommandPalette(false);

    dtw->_canvas->grab_focus();
}

void SPDesktopWidget::apply_ctrlbar_settings() {
    Inkscape::Preferences* prefs = Inkscape::Preferences::get();
    int min = ToolboxFactory::min_pixel_size;
    int max = ToolboxFactory::max_pixel_size;
    int size = prefs->getIntLimited(ToolboxFactory::ctrlbars_icon_size, min, min, max);
    ToolboxFactory::set_icon_size(snap_toolbox, size);
    ToolboxFactory::set_icon_size(commands_toolbox, size);
    ToolboxFactory::set_icon_size(aux_toolbox, size);
}

void SPDesktopWidget::update_statusbar_visibility() {
    auto prefs = Inkscape::Preferences::get();
    Glib::ustring path("/statusbar/visibility/");

    _coord_status->set_visible(prefs->getBool(path + "coordinates", true));
    _rotation_status_box->set_visible(prefs->getBool(path + "rotation", true));
    _layer_selector->set_visible(prefs->getBool(path + "layer", true));
    _selected_style->set_visible(prefs->getBool(path + "style", true));
}

void
SPDesktopWidget::setMessage (Inkscape::MessageType type, const gchar *message)
{
    _select_status->set_markup(message ? message : "");

    // make sure the important messages are displayed immediately!
    if (type == Inkscape::IMMEDIATE_MESSAGE && _select_status->get_is_drawable()) {
        _select_status->queue_draw();
    }

    _select_status->set_tooltip_text(_select_status->get_text());
}

Geom::Point
SPDesktopWidget::window_get_pointer()
{
    int x, y;
    auto window = _canvas->get_window();
    auto display = window->get_display();
    auto seat = display->get_default_seat();
    auto device = seat->get_pointer();
    Gdk::ModifierType m;
    window->get_device_position(device, x, y, m);

    return Geom::Point(x, y);
}

/**
 * Called before SPDesktopWidget destruction.
 * (Might be called more than once)
 */
void
SPDesktopWidget::on_unrealize()
{
    auto dtw = this;

    if (_tbbox) {
        Inkscape::Preferences::get()->setInt("/toolbox/tools/width", _tbbox->get_position());
    }

    if (dtw->desktop) {
        if ( watcher ) {
            watcher->remove(dtw);
        }

        for (auto &conn : dtw->_connections) {
            conn.disconnect();
        }

        // Canvas
        dtw->_canvas->set_drawing(nullptr); // Ensures deactivation
        dtw->_canvas->set_desktop(nullptr); // Todo: Remove desktop dependency.

        // Zoom
        dtw->_zoom_status_input_connection.disconnect();
        dtw->_zoom_status_output_connection.disconnect();
        g_signal_handlers_disconnect_by_data(G_OBJECT(dtw->_zoom_status->gobj()), dtw->_zoom_status->gobj());
        dtw->_zoom_status_value_changed_connection.disconnect();
        dtw->_zoom_status_populate_popup_connection.disconnect();

        // Rotation
        dtw->_rotation_status_input_connection.disconnect();
        dtw->_rotation_status_output_connection.disconnect();
        g_signal_handlers_disconnect_by_data(G_OBJECT(dtw->_rotation_status->gobj()), dtw->_rotation_status->gobj());
        dtw->_rotation_status_value_changed_connection.disconnect();
        dtw->_rotation_status_populate_popup_connection.disconnect();

        dtw->_panels->setDesktop(nullptr);

        delete _container;

        _layer_selector->setDesktop(nullptr);
        INKSCAPE.remove_desktop(dtw->desktop); // clears selection and event_context
        dtw->modified_connection.disconnect();
        dtw->desktop->destroy();
        Inkscape::GC::release (dtw->desktop);
        dtw->desktop = nullptr;
    }

    parent_type::on_unrealize();
}

SPDesktopWidget::~SPDesktopWidget() {
    delete _canvas_grid;
}

/**
 * Set the title in the desktop-window (if desktop has an own window).
 *
 * The title has form file name: desktop number - Inkscape.
 * The desktop number is only shown if it's 2 or higher,
 */
void
SPDesktopWidget::updateTitle(gchar const* uri)
{
    if (window) {

        SPDocument *doc = this->desktop->doc();
        auto namedview = doc->getNamedView();

        std::string Name;
        if (doc->isModifiedSinceSave()) {
            Name += "*";
        }

        Name += uri;

        if (namedview->viewcount > 1) {
            Name += ": ";
            Name += std::to_string(namedview->viewcount);
        }
        Name += " (";

        auto render_mode = desktop->getCanvas()->get_render_mode();
        auto color_mode  = desktop->getCanvas()->get_color_mode();

        if (render_mode == Inkscape::RenderMode::OUTLINE) {
            Name += N_("outline");
        } else if (render_mode == Inkscape::RenderMode::NO_FILTERS) {
            Name += N_("no filters");
        } else if (render_mode == Inkscape::RenderMode::VISIBLE_HAIRLINES) {
            Name += N_("visible hairlines");
        } else if (render_mode == Inkscape::RenderMode::OUTLINE_OVERLAY) {
            Name += N_("outline overlay");
        }

        if (color_mode != Inkscape::ColorMode::NORMAL &&
            render_mode != Inkscape::RenderMode::NORMAL) {
                Name += ", ";
        }

        if (color_mode == Inkscape::ColorMode::GRAYSCALE) {
            Name += N_("grayscale");
        } else if (color_mode == Inkscape::ColorMode::PRINT_COLORS_PREVIEW) {
            Name += N_("print colors preview");
        }

        if (*Name.rbegin() == '(') {  // Can not use C++11 .back() or .pop_back() with ustring!
            Name.erase(Name.size() - 2);
        } else {
            Name += ")";
        }

        Name += " - Inkscape";

        // Name += " (";
        // Name += Inkscape::version_string;
        // Name += ")";

        window->set_title (Name);
    }
}

DialogContainer *SPDesktopWidget::getDialogContainer()
{
    return _container;
}

/**
 * Resize handler, keeps the desktop centered.
 */
void SPDesktopWidget::on_size_allocate(Gtk::Allocation &allocation)
{
    // This function is called a lot during mouse move events without
    // resizing the widget. Desktop position/zoom must not be updated
    // for these trivial invocations.
    if (allocation == get_allocation()) {
        parent_type::on_size_allocate(allocation);
        return;
    }

    Geom::Rect const d_canvas = _canvas->get_area_world();

    parent_type::on_size_allocate(allocation);

    if (d_canvas.hasZeroArea()) {
        return;
    }

    Geom::Point const midpoint_dt = desktop->w2d(d_canvas.midpoint());
    double zoom = desktop->current_zoom();

    if (_canvas_grid->GetStickyZoom()->get_active()) {
        /* Calculate adjusted zoom */
        double oldshortside = d_canvas.minExtent();
        double newshortside = _canvas->get_area_world().minExtent();
        zoom *= newshortside / oldshortside;
    }

    desktop->zoom_absolute(midpoint_dt, zoom, false);
}

/**
 * Callback to realize desktop widget.
 */
void SPDesktopWidget::on_realize()
{
    SPDesktopWidget *dtw = this;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    parent_type::on_realize();

    Geom::Rect d = Geom::Rect::from_xywh(Geom::Point(0,0), (dtw->desktop->doc())->getDimensions());

    if (d.width() < 1.0 || d.height() < 1.0) return;

    dtw->desktop->set_display_area (d, 10);

    dtw->updateNamedview();
    gchar *gtkThemeName;
    gboolean gtkApplicationPreferDarkTheme;
    GtkSettings *settings = gtk_settings_get_default();
    Gtk::Container *window = get_toplevel();
    if (settings && window) {
        g_object_get(settings, "gtk-theme-name", &gtkThemeName, nullptr);
        g_object_get(settings, "gtk-application-prefer-dark-theme", &gtkApplicationPreferDarkTheme, nullptr);
        bool dark = INKSCAPE.themecontext->isCurrentThemeDark(dynamic_cast<Gtk::Container *>(window));
        if (dark) {
            prefs->setBool("/theme/darkTheme", true);
            window->get_style_context()->add_class("dark");
            window->get_style_context()->remove_class("bright");
        } else {
            prefs->setBool("/theme/darkTheme", false);
            window->get_style_context()->add_class("bright");
            window->get_style_context()->remove_class("dark");
        }
        if (prefs->getBool("/theme/symbolicIcons", false)) {
            window->get_style_context()->add_class("symbolic");
            window->get_style_context()->remove_class("regular");
        } else {
            window->get_style_context()->add_class("regular");
            window->get_style_context()->remove_class("symbolic");
        }
        INKSCAPE.themecontext->getChangeThemeSignal().emit();
    }
}

/* This is just to provide access to common functionality from sp_desktop_widget_realize() above
   as well as from SPDesktop::change_document() */
void SPDesktopWidget::updateNamedview()
{
    // Listen on namedview modification
    // originally (prior to the sigc++ conversion) the signal was simply
    // connected twice rather than disconnecting the first connection
    modified_connection.disconnect();

    modified_connection = desktop->namedview->connectModified(sigc::mem_fun(*this, &SPDesktopWidget::namedviewModified));
    namedviewModified(desktop->namedview, SP_OBJECT_MODIFIED_FLAG);

    updateTitle( desktop->doc()->getDocumentName() );
}

/**
 * Callback to handle desktop widget event.
 */
gint
SPDesktopWidget::event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
    if (event->type == GDK_BUTTON_PRESS) {
        // defocus any spinbuttons
        dtw->_canvas->grab_focus();
    }

    if ((event->type == GDK_BUTTON_PRESS) && (event->button.button == 3)) {
        if (event->button.state & GDK_SHIFT_MASK) {
            dtw->desktop->getCanvasDrawing()->set_sticky(true);
        } else {
            dtw->desktop->getCanvasDrawing()->set_sticky(false);
        }
    }

    {
        // The key press/release events need to be passed to desktop handler explicitly,
        // because otherwise the event contexts only receive key events when the mouse cursor
        // is over the canvas. This redirection is only done for key events and only if there's no
        // current item on the canvas, because item events and all mouse events are caught
        // and passed on by the canvas acetate (I think). --bb

        if ((event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE)
            && !dtw->_canvas->get_current_canvas_item()) {
            return (gint)sp_desktop_root_handler (event, dtw->desktop);
        }
    }

    return FALSE;
}

void
SPDesktopWidget::color_profile_event(EgeColorProfTracker */*tracker*/, SPDesktopWidget *dtw)
{
    // Handle profile changes
    GdkWindow *window = dtw->get_window()->gobj();

    // Figure out the ID for the monitor
    auto display = gdk_display_get_default();
    auto monitor = gdk_display_get_monitor_at_window(display, window);

    int n_monitors = gdk_display_get_n_monitors(display);

    int monitorNum = -1;

    // Now loop through the set of monitors and figure out whether this monitor matches
    for (int i_monitor = 0; i_monitor < n_monitors; ++i_monitor) {
        auto monitor_at_index = gdk_display_get_monitor(display, i_monitor);
        if (monitor_at_index == monitor) monitorNum = i_monitor;
    }

    Glib::ustring id = Inkscape::CMSSystem::getDisplayId( monitorNum );
    dtw->_canvas->set_cms_key(id);
    dtw->cms_adjust_set_sensitive(!id.empty());
}

void
SPDesktopWidget::update_guides_lock()
{
    bool down = _canvas_grid->GetGuideLock()->get_active();

    auto doc  = desktop->getDocument();
    auto nv   = desktop->getNamedView();

    if ( down != nv->lockguides ) {
        nv->lockguides = down;
        sp_namedview_guides_toggle_lock(doc, nv);
        if (down) {
            setMessage (Inkscape::NORMAL_MESSAGE, _("Locked all guides"));
        } else {
            setMessage (Inkscape::NORMAL_MESSAGE, _("Unlocked all guides"));
        }
    }
}

void
SPDesktopWidget::cms_adjust_toggled()
{
    auto _cms_adjust = _canvas_grid->GetCmsAdjust();

    bool down = _cms_adjust->get_active();
    if ( down != _canvas->get_cms_active() ) {
        _canvas->set_cms_active(down);
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/options/displayprofile/enable", down);
        if (down) {
            setMessage (Inkscape::NORMAL_MESSAGE, _("Color-managed display is <b>enabled</b> in this window"));
        } else {
            setMessage (Inkscape::NORMAL_MESSAGE, _("Color-managed display is <b>disabled</b> in this window"));
        }
    }
}

void
SPDesktopWidget::cms_adjust_set_sensitive(bool enabled)
{
    _canvas_grid->GetCmsAdjust()->set_sensitive(enabled);
}

/**
 * \store dessktop position
 */
void SPDesktopWidget::storeDesktopPosition(bool store_maximize)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool maxed = desktop->is_maximized();
    bool full = desktop->is_fullscreen();
    // Don't store max/full when setting max/full (it's about to change)
    if (store_maximize) {
        prefs->setBool("/desktop/geometry/fullscreen", full);
        prefs->setBool("/desktop/geometry/maximized", maxed);
    }
    // Don't save geom for maximized, fullscreen or iconified windows.
    // It just tells you the current maximized size, which is not
    // as useful as whatever value it had previously.
    if (!desktop->is_iconified() && !maxed && !full) {
        gint w = -1;
        gint h, x, y;
        desktop->getWindowGeometry(x, y, w, h);
        g_assert(w != -1);
        prefs->setInt("/desktop/geometry/width", w);
        prefs->setInt("/desktop/geometry/height", h);
        prefs->setInt("/desktop/geometry/x", x);
        prefs->setInt("/desktop/geometry/y", y);
    }
}

void
SPDesktopWidget::enableInteraction()
{
  g_return_if_fail(_interaction_disabled_counter > 0);

  _interaction_disabled_counter--;

  if (_interaction_disabled_counter == 0) {
    this->set_sensitive();
  }
}

void
SPDesktopWidget::disableInteraction()
{
  if (_interaction_disabled_counter == 0) {
    this->set_sensitive(false);
  }

  _interaction_disabled_counter++;
}

void
SPDesktopWidget::setCoordinateStatus(Geom::Point p)
{
    gchar *cstr;
    cstr = g_strdup_printf("%7.2f", _dt2r * p[Geom::X]);
    _coord_status_x->set_markup(cstr);
    g_free(cstr);

    cstr = g_strdup_printf("%7.2f", _dt2r * p[Geom::Y]);
    _coord_status_y->set_markup(cstr);
    g_free(cstr);
}

void
SPDesktopWidget::letZoomGrabFocus()
{
    if (_zoom_status) _zoom_status->grab_focus();
}

void
SPDesktopWidget::getWindowGeometry (gint &x, gint &y, gint &w, gint &h)
{
    if (window)
    {
        window->get_size (w, h);
        window->get_position (x, y);
    }
}

void
SPDesktopWidget::setWindowPosition (Geom::Point p)
{
    if (window)
    {
        window->move (gint(round(p[Geom::X])), gint(round(p[Geom::Y])));
    }
}

void
SPDesktopWidget::setWindowSize (gint w, gint h)
{
    if (window)
    {
        window->set_default_size (w, h);
        window->resize (w, h);
    }
}

/**
 * \note transientizing does not work on windows; when you minimize a document
 * and then open it back, only its transient emerges and you cannot access
 * the document window. The document window must be restored by rightclicking
 * the taskbar button and pressing "Restore"
 */
void
SPDesktopWidget::setWindowTransient (void *p, int transient_policy)
{
    if (window)
    {
        GtkWindow *w = GTK_WINDOW(window->gobj());
        gtk_window_set_transient_for (GTK_WINDOW(p), w);

        /*
         * This enables "aggressive" transientization,
         * i.e. dialogs always emerging on top when you switch documents. Note
         * however that this breaks "click to raise" policy of a window
         * manager because the switched-to document will be raised at once
         * (so that its transients also could raise)
         */
        if (transient_policy == PREFS_DIALOGS_WINDOWS_AGGRESSIVE)
            // without this, a transient window not always emerges on top
            gtk_window_present (w);
    }
}

void
SPDesktopWidget::presentWindow()
{
    if (window)
        window->present();
}

bool SPDesktopWidget::showInfoDialog( Glib::ustring const &message )
{
    bool result = false;
    if (window)
    {
        Gtk::MessageDialog dialog(*window, message, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
        dialog.property_destroy_with_parent() = true;
        dialog.set_name("InfoDialog");
        dialog.set_title(_("Note:")); // probably want to take this as a parameter.
        dialog.run();
    }
    return result;
}

bool SPDesktopWidget::warnDialog (Glib::ustring const &text)
{
    Gtk::MessageDialog dialog (*window, text, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK_CANCEL);
    gint response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
        return true;
    else
        return false;
}

void
SPDesktopWidget::iconify()
{
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(_canvas->gobj())));
    if (GTK_IS_WINDOW(topw)) {
        if (desktop->is_iconified()) {
            gtk_window_deiconify(topw);
        } else {
            gtk_window_iconify(topw);
        }
    }
}

void
SPDesktopWidget::maximize()
{
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(_canvas->gobj())));
    if (GTK_IS_WINDOW(topw)) {
        if (desktop->is_maximized()) {
            gtk_window_unmaximize(topw);
        } else {
            storeDesktopPosition(false);
            gtk_window_maximize(topw);
        }
    }
}

void
SPDesktopWidget::fullscreen()
{
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(_canvas->gobj())));
    if (GTK_IS_WINDOW(topw)) {
        if (desktop->is_fullscreen()) {
            gtk_window_unfullscreen(topw);
            // widget layout is triggered by the resulting window_state_event
        } else {
            storeDesktopPosition(false);
            gtk_window_fullscreen(topw);
            // widget layout is triggered by the resulting window_state_event
        }
    }
}

/**
 * Hide whatever the user does not want to see in the window.
 * Also move command toolbar to top or side as required.
 */
void SPDesktopWidget::layoutWidgets()
{
    SPDesktopWidget *dtw = this;
    Glib::ustring pref_root;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (desktop && desktop->is_focusMode()) {
        pref_root = "/focus/";
    } else if (desktop && desktop->is_fullscreen()) {
        pref_root = "/fullscreen/";
    } else {
        pref_root = "/window/";
    }

    if (!prefs->getBool(pref_root + "commands/state", true)) {
        gtk_widget_hide (dtw->commands_toolbox);
    } else {
        gtk_widget_show_all (dtw->commands_toolbox);
    }

    if (!prefs->getBool(pref_root + "snaptoolbox/state", true)) {
        gtk_widget_hide (dtw->snap_toolbox);
    } else {
        gtk_widget_show_all (dtw->snap_toolbox);
    }

    if (!prefs->getBool(pref_root + "toppanel/state", true)) {
        gtk_widget_hide (dtw->aux_toolbox);
    } else {
        // we cannot just show_all because that will show all tools' panels;
        // this is a function from toolbox.cpp that shows only the current tool's panel
        ToolboxFactory::showAuxToolbox(dtw->aux_toolbox);
    }

    if (!prefs->getBool(pref_root + "toolbox/state", true)) {
        gtk_widget_hide (dtw->tool_toolbox);
    } else {
        gtk_widget_show_all (dtw->tool_toolbox);
    }

    if (!prefs->getBool(pref_root + "statusbar/state", true)) {
        dtw->_statusbar->hide();
    } else {
        dtw->_statusbar->show_all();
    }

    if (!prefs->getBool(pref_root + "panels/state", true)) {
        dtw->_panels->hide();
    } else {
        dtw->_panels->show_all();
    }

    _canvas_grid->ShowScrollbars(prefs->getBool(pref_root + "scrollbars/state", true));
    _canvas_grid->ShowRulers(    prefs->getBool(pref_root + "rulers/state",     true));

    // Move command toolbar as required.

    // If interface_mode unset, use screen aspect ratio. Needs to be synced with "canvas-interface-mode" action.
    Gdk::Rectangle monitor_geometry = Inkscape::UI::get_monitor_geometry_primary();
    double const width  = monitor_geometry.get_width();
    double const height = monitor_geometry.get_height();
    bool widescreen = (height > 0 && width/height > 1.65);
    widescreen = prefs->getInt(pref_root + "interface_mode", widescreen);

    auto commands_toolbox_cpp = dynamic_cast<Gtk::Bin *>(Glib::wrap(commands_toolbox));
    if (commands_toolbox_cpp) {

        // Unlink command toolbar.
        commands_toolbox_cpp->reference(); // So toolbox is not deleted.
        auto parent = commands_toolbox_cpp->get_parent();
        parent->remove(*commands_toolbox_cpp);

        auto orientation = Gtk::ORIENTATION_HORIZONTAL;
        auto orientation_c = GTK_ORIENTATION_HORIZONTAL;
        // Link command toolbar back.
        if (!widescreen) {
            _top_toolbars->attach(*commands_toolbox_cpp, 0, 0); // Always first
            gtk_box_set_child_packing(_vbox->gobj(), commands_toolbox, false, true, 0, GTK_PACK_START); // expand, fill, padding, pack_type
            orientation = Gtk::ORIENTATION_HORIZONTAL;
            orientation_c = GTK_ORIENTATION_HORIZONTAL;
            commands_toolbox_cpp->set_hexpand(true);
        } else {
            _hbox->add(*commands_toolbox_cpp);
            gtk_box_set_child_packing(_hbox->gobj(), commands_toolbox, false, true, 0, GTK_PACK_START); // expand, fill, padding, pack_type
            orientation = Gtk::ORIENTATION_VERTICAL;
            orientation_c = GTK_ORIENTATION_VERTICAL;
            commands_toolbox_cpp->set_hexpand(false);
        }
        commands_toolbox_cpp->unreference();

        auto box = dynamic_cast<Gtk::Box *>(commands_toolbox_cpp->get_child());
        if (box) {
            box->set_orientation(orientation);
            for (auto child : box->get_children()) {
                if (auto toolbar = dynamic_cast<Gtk::Toolbar *>(child)) {
                    gtk_orientable_set_orientation(GTK_ORIENTABLE(toolbar->gobj()), orientation_c);
                    //toolbar->set_orientation(orientation); // Missing in C++ interface!
                }
            }
        }
    } else {
        std::cerr << "SPDesktopWidget::layoutWidgets(): Wrong widget type for command toolbar!" << std::endl;
    }

    // Temporary for Gtk3: Gtk toolbar resets icon sizes, so reapply them.
    // TODO: remove this call in Gtk4 after Gtk::Toolbar is eliminated.
    apply_ctrlbar_settings();

    auto& snap = *Glib::wrap(snap_toolbox);
    auto& aux = *Glib::wrap(aux_toolbox);

    // This ensures that the Snap toolbox is on the top and only takes the needed space.
    if (_top_toolbars->get_children().size() == 3 && gtk_widget_get_visible(commands_toolbox)) {
        _top_toolbars->child_property_height(snap) =  1;
        _top_toolbars->child_property_width(aux) = 2;
        snap.set_valign(Gtk::ALIGN_START);
    }
    else {
        _top_toolbars->child_property_width(aux) = 1;
        _top_toolbars->child_property_height(snap) =  2;
        snap.set_valign(Gtk::ALIGN_CENTER);
    }

    Inkscape::UI::resize_widget_children(_top_toolbars);
}

Gtk::Toolbar *
SPDesktopWidget::get_toolbar_by_name(const Glib::ustring& name)
{
    // The name is actually attached to the GtkGrid that contains
    // the toolbar, so we need to get the grid first
    auto widget = sp_search_by_name_recursive(Glib::wrap(aux_toolbox), name);
    auto grid = dynamic_cast<Gtk::Grid*>(widget);

    if (!grid) return nullptr;

    auto child = grid->get_child_at(0,0);
    auto tb = dynamic_cast<Gtk::Toolbar*>(child);

    return tb;
}

void
SPDesktopWidget::setToolboxFocusTo (const gchar* label)
{
    // First try looking for a named widget
    auto hb = sp_search_by_name_recursive(Glib::wrap(aux_toolbox), label);

    // Fallback to looking for a named data member (deprecated)
    if (!hb) {
        hb = Glib::wrap(GTK_WIDGET(sp_search_by_data_recursive(aux_toolbox, (gpointer) label)));
    }

    if (hb)
    {
        hb->grab_focus();
    }
}

void
SPDesktopWidget::setToolboxAdjustmentValue (gchar const *id, double value)
{
    // First try looking for a named widget
    auto hb = sp_search_by_name_recursive(Glib::wrap(aux_toolbox), id);

    // Fallback to looking for a named data member (deprecated)
    if (!hb) {
        hb = Glib::wrap(GTK_WIDGET(sp_search_by_data_recursive(aux_toolbox, (gpointer)id)));
    }

    if (hb) {
        auto sb = dynamic_cast<Inkscape::UI::Widget::SpinButtonToolItem *>(hb);
        auto a = sb->get_adjustment();

        if(a) a->set_value(value);
    }

    else g_warning ("Could not find GtkAdjustment for %s\n", id);
}


bool
SPDesktopWidget::isToolboxButtonActive (const gchar* id)
{
    bool isActive = false;
    auto thing = sp_search_by_name_recursive(Glib::wrap(aux_toolbox), id);

    // The toolbutton could be a few different types so try casting to
    // each of them.
    // TODO: This will be simpler in Gtk+ 4 when ToolItems have gone
    auto toggle_button      = dynamic_cast<Gtk::ToggleButton *>(thing);
    auto toggle_tool_button = dynamic_cast<Gtk::ToggleToolButton *>(thing);

    if ( !thing ) {
        //g_message( "Unable to locate item for {%s}", id );
    } else if (toggle_button) {
        isActive = toggle_button->get_active();
    } else if (toggle_tool_button) {
        isActive = toggle_tool_button->get_active();
    } else {
        //g_message( "Item for {%s} is of an unsupported type", id );
    }

    return isActive;
}

SPDesktopWidget::SPDesktopWidget(InkscapeWindow *inkscape_window, SPDocument *document)
    : SPDesktopWidget(inkscape_window)
{
    set_name("SPDesktopWidget");

    SPDesktopWidget *dtw = this;

    SPNamedView *namedview = document->getNamedView();

    dtw->_dt2r = 1. / namedview->display_units->factor;

    dtw->_ruler_origin = Geom::Point(0,0); //namedview->gridorigin;   Why was the grid origin used here?

    // This section seems backwards!
    dtw->desktop = new SPDesktop();
    dtw->desktop->init (namedview, dtw->_canvas, this);
    dtw->_canvas->set_desktop(desktop);
    INKSCAPE.add_desktop (dtw->desktop);

    // Add the shape geometry to libavoid for autorouting connectors.
    // This needs desktop set for its spacing preferences.
    init_avoided_shape_geometry(dtw->desktop);

    dtw->_selected_style->setDesktop(dtw->desktop);

    /* Once desktop is set, we can update rulers */
    dtw->_canvas_grid->UpdateRulers();

    dtw->setView(dtw->desktop);

    /* Listen on namedview modification */
    dtw->modified_connection = namedview->connectModified(sigc::mem_fun(*dtw, &SPDesktopWidget::namedviewModified));

    _layer_selector->setDesktop(dtw->desktop);

    // We never want a page widget if there's no desktop.
    _page_selector = Gtk::manage(new Inkscape::UI::Widget::PageSelector(desktop));
    _statusbar->pack_end(*_page_selector, false, false);

    ToolboxFactory::setToolboxDesktop(dtw->aux_toolbox, dtw->desktop);

    dtw->layoutWidgets();

    dtw->_panels->setDesktop(dtw->desktop);
}


void
SPDesktopWidget::update_rulers()
{
    _canvas_grid->UpdateRulers();
}


void SPDesktopWidget::namedviewModified(SPObject *obj, guint flags)
{
    SPNamedView *nv=SP_NAMEDVIEW(obj);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        _dt2r = 1. / nv->display_units->factor;
        _ruler_origin = Geom::Point(0,0); //nv->gridorigin;   Why was the grid origin used here?

        _canvas_grid->GetVRuler()->set_unit(nv->getDisplayUnit());
        _canvas_grid->GetHRuler()->set_unit(nv->getDisplayUnit());
        _canvas_grid->GetVRuler()->set_tooltip_text(gettext(nv->display_units->name_plural.c_str()));
        _canvas_grid->GetHRuler()->set_tooltip_text(gettext(nv->display_units->name_plural.c_str()));
        _canvas_grid->UpdateRulers();

        /* This loops through all the grandchildren of aux toolbox,
         * and for each that it finds, it performs an sp_search_by_name_recursive(),
         * looking for widgets named "unit-tracker" (this is used by
         * all toolboxes to refer to the unit selector). The default document units
         * is then selected within these unit selectors.
         *
         * This should solve: https://bugs.launchpad.net/inkscape/+bug/362995
         */
        if (GTK_IS_CONTAINER(aux_toolbox)) {
            std::vector<Gtk::Widget*> ch = Glib::wrap(GTK_CONTAINER(aux_toolbox))->get_children();
            for (auto i:ch) {
                if (auto container = dynamic_cast<Gtk::Container *>(i)) {
                    std::vector<Gtk::Widget*> grch = container->get_children();
                    for (auto j:grch) {

                        if (!GTK_IS_WIDGET(j->gobj())) // wasn't a widget
                            continue;

                        // Don't apply to text toolbar. We want to be able to
                        // use different units for text. (Bug 1562217)
                        const Glib::ustring name = j->get_name();
                        if ( name == "TextToolbar" || name == "MeasureToolbar" || name == "CalligraphicToolbar" )
                            continue;

                        auto tracker = dynamic_cast<Inkscape::UI::Widget::ComboToolItem*>(sp_search_by_name_recursive(j, "unit-tracker"));

                        if (tracker) { // it's null when inkscape is first opened
                            if (auto ptr = static_cast<UnitTracker*>(tracker->get_data(Glib::Quark("unit-tracker")))) {
                                ptr->setActiveUnit(nv->display_units);
                            }
                        }
                    } // grandchildren
                } // if child is a container
            } // children
        } // if aux_toolbox is a container
    }
}

void
SPDesktopWidget::on_adjustment_value_changed()
{
    if (update)
        return;

    update = true;

    // Do not call canvas->scrollTo directly... messes up 'offset'.
    desktop->scroll_absolute( Geom::Point(_canvas_grid->GetHAdj()->get_value(),
                                          _canvas_grid->GetVAdj()->get_value()), false);

    update = false;
}

/* we make the desktop window with focus active, signal is connected in interface.c */
bool SPDesktopWidget::onFocusInEvent(GdkEventFocus*)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/bitmapautoreload/value", true)) {
        std::vector<SPObject *> imageList = (desktop->doc())->getResourceList("image");
        for (auto it : imageList) {
            SPImage* image = SP_IMAGE(it);
            image->refresh_if_outdated();
        }
    }

    INKSCAPE.activate_desktop (desktop);

    return false;
}

// ------------------------ Zoom ------------------------
static gdouble
sp_dtw_zoom_value_to_display (gdouble value)
{
    return floor (10 * (pow (2, value) * 100.0 + 0.05)) / 10;
}

static gdouble
sp_dtw_zoom_display_to_value (gdouble value)
{
    return  log (value / 100.0) / log (2);
}

int
SPDesktopWidget::zoom_input(double *new_val)
{
    double new_typed = g_strtod (_zoom_status->get_text().c_str(), nullptr);
    *new_val = sp_dtw_zoom_display_to_value (new_typed);
    return true;
}

bool
SPDesktopWidget::zoom_output()
{
    gchar b[64];
    double val = sp_dtw_zoom_value_to_display (_zoom_status->get_value());
    if (val < 10) {
        g_snprintf (b, 64, "%4.1f%%", val);
    } else {
        g_snprintf (b, 64, "%4.0f%%", val);
    }
    _zoom_status->set_text(b);
    return true;
}

void
SPDesktopWidget::zoom_value_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double const zoom_factor = pow (2, _zoom_status->get_value());

    // Zoom around center of window
    Geom::Rect const d_canvas = _canvas->get_area_world();
    Geom::Point midpoint = desktop->w2d(d_canvas.midpoint());

    _zoom_status_value_changed_connection.block();
    if(prefs->getDouble("/options/zoomcorrection/shown", true)) {
        desktop->zoom_realworld(midpoint, zoom_factor);
    } else {
        desktop->zoom_absolute(midpoint, zoom_factor);
    }
    _zoom_status_value_changed_connection.unblock();
    _zoom_status->defocus();
}

void
SPDesktopWidget::zoom_menu_handler(double factor)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if(prefs->getDouble("/options/zoomcorrection/shown", true)) {
        desktop->zoom_realworld(desktop->current_center(), factor);
    } else {
        desktop->zoom_absolute(desktop->current_center(), factor, false);
    }
}



void
SPDesktopWidget::zoom_populate_popup(Gtk::Menu *menu)
{
    for ( auto iter : menu->get_children()) {
        menu->remove(*iter);
    }

    auto item_1000 = Gtk::manage(new Gtk::MenuItem("1000%"));
    auto item_500  = Gtk::manage(new Gtk::MenuItem("500%"));
    auto item_200  = Gtk::manage(new Gtk::MenuItem("200%"));
    auto item_100  = Gtk::manage(new Gtk::MenuItem("100%"));
    auto item_50   = Gtk::manage(new Gtk::MenuItem( "50%"));
    auto item_25   = Gtk::manage(new Gtk::MenuItem( "25%"));
    auto item_10   = Gtk::manage(new Gtk::MenuItem( "10%"));

    item_1000->signal_activate().connect(sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler), 10.00));
    item_500->signal_activate().connect( sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler),  5.00));
    item_200->signal_activate().connect( sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler),  2.00));
    item_100->signal_activate().connect( sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler),  1.00));
    item_50->signal_activate().connect(  sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler),  0.50));
    item_25->signal_activate().connect(  sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler),  0.25));
    item_10->signal_activate().connect(  sigc::bind(sigc::mem_fun(this, &SPDesktopWidget::zoom_menu_handler),  0.10));

    menu->append(*item_1000);
    menu->append(*item_500);
    menu->append(*item_200);
    menu->append(*item_100);
    menu->append(*item_50);
    menu->append(*item_25);
    menu->append(*item_10);

    auto sep = Gtk::manage(new Gtk::SeparatorMenuItem());
    menu->append(*sep);

    auto item_page = Gtk::manage(new Gtk::MenuItem(_("Page")));
    item_page->signal_activate().connect([=]() { desktop->getDocument()->getPageManager().zoomToSelectedPage(desktop); });
    menu->append(*item_page);

    auto item_drawing = Gtk::manage(new Gtk::MenuItem(_("Drawing")));
    item_drawing->signal_activate().connect(sigc::mem_fun(desktop, &SPDesktop::zoom_drawing));
    menu->append(*item_drawing);

    auto item_selection = Gtk::manage(new Gtk::MenuItem(_("Selection")));
    item_selection->signal_activate().connect(sigc::mem_fun(desktop, &SPDesktop::zoom_selection));
    menu->append(*item_selection);

    auto item_center_page = Gtk::manage(new Gtk::MenuItem(_("Centre Page")));
    item_center_page->signal_activate().connect([=]() { desktop->getDocument()->getPageManager().centerToSelectedPage(desktop); });
    menu->append(*item_center_page);

    menu->show_all();
}


void
SPDesktopWidget::sticky_zoom_toggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/options/stickyzoom/value", _canvas_grid->GetStickyZoom()->get_active());
}


void
SPDesktopWidget::update_zoom()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    // It's very important that the value used in set_value is the same as the one
    // set as it otherwise creates an infinate loop between the spin button and update_zoom
    double correction = 1.0;
    if(prefs->getDouble("/options/zoomcorrection/shown", true)) {
        correction = prefs->getDouble("/options/zoomcorrection/value", 1.0);
    }
    _zoom_status_value_changed_connection.block();
    _zoom_status->set_value(log(desktop->current_zoom() / correction) / log(2));
    _zoom_status->queue_draw();
    _zoom_status_value_changed_connection.unblock();
}


// ---------------------- Rotation ------------------------

bool
SPDesktopWidget::rotation_output()
{
    gchar b[64];
    double val = _rotation_status->get_value();

    if (val < -180) val += 360;
    if (val >  180) val -= 360;

    g_snprintf (b, 64, "%7.2f°", val);

    _rotation_status->set_text(b);
    return true;
}

void
SPDesktopWidget::rotation_value_changed()
{
    double const rotate_factor = M_PI / 180.0 * _rotation_status->get_value();
    // std::cout << "SPDesktopWidget::rotation_value_changed: "
    //           << _rotation_status->get_value()
    //           << "  (" << rotate_factor << ")" <<std::endl;

    // Rotate around center of window
    Geom::Rect const d_canvas = _canvas->get_area_world();
    _rotation_status_value_changed_connection.block();
    Geom::Point midpoint = desktop->w2d(d_canvas.midpoint());
    desktop->rotate_absolute_center_point (midpoint, rotate_factor);
    _rotation_status_value_changed_connection.unblock();

    _rotation_status->defocus();
}

void
SPDesktopWidget::rotation_populate_popup(Gtk::Menu *menu)
{
    for ( auto iter : menu->get_children()) {
        menu->remove(*iter);
    }

    auto item_m135 = Gtk::manage(new Gtk::MenuItem("-135°"));
    auto item_m90  = Gtk::manage(new Gtk::MenuItem( "-90°"));
    auto item_m45  = Gtk::manage(new Gtk::MenuItem( "-45°"));
    auto item_0    = Gtk::manage(new Gtk::MenuItem(   "0°"));
    auto item_p45  = Gtk::manage(new Gtk::MenuItem(  "45°"));
    auto item_p90  = Gtk::manage(new Gtk::MenuItem(  "90°"));
    auto item_p135 = Gtk::manage(new Gtk::MenuItem( "135°"));
    auto item_p180 = Gtk::manage(new Gtk::MenuItem( "180°"));

    item_m135->signal_activate().connect(sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value), -135));
    item_m90->signal_activate().connect( sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value), -90));
    item_m45->signal_activate().connect( sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value), -45));
    item_0->signal_activate().connect(   sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value),   0));
    item_p45->signal_activate().connect( sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value),  45));
    item_p90->signal_activate().connect( sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value),  90));
    item_p135->signal_activate().connect(sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value), 135));
    item_p180->signal_activate().connect(sigc::bind(sigc::mem_fun(_rotation_status, &Gtk::SpinButton::set_value), 180));

    menu->append(*item_m135);
    menu->append(*item_m90);
    menu->append(*item_m45);
    menu->append(*item_0);
    menu->append(*item_p45);
    menu->append(*item_p90);
    menu->append(*item_p135);
    menu->append(*item_p180);

    menu->show_all();
}


void
SPDesktopWidget::update_rotation()
{
    _rotation_status_value_changed_connection.block();
    _rotation_status->set_value(desktop->current_rotation() / M_PI * 180.0);
    _rotation_status->queue_draw();
    _rotation_status_value_changed_connection.unblock();

}


// --------------- Rulers/Scrollbars/Etc. -----------------
void
SPDesktopWidget::toggle_command_palette() {
    // TODO: Turn into action and remove this function.
    _canvas_grid->ToggleCommandPalette();
}

void
SPDesktopWidget::toggle_rulers()
{
    // TODO: Turn into action and remove this function.
    _canvas_grid->ToggleRulers();
}

void
SPDesktopWidget::toggle_scrollbars()
{
    // TODO: Turn into action and remove this function.
    _canvas_grid->ToggleScrollbars();
}

bool
SPDesktopWidget::get_color_prof_adj_enabled() const
{
    auto _cms_adjust = _canvas_grid->GetCmsAdjust();
    return _cms_adjust->get_sensitive() && _cms_adjust->get_active();
}

void
SPDesktopWidget::toggle_color_prof_adj()
{
    auto _cms_adjust = _canvas_grid->GetCmsAdjust();
    if (_cms_adjust->get_sensitive()) {
        bool active = _cms_adjust->get_active();
        _cms_adjust->set_active(!active);
    }
}

static void
set_adjustment (Glib::RefPtr<Gtk::Adjustment> &adj, double l, double u, double ps, double si, double pi)
{
    if ((l != adj->get_lower()) ||
        (u != adj->get_upper()) ||
        (ps != adj->get_page_size()) ||
        (si != adj->get_step_increment()) ||
        (pi != adj->get_page_increment())) {
	    adj->set_lower(l);
	    adj->set_upper(u);
	    adj->set_page_size(ps);
	    adj->set_step_increment(si);
	    adj->set_page_increment(pi);
    }
}

void
SPDesktopWidget::update_scrollbars(double scale)
{
    if (update) return;
    update = true;

    /* The desktop region we always show unconditionally */
    SPDocument *doc = desktop->doc();

    auto deskarea = doc->preferredBounds();
    deskarea->expandBy(doc->getDimensions()); // Double size

    /* The total size of pages should be added unconditionally */
    deskarea->unionWith(doc->getPageManager().getDesktopRect());

    if (Inkscape::Preferences::get()->getInt("/tools/bounding_box") == 0) {
        deskarea->unionWith(doc->getRoot()->desktopVisualBounds());
    } else {
        deskarea->unionWith(doc->getRoot()->desktopGeometricBounds());
    }

    /* Canvas region we always show unconditionally */
    double const y_dir = desktop->yaxisdir();
    Geom::Rect carea( Geom::Point(deskarea->left()  * scale - 64, (deskarea->top()    * scale + 64) * y_dir),
                      Geom::Point(deskarea->right() * scale + 64, (deskarea->bottom() * scale - 64) * y_dir) );

    Geom::Rect viewbox = _canvas->get_area_world();

    /* Viewbox is always included into scrollable region */
    carea = Geom::unify(carea, viewbox);

    auto _hadj = _canvas_grid->GetHAdj();
    auto _vadj = _canvas_grid->GetVAdj();
    set_adjustment(_hadj, carea.min()[Geom::X], carea.max()[Geom::X],
                   viewbox.dimensions()[Geom::X],
                   0.1 * viewbox.dimensions()[Geom::X],
                   viewbox.dimensions()[Geom::X]);
    _hadj->set_value(viewbox.min()[Geom::X]);

    set_adjustment(_vadj, carea.min()[Geom::Y], carea.max()[Geom::Y],
                   viewbox.dimensions()[Geom::Y],
                   0.1 * viewbox.dimensions()[Geom::Y],
                   viewbox.dimensions()[Geom::Y]);
    _vadj->set_value(viewbox.min()[Geom::Y]);

    update = false;
}

gint
SPDesktopWidget::ruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw, bool horiz)
{
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        dtw->on_ruler_box_button_press_event(&event->button, Glib::wrap(widget), horiz);
        break;
    case GDK_MOTION_NOTIFY:
        dtw->on_ruler_box_motion_notify_event(&event->motion, Glib::wrap(widget), horiz);
        break;
    case GDK_BUTTON_RELEASE:
        dtw->on_ruler_box_button_release_event(&event->button, Glib::wrap(widget), horiz);
        break;
    default:
            break;
    }

    return FALSE;
}

bool
SPDesktopWidget::on_ruler_box_motion_notify_event(GdkEventMotion *event, Gtk::Widget *widget, bool horiz)
{
    if (horiz) {
        sp_event_context_snap_delay_handler(desktop->event_context, (gpointer) widget->gobj(), (gpointer) this, event, Inkscape::UI::Tools::DelayedSnapEvent::GUIDE_HRULER);
    }
    else {
        sp_event_context_snap_delay_handler(desktop->event_context, (gpointer) widget->gobj(), (gpointer) this, event, Inkscape::UI::Tools::DelayedSnapEvent::GUIDE_VRULER);
    }

    int wx, wy;

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(_canvas->gobj()));

    gint width, height;

    gdk_window_get_device_position(window, event->device, &wx, &wy, nullptr);
    gdk_window_get_geometry(window, nullptr /*x*/, nullptr /*y*/, &width, &height);

    Geom::Point const event_win(wx, wy);

    if (_ruler_clicked) {
        Geom::Point event_w(_canvas->canvas_to_world(event_win));
        Geom::Point event_dt(desktop->w2d(event_w));

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
        if ( ( abs( (gint) event->x - _xp ) < tolerance )
                && ( abs( (gint) event->y - _yp ) < tolerance ) ) {
            return false;
        }

        _ruler_dragged = true;

        // explicitly show guidelines; if I draw a guide, I want them on
        if ((horiz ? wy : wx) >= 0) {
            desktop->namedview->setGuides(true);
        }

        if (!(event->state & GDK_SHIFT_MASK)) {
            ruler_snap_new_guide(desktop, event_dt, _normal);
        }
        _active_guide->set_normal(_normal);
        _active_guide->set_origin(event_dt);

        desktop->set_coordinate_status(event_dt);
    }

    return false;
}

// End guide creation or toggle guides on/off.
bool
SPDesktopWidget::on_ruler_box_button_release_event(GdkEventButton *event, Gtk::Widget *widget, bool horiz)
{
    int wx, wy;

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(_canvas->gobj()));

    gint width, height;

    gdk_window_get_device_position(window, event->device, &wx, &wy, nullptr);
    gdk_window_get_geometry(window, nullptr /*x*/, nullptr /*y*/, &width, &height);

    Geom::Point const event_win(wx, wy);

    if (_ruler_clicked && event->button == 1) {
        desktop->event_context->discard_delayed_snap_event();

        auto seat = gdk_device_get_seat(event->device);
        gdk_seat_ungrab(seat);

        Geom::Point const event_w(_canvas->canvas_to_world(event_win));
        Geom::Point event_dt(desktop->w2d(event_w));

        if (!(event->state & GDK_SHIFT_MASK)) {
            ruler_snap_new_guide(desktop, event_dt, _normal);
        }

        delete _active_guide;
        _active_guide = nullptr;
        if ((horiz ? wy : wx) >= 0) {
            Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
            Inkscape::XML::Node *repr = xml_doc->createElement("sodipodi:guide");

            // If root viewBox set, interpret guides in terms of viewBox (90/96)
            double newx = event_dt.x();
            double newy = event_dt.y();

            // <sodipodi:guide> stores inverted y-axis coordinates
            if (desktop->is_yaxisdown()) {
                newy = desktop->doc()->getHeight().value("px") - newy;
                _normal[Geom::Y] *= -1.0;
            }

            SPRoot *root = desktop->doc()->getRoot();
            if( root->viewBox_set ) {
                newx = newx * root->viewBox.width()  / root->width.computed;
                newy = newy * root->viewBox.height() / root->height.computed;
            }
            repr->setAttributePoint("position", Geom::Point( newx, newy ));
            repr->setAttributePoint("orientation", _normal);
            desktop->namedview->appendChild(repr);
            Inkscape::GC::release(repr);
            DocumentUndo::done(desktop->getDocument(), _("Create guide"), "");
        }
        desktop->set_coordinate_status(event_dt);

        if (!_ruler_dragged) {
            // Ruler click (without drag) toggle the guide visibility on and off
            sp_namedview_toggle_guides(desktop->getDocument(), desktop->namedview);
        }

        _ruler_clicked = false;
        _ruler_dragged = false;
    }

    return false;
}

// Start guide creation by dragging from ruler.
bool
SPDesktopWidget::on_ruler_box_button_press_event(GdkEventButton *event, Gtk::Widget *widget, bool horiz)
{
    if (_ruler_clicked) // event triggered on a double click: do no process the click
        return false;

    int wx, wy;

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(_canvas->gobj()));

    gint width, height;

    gdk_window_get_device_position(window, event->device, &wx, &wy, nullptr);
    gdk_window_get_geometry(window, nullptr /*x*/, nullptr /*y*/, &width, &height);

    Geom::Point const event_win(wx, wy);

    if (event->button == 1) {
        _ruler_clicked = true;
        _ruler_dragged = false;
        // save click origin
        _xp = (gint) event->x;
        _yp = (gint) event->y;

        Geom::Point const event_w(_canvas->canvas_to_world(event_win));
        Geom::Point const event_dt(desktop->w2d(event_w));

        // calculate the normal of the guidelines when dragged from the edges of rulers.
        auto const y_dir = desktop->yaxisdir();
        Geom::Point normal_bl_to_tr(1., y_dir); //bottomleft to topright
        Geom::Point normal_tr_to_bl(-1., y_dir); //topright to bottomleft
        normal_bl_to_tr.normalize();
        normal_tr_to_bl.normalize();
        Inkscape::CanvasGrid * grid = sp_namedview_get_first_enabled_grid(desktop->namedview);
        if (grid){
            if (grid->getGridType() == Inkscape::GRID_AXONOMETRIC ) {
                Inkscape::CanvasAxonomGrid *axonomgrid = dynamic_cast<Inkscape::CanvasAxonomGrid *>(grid);
                if (event->state & GDK_CONTROL_MASK) {
                    // guidelines normal to gridlines
                    normal_bl_to_tr = Geom::Point::polar(-axonomgrid->angle_rad[0], 1.0);
                    normal_tr_to_bl = Geom::Point::polar(axonomgrid->angle_rad[2], 1.0);
                } else {
                    normal_bl_to_tr = rot90(Geom::Point::polar(axonomgrid->angle_rad[2], 1.0));
                    normal_tr_to_bl = rot90(Geom::Point::polar(-axonomgrid->angle_rad[0], 1.0));
                }
            }
        }
        if (horiz) {
            if (wx < 50) {
                _normal = normal_bl_to_tr;
            } else if (wx > width - 50) {
                _normal = normal_tr_to_bl;
            } else {
                _normal = Geom::Point(0.,1.);
            }
        } else {
            if (wy < 50) {
                _normal = normal_bl_to_tr;
            } else if (wy > height - 50) {
                _normal = normal_tr_to_bl;
            } else {
                _normal = Geom::Point(1.,0.);
            }
        }

        _active_guide = new Inkscape::CanvasItemGuideLine(desktop->getCanvasGuides(), Glib::ustring(), event_dt, _normal);
        _active_guide->set_stroke(desktop->namedview->guidehicolor);

        // Ruler grabs all events until button release.
        auto window = widget->get_window()->gobj();
        auto seat = gdk_device_get_seat(event->device);
        gdk_seat_grab(seat,
                window,
                GDK_SEAT_CAPABILITY_ALL_POINTING,
                FALSE,
                nullptr,
                (GdkEvent*)event,
                nullptr,
                nullptr);
    }

    return false;
}

void
SPDesktopWidget::ruler_snap_new_guide(SPDesktop *desktop, Geom::Point &event_dt, Geom::Point &normal)
{
    desktop->getCanvas()->grab_focus();
    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop);
    // We're dragging a brand new guide, just pulled of the rulers seconds ago. When snapping to a
    // path this guide will change it slope to become either tangential or perpendicular to that path. It's
    // therefore not useful to try tangential or perpendicular snapping, so this will be disabled temporarily
    bool pref_perp = m.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_PATH_PERPENDICULAR);
    bool pref_tang = m.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_PATH_TANGENTIAL);
    m.snapprefs.setTargetSnappable(Inkscape::SNAPTARGET_PATH_PERPENDICULAR, false);
    m.snapprefs.setTargetSnappable(Inkscape::SNAPTARGET_PATH_TANGENTIAL, false);
    // We only have a temporary guide which is not stored in our document yet.
    // Because the guide snapper only looks in the document for guides to snap to,
    // we don't have to worry about a guide snapping to itself here
    Geom::Point normal_orig = normal;
    m.guideFreeSnap(event_dt, normal, false, false);
    // After snapping, both event_dt and normal have been modified accordingly; we'll take the normal (of the
    // curve we snapped to) to set the normal the guide. And rotate it by 90 deg. if needed
    if (pref_perp) { // Perpendicular snapping to paths is requested by the user, so let's do that
        if (normal != normal_orig) {
            normal = Geom::rot90(normal);
        }
    }
    if (!(pref_tang || pref_perp)) { // if we don't want to snap either perpendicularly or tangentially, then
        normal = normal_orig; // we must restore the normal to it's original state
    }
    // Restore the preferences
    m.snapprefs.setTargetSnappable(Inkscape::SNAPTARGET_PATH_PERPENDICULAR, pref_perp);
    m.snapprefs.setTargetSnappable(Inkscape::SNAPTARGET_PATH_TANGENTIAL, pref_tang);
    m.unSetup();
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
