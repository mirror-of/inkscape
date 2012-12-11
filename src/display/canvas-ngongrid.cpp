/*
 * Authors:
 *    Matthew Woehlke <mw_triad@users.sourceforge.net>
 *    Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2006-2012 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

 /*
  * Current limits are: one axis (y-axis) is always vertical. The other two
  * axes are bound to a certain range of angles. The z-axis always has an angle
  * smaller than 90 degrees (measured from horizontal, 0 degrees being a line extending
  * to the right). The x-axis will always have an angle between 0 and 90 degrees.
  */


#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <glibmm/i18n.h>

#include "display/canvas-ngongrid.h"

#include "ui/widget/registered-widget.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "display/cairo-utils.h"
#include "display/canvas-grid.h"
#include "display/sp-canvas-util.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "inkscape.h"
#include "preferences.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "svg/svg-color.h"
#include "2geom/line.h"
#include "2geom/angle.h"
#include "util/mathfns.h"
#include "round.h"
#include "helper/units.h"


/**
 * This function calls Cairo to render a line on a particular canvas buffer.
 * Coordinates are interpreted as SCREENcoordinates
 */
static void
sp_cngongrid_drawline (SPCanvasBuf *buf, gdouble x0, gdouble y0, gdouble x1, gdouble y1, guint32 rgba)
{
    // Prevent aliasing of horizontal/vertical lines
    if (Geom::are_near(x0, x1)) {
        x0 = round(x0);
        x1 = round(x1);
    }
    if (Geom::are_near(y0, y1)) {
        y0 = round(y0);
        y1 = round(y1);
    }
    //TODO: clip to viewport?
    cairo_move_to(buf->ct, 0.5 + x0, 0.5 + y0);
    cairo_line_to(buf->ct, 0.5 + x1, 0.5 + y1);
    ink_cairo_set_source_rgba32(buf->ct, rgba);
    cairo_stroke(buf->ct);
}

static gdouble
distance(gdouble x, gdouble y, double dx, double dy)
{
    return (dy * x) - (dx * y);
}

static gdouble
find_bound(Geom::Rect const &rect, double dx, double dy, gdouble const & (*bound_func)(gdouble const &, gdouble const &))
{
    // Note: Y+ is DOWN, not up!
    if ( (dx > 0.0) == (dy > 0.0) ) { // Interesting points are bottom-left, top-right
        gdouble const a = distance(rect.left(), rect.bottom(), dx, dy);
        gdouble const b = distance(rect.right(), rect.top(), dx, dy);
        return bound_func(a, b);
    } else { // Interesting points are top-left, bottom-right
        gdouble const a = distance(rect.left(), rect.top(), dx, dy);
        gdouble const b = distance(rect.right(), rect.bottom(), dx, dy);
        return bound_func(a, b);
    }
}

namespace Inkscape {


/**
* A DIRECT COPY-PASTE FROM DOCUMENT-PROPERTIES.CPP  TO QUICKLY GET RESULTS
*
 * Helper function that attachs widgets in a 3xn table. The widgets come in an
 * array that has two entries per table row. The two entries code for four
 * possible cases: (0,0) means insert space in first column; (0, non-0) means
 * widget in columns 2-3; (non-0, 0) means label in columns 1-3; and
 * (non-0, non-0) means two widgets in columns 2 and 3.
**/
#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 10
static inline void
attach_all(Gtk::Table &table, Gtk::Widget const *const arr[], unsigned size, int start = 0)
{
    for (unsigned i=0, r=start; i<size/sizeof(Gtk::Widget*); i+=2) {
        if (arr[i] && arr[i+1]) {
            table.attach (const_cast<Gtk::Widget&>(*arr[i]),   1, 2, r, r+1,
                          Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 2, 3, r, r+1,
                          Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        } else {
            if (arr[i+1]) {
                table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 1, 3, r, r+1,
                              Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            } else if (arr[i]) {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&> (const_cast<Gtk::Widget&>(*arr[i]));
                label.set_alignment (0.0);
                table.attach (label, 0, 3, r, r+1,
                              Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            } else {
                Gtk::HBox *space = manage (new Gtk::HBox);
                space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);
                table.attach (*space, 0, 1, r, r+1,
                              (Gtk::AttachOptions)0, (Gtk::AttachOptions)0,0,0);
            }
        }
        ++r;
    }
}

CanvasNGonGrid::CanvasNGonGrid (SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc)
    : CanvasGrid(nv, in_repr, in_doc, GRID_POLYGONAL)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gridunit = sp_unit_get_by_abbreviation( prefs->getString("/options/grids/ngon/units").data() );
    if (!gridunit)
        gridunit = &sp_unit_get_by_id(SP_UNIT_PX);
    origin[Geom::X] = sp_units_get_pixels( prefs->getDouble("/options/grids/ngon/origin_x", 0.0), *gridunit );
    origin[Geom::Y] = sp_units_get_pixels( prefs->getDouble("/options/grids/ngon/origin_y", 0.0), *gridunit );
    sections = prefs->getInt("/options/grids/ngon/sect_n", 8);
    lengthx = sp_units_get_pixels( prefs->getDouble("/options/grids/ngon/spacing_x", 1.0), *gridunit );
    lengthy = sp_units_get_pixels( prefs->getDouble("/options/grids/ngon/spacing_y", 1.0), *gridunit );
    angle_deg = prefs->getDouble("/options/grids/ngon/rotation", 0.0);
    color = prefs->getInt("/options/grids/ngon/color", 0x0000ff20);
    empcolor = prefs->getInt("/options/grids/ngon/empcolor", 0x0000ff40);
    empspacing = prefs->getInt("/options/grids/ngon/empspacing", 5);

    angle_rad = Geom::deg_to_rad(angle_deg);
    se_angle_deg = 180.0 / sections;
    se_angle_rad = Geom::deg_to_rad(se_angle_deg);
    se_tan = tan(se_angle_rad);

    snapper = new CanvasNGonGridSnapper(this, &namedview->snap_manager, 0);

    if (repr) readRepr();
}

CanvasNGonGrid::~CanvasNGonGrid ()
{
    if (snapper) delete snapper;
}


/* fixme: Collect all these length parsing methods and think common sane API */

static gboolean sp_nv_read_length(gchar const *str, guint base, gdouble *val, SPUnit const **unit)
{
    if (!str) {
        return FALSE;
    }

    gchar *u;
    gdouble v = g_ascii_strtod(str, &u);
    if (!u) {
        return FALSE;
    }
    while (isspace(*u)) {
        u += 1;
    }

    if (!*u) {
        /* No unit specified - keep default */
        *val = v;
        return TRUE;
    }

    if (base & SP_UNIT_DEVICE) {
        if (u[0] && u[1] && !isalnum(u[2]) && !strncmp(u, "px", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_PX);
            *val = v;
            return TRUE;
        }
    }

    if (base & SP_UNIT_ABSOLUTE) {
        if (!strncmp(u, "pt", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_PT);
        } else if (!strncmp(u, "mm", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_MM);
        } else if (!strncmp(u, "cm", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_CM);
        } else if (!strncmp(u, "m", 1)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_M);
        } else if (!strncmp(u, "in", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_IN);
        } else if (!strncmp(u, "ft", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_FT);
        } else if (!strncmp(u, "pc", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_PC);
        } else {
            return FALSE;
        }
        *val = v;
        return TRUE;
    }

    return FALSE;
}

static gboolean sp_nv_read_opacity(gchar const *str, guint32 *color)
{
    if (!str) {
        return FALSE;
    }

    gchar *u;
    gdouble v = g_ascii_strtod(str, &u);
    if (!u) {
        return FALSE;
    }
    v = CLAMP(v, 0.0, 1.0);

    *color = (*color & 0xffffff00) | (guint32) floor(v * 255.9999);

    return TRUE;
}



void
CanvasNGonGrid::readRepr()
{
    gchar const *value;
    if ( (value = repr->attribute("originx")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &origin[Geom::X], &gridunit);
        origin[Geom::X] = sp_units_get_pixels(origin[Geom::X], *(gridunit));
    }
    if ( (value = repr->attribute("originy")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &origin[Geom::Y], &gridunit);
        origin[Geom::Y] = sp_units_get_pixels(origin[Geom::Y], *(gridunit));
    }

    if ( (value = repr->attribute("sections")) ) {
      sections = atoi(value);
      if (sections < 3) sections = 3;
      if (sections > 360) sections = 360;
      se_angle_deg = 180.0 / sections;
      se_angle_rad = Geom::deg_to_rad(se_angle_deg);
      se_tan = tan(se_angle_rad);
    }

    if ( (value = repr->attribute("spacingx")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &lengthx, &gridunit);
        lengthx = sp_units_get_pixels(lengthx, *(gridunit));
        if (lengthx < 0.0500) lengthx = 0.0500;
    }

    if ( (value = repr->attribute("spacingy")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &lengthy, &gridunit);
        lengthy = sp_units_get_pixels(lengthy, *(gridunit));
        if (lengthy < 0.0500) lengthy = 0.0500;
    }

    if ( (value = repr->attribute("rotation")) ) {
        angle_deg = g_ascii_strtod(value, NULL);
        double max_angle = 360.0 / sections;
        if (fabs(angle_deg) > max_angle) angle_deg = fmod(angle_deg, max_angle);
        angle_rad = Geom::deg_to_rad(angle_deg);
    }

    if ( (value = repr->attribute("color")) ) {
        color = (color & 0xff) | sp_svg_read_color(value, color);
    }

    if ( (value = repr->attribute("empcolor")) ) {
        empcolor = (empcolor & 0xff) | sp_svg_read_color(value, empcolor);
    }

    if ( (value = repr->attribute("opacity")) ) {
        sp_nv_read_opacity(value, &color);
    }
    if ( (value = repr->attribute("empopacity")) ) {
        sp_nv_read_opacity(value, &empcolor);
    }

    if ( (value = repr->attribute("empspacing")) ) {
        empspacing = atoi(value);
    }

    if ( (value = repr->attribute("visible")) ) {
        visible = (strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    if ( (value = repr->attribute("enabled")) ) {
        g_assert(snapper != NULL);
        snapper->setEnabled(strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    if ( (value = repr->attribute("snapvisiblegridlinesonly")) ) {
        g_assert(snapper != NULL);
        snapper->setSnapVisibleOnly(strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    for (GSList *l = canvasitems; l != NULL; l = l->next) {
        sp_canvas_item_request_update ( SP_CANVAS_ITEM(l->data) );
    }
    return;
}

/**
 * Called when XML node attribute changed; updates dialog widgets if change was not done by widgets themselves.
 */
void
CanvasNGonGrid::onReprAttrChanged(Inkscape::XML::Node */*repr*/, gchar const */*key*/, gchar const */*oldval*/, gchar const */*newval*/, bool /*is_interactive*/)
{
    readRepr();

    if ( ! (_wr.isUpdating()) )
        updateWidgets();
}




Gtk::Widget *
CanvasNGonGrid::newSpecificWidget()
{
    Gtk::Table * table = Gtk::manage( new Gtk::Table(1,1) );
    table->set_spacings(2);

_wr.setUpdating (true);

    Inkscape::UI::Widget::RegisteredUnitMenu *_rumg = Gtk::manage( new Inkscape::UI::Widget::RegisteredUnitMenu(
            _("Grid _units:"), "units", _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_ox = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("_Origin X:"), _("X coordinate of grid origin"), "originx", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_oy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("O_rigin Y:"), _("Y coordinate of grid origin"), "originy", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalar *_rsu_ns = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalar(
            _("_Number of sections:"), "", "sections", _wr, repr, doc ) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_sx = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("Spacing _X:"), _("Distance between concentric grid polygons"), "spacingx", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_sy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("Spacing _Y:"), _("Distance between semi-radial grid lines"), "spacingy", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalar *_rsu_ar = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalar(
            _("Ro_tation:"), _("Angle of axis rotation"), "rotation", _wr, repr, doc ) );

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Minor grid line _color:"), _("Minor grid line color"), _("Color of the minor grid lines"),
            "color", "opacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gmcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Ma_jor grid line color:"), _("Major grid line color"),
            _("Color of the major (highlighted) grid lines"),
            "empcolor", "empopacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredSuffixedInteger *_rsi = Gtk::manage( new Inkscape::UI::Widget::RegisteredSuffixedInteger(
            _("_Major grid line every:"), "", _("lines"), "empspacing", _wr, repr, doc ) );

    _rsu_ox->setDigits(5);
    _rsu_ox->setIncrements(0.1, 1.0);

    _rsu_oy->setDigits(5);
    _rsu_oy->setIncrements(0.1, 1.0);

    _rsu_ns->setDigits(0);
    _rsu_ns->setIncrements(1, 3);

    _rsu_sx->setDigits(5);
    _rsu_sx->setIncrements(0.1, 1.0);

    _rsu_sy->setDigits(5);
    _rsu_sy->setIncrements(0.1, 1.0);

    _rsu_ar->setDigits(5);
    _rsu_ar->setIncrements(0.5, 5.0);

_wr.setUpdating (false);

    Gtk::Widget const *const widget_array[] = {
        0,                  _rumg,
        0,                  _rsu_ox,
        0,                  _rsu_oy,
        0,                  _rsu_ns,
        0,                  _rsu_sx,
        0,                  _rsu_sy,
        0,                  _rsu_ar,
        _rcp_gcol->_label,  _rcp_gcol,
        0,                  0,
        _rcp_gmcol->_label, _rcp_gmcol,
        0,                  _rsi,
    };

    attach_all (*table, widget_array, sizeof(widget_array));

    // set widget values
    _rumg->setUnit (gridunit);

    gdouble val;
    val = origin[Geom::X];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_ox->setValue (val);
    val = origin[Geom::Y];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_oy->setValue (val);
    _rsu_ns->setValue (sections);
    val = lengthx;
    double gridx = sp_pixels_get_units (val, *(gridunit));
    _rsu_sx->setValue (gridx);
    val = lengthy;
    double gridy = sp_pixels_get_units (val, *(gridunit));
    _rsu_sy->setValue (gridy);

    _rsu_ar->setValue(angle_deg);

    _rcp_gcol->setRgba32 (color);
    _rcp_gmcol->setRgba32 (empcolor);
    _rsi->setValue (empspacing);

    return table;
}


/**
 * Update dialog widgets from object's values.
 */
void
CanvasNGonGrid::updateWidgets()
{
/*    if (_wr.isUpdating()) return;

    _wr.setUpdating (true);

    _rcb_visible.setActive(visible);
    if (snapper != NULL) {
        _rcb_enabled.setActive(snapper->getEnabled());
    }

    _rumg.setUnit (gridunit);

    gdouble val;
    val = origin[Geom::X];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_ox->setValue (val);
    val = origin[Geom::Y];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_oy->setValue (val);
    val = lengthx;
    double gridx = sp_pixels_get_units (val, *(gridunit));
    _rsu_sx->setValue (gridx);
    val = lengthy;
    double gridy = sp_pixels_get_units (val, *(gridunit));
    _rsu_sy->setValue (gridy);

    _rsu_ar->setValue(angle_deg);

    _rcp_gcol->setRgba32 (color);
    _rcp_gmcol->setRgba32 (empcolor);
    _rsi->setValue (empspacing);

    _wr.setUpdating (false);

    return;
    */
}



void
CanvasNGonGrid::Update (Geom::Affine const &affine, unsigned int /*flags*/)
{
    ow = origin * affine;
    sw = Geom::Point(fabs(affine[0]),fabs(affine[3]));
    sw[Geom::X] *= lengthx;
    sw[Geom::Y] *= lengthy;

    scaled = false;

    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = empspacing;

        if (scaling_factor <= 1)
            scaling_factor = 5;

        int watchdog = 0;
        while (  (sw[dim] < 8.0) & (watchdog < 100) ) {
            scaled = true;
            sw[dim] *= scaling_factor;
            // First pass, go up to the major line spacing, then
            // keep increasing by two.
            scaling_factor = 2;
            watchdog++;
        }

    }

    lxw = sw[Geom::X];
    lyw = sw[Geom::Y];

    if (empspacing == 0) {
        scaled = true;
    }
}

void
CanvasNGonGrid::Render (SPCanvasBuf *buf)
{
    //set correct coloring, depending preference (when zoomed out, always major coloring or minor coloring)
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint32 _empcolor;
    bool preference = prefs->getBool("/options/grids/no_emphasize_when_zoomedout", false);
    if( scaled && preference ) {
        _empcolor = color;
    } else {
        _empcolor = empcolor;
    }

    cairo_save(buf->ct);
    cairo_translate(buf->ct, -buf->rect.left(), -buf->rect.top());
    cairo_set_line_width(buf->ct, 1.0);
    cairo_set_line_cap(buf->ct, CAIRO_LINE_CAP_SQUARE);

    double angle_step = 360.0 / sections;
    for (int s = 0; s < sections; ++s) {
        //TODO: get angles from origin to viewport corners, use to test if section needs to be rendered
        renderSection(buf, (s * angle_step) - angle_deg, _empcolor);
    }

    cairo_restore(buf->ct);
}

void
CanvasNGonGrid::renderSection (SPCanvasBuf *buf, double section_angle_deg, guint32 _empcolor)
{
    // pc = preimagecoordinates (the coordinates of the section before rotation)
    // gc = gridcoordinates (the coordinates calculated from the grids origin 'grid->ow')

    Geom::Rect buf_gc(buf->rect);
    buf_gc -= ow;

    double const section_angle_rad = Geom::deg_to_rad(section_angle_deg);
    double const section_sin = sin(section_angle_rad);
    double const section_cos = cos(section_angle_rad);

    gdouble xmin = find_bound(buf_gc, -section_sin, section_cos, &std::min) / lxw;
    gdouble xmax = find_bound(buf_gc, -section_sin, section_cos, &std::max) / lxw;
    if (xmax <= 0) return; // Section is entirely out of viewport

    gdouble ymin = find_bound(buf_gc, -section_cos, -section_sin, &std::min) / lyw;
    gdouble ymax = find_bound(buf_gc, -section_cos, -section_sin, &std::max) / lyw;

    gdouble const lxmax = xmax * lxw;
    gdouble const xbound = (floor(xmax) + 0.5) * lxw;
    gdouble const ybound = (floor(ymax) + 0.5) * lyw;

    // Render section edge line
    {
        gdouble const pc_x = lxmax;
        gdouble const pc_y = lxmax * se_tan;
        gdouble const gc_x1 = ( (pc_x * section_cos) - (pc_y * section_sin) ) + ow[Geom::X];
        gdouble const gc_y1 = ( (pc_x * section_sin) + (pc_y * section_cos) ) + ow[Geom::Y];
        sp_cngongrid_drawline (buf, ow[Geom::X], ow[Geom::Y], gc_x1, gc_y1, color);
    }

    // Render semi-radius lines
    ymin = ceil(ymin);
    gint xlinenum = ymin;
    for (gdouble y = ymin * lyw; y <= ybound; y += lyw, xlinenum++) {
        // Compute points in preimage coordinates
        gdouble const pc_x0 = fabs(y) / se_tan;
        gdouble const pc_x1 = lxmax;
        if (pc_x1 <= pc_x0) continue;
        // Compute points in grid coordinates (with rotation applied)
        gdouble const ys = y * section_sin;
        gdouble const yc = y * section_cos;
        gdouble const gc_x0 = ( (pc_x0 * section_cos) - ys ) + ow[Geom::X];
        gdouble const gc_x1 = ( (pc_x1 * section_cos) - ys ) + ow[Geom::X];
        gdouble const gc_y0 = ( (pc_x0 * section_sin) + yc ) + ow[Geom::Y];
        gdouble const gc_y1 = ( (pc_x1 * section_sin) + yc ) + ow[Geom::Y];
        // Draw segment
        guint32 const _color = ( (!scaled && (xlinenum % empspacing) != 0) ? color : _empcolor);
        sp_cngongrid_drawline (buf, gc_x0, gc_y0, gc_x1, gc_y1, _color);
    }

    // Render concentric lines
    xmin = std::max(1.0, ceil(xmin));
    gint ylinenum = xmin;
    for (gdouble x = xmin * lxw; x < xbound; x += lxw, ylinenum++) {
        // Compute points in preimage coordinates
        gdouble const pc_y = x * se_tan;
        // Compute points in grid coordinates (with rotation applied)
        gdouble const xs = x * section_sin;
        gdouble const xc = x * section_cos;
        gdouble const ys = pc_y * section_sin;
        gdouble const yc = pc_y * section_cos;
        gdouble const gc_x0 = xc + ys + ow[Geom::X];
        gdouble const gc_x1 = xc - ys + ow[Geom::X];
        gdouble const gc_y0 = xs - yc + ow[Geom::Y];
        gdouble const gc_y1 = xs + yc + ow[Geom::Y];
        // Draw segment
        guint32 const _color = ( (!scaled && (ylinenum % empspacing) != 0) ? color : _empcolor);
        sp_cngongrid_drawline (buf, gc_x0, gc_y0, gc_x1, gc_y1, _color);
    }
}

CanvasNGonGridSnapper::CanvasNGonGridSnapper(CanvasNGonGrid *grid, SnapManager *sm, Geom::Coord const d) : LineSnapper(sm, d)
{
    this->grid = grid;
}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
Geom::Coord CanvasNGonGridSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getGridTolerance() / zoom;
}

bool CanvasNGonGridSnapper::getSnapperAlwaysSnap() const
{
    return _snapmanager->snapprefs.getGridTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

LineSnapper::LineList
CanvasNGonGridSnapper::_getSnapLines(Geom::Point const &p) const
{
    LineList s;

    if ( grid == NULL ) {
        return s;
    }

    double spacing_h;
    double spacing_v;

    if (getSnapVisibleOnly()) {
        // Only snapping to visible grid lines
        spacing_h = grid->lxw; // horizontal
        spacing_v = grid->lyw; // vertical
        // convert screen pixels to px
        // FIXME: after we switch to snapping dist in screen pixels, this will be unnecessary
        SPDesktop const *dt = _snapmanager->getDesktop();
        if (dt) {
            spacing_h /= dt->current_zoom();
            spacing_v /= dt->current_zoom();
        }
    } else {
        // Snapping to any grid line, whether it's visible or not
        spacing_h = grid->lengthx;
        spacing_v = grid->lengthy;
    }

    // In a polygonal grid, any point will be surrounded by 6 grid lines:
    // - 4 lines of a section grid
    // - 2 lines of the section edges
    // Of these, we use the closer section edge, and the closer each of the grid X and Y lines

    // Calculate what section the point is in
    Geom::Point gc_point = p - grid->origin;
    double point_angle_rad;
    bool const x_is_zero = Geom::are_near(gc_point[Geom::X], 0.);
    bool const y_is_zero = Geom::are_near(gc_point[Geom::Y], 0.);
    if (Geom::are_near(p, grid->origin))
        point_angle_rad = 0;
    else
        point_angle_rad = Geom::atan2(gc_point);
    double const section_ratio = (point_angle_rad - grid->angle_rad - grid->se_angle_rad) / (2.0 * M_PI);
    int const section = floor( grid->sections * section_ratio ) + 1;

    // Compute spacing-unit vectors for section
    double const section_angle_rad = (2.0 * section * grid->se_angle_rad) + grid->angle_rad;
    double const section_sin = sin(section_angle_rad);
    double const section_cos = cos(section_angle_rad);
    Geom::Point const gc_nx(-section_sin, section_cos);
    Geom::Point const gc_ny( section_cos, section_sin);
    Geom::Point const gc_sx = gc_ny * spacing_h;
    Geom::Point const gc_sy = gc_nx * spacing_v;

    // Get point in section pre-image space
    double const pc_x = ( (gc_point[Geom::Y] * section_sin) + (gc_point[Geom::X] * section_cos) ) / spacing_h;
    double const pc_y = ( (gc_point[Geom::Y] * section_cos) - (gc_point[Geom::X] * section_sin) ) / spacing_v;

    // Add the nearer section edge line
    double const section_edge_angle_rad = section_angle_rad + ( (pc_y > 0.0 ? 1.0 : -1.0) * grid->se_angle_rad);
    Geom::Point const section_edge_norm(-sin(section_edge_angle_rad), cos(section_edge_angle_rad));
    s.push_back( std::make_pair(section_edge_norm, grid->origin) );

    // Add the two nearer lines of the grid square
    Geom::Point const gc_corner = (gc_sx * round(pc_x)) + (gc_sy * round(pc_y)) + grid->origin;
    s.push_back( std::make_pair(gc_nx, gc_corner) );
    s.push_back( std::make_pair(gc_ny, gc_corner) );

    return s;
}

void CanvasNGonGridSnapper::_addSnappedLine(IntermSnapResults &isr, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, Geom::Point const normal_to_line, Geom::Point const point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, source, source_num, Inkscape::SNAPTARGET_GRID, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    isr.grid_lines.push_back(dummy);
}

void CanvasNGonGridSnapper::_addSnappedPoint(IntermSnapResults &isr, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GRID, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

void CanvasNGonGridSnapper::_addSnappedLinePerpendicularly(IntermSnapResults &isr, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GRID_PERPENDICULAR, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

bool CanvasNGonGridSnapper::ThisSnapperMightSnap() const
{
    return _snap_enabled && _snapmanager->snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_GRID);
}


}; // namespace Inkscape


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
