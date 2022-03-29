// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef INKSCAPE_UI_WIDGET_CANVAS_H
#define INKSCAPE_UI_WIDGET_CANVAS_H
/*
 * Authors:
 *   Tavmjong Bah
 *   PBS <pbs3141@gmail.com>
 *
 * Copyright (C) 2022 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory>
#include <gtkmm.h>
#include <2geom/rect.h>
#include <2geom/int-rect.h>
#include "display/rendermode.h"

class SPDesktop;

namespace Inkscape {

class CanvasItem;
class CanvasItemGroup;
class Drawing;

namespace UI {
namespace Widget {

class CanvasPrivate;

/**
 * A Gtk::DrawingArea widget for Inkscape's canvas.
 */ 
class Canvas : public Gtk::DrawingArea
{
    using parent_type = Gtk::DrawingArea;

public:

    Canvas();
    ~Canvas() override;

    /* Configuration */

    // Desktop (Todo: Remove.)
    void set_desktop(SPDesktop *desktop) { _desktop = desktop; }
    SPDesktop *get_desktop() const { return _desktop; }

    // Drawing
    void set_drawing(Inkscape::Drawing *drawing);

    // Canvas item root
    CanvasItemGroup *get_canvas_item_root() const { return _canvas_item_root; }

    // Geometry
    void set_pos   (const Geom::IntPoint &pos);
    void set_pos   (const Geom::Point    &fpos) { set_pos(fpos.round()); }
    void set_affine(const Geom::Affine   &affine);
    const Geom::IntPoint& get_pos   () const {return _pos;}
    const Geom::Affine&   get_affine() const {return _affine;}

    // Background
    void set_background_color(guint32 rgba);
    void set_background_checkerboard(guint32 rgba = 0xC4C4C4FF, bool use_alpha = false);

    //  Rendering modes
    void set_render_mode(Inkscape::RenderMode mode);
    void set_color_mode (Inkscape::ColorMode  mode);
    void set_split_mode (Inkscape::SplitMode  mode);
    Inkscape::RenderMode get_render_mode() const { return _render_mode; }
    Inkscape::ColorMode  get_color_mode()  const { return _color_mode; }
    Inkscape::SplitMode  get_split_mode()  const { return _split_mode; }

    // CMS
    void set_cms_key(std::string key);
    const std::string& get_cms_key() const { return _cms_key; }
    void set_cms_active(bool active) { _cms_active = active; }
    bool get_cms_active() const { return _cms_active; }

    /* Observers */

    // Geometry
    Geom::IntPoint get_dimensions() const;
    bool world_point_inside_canvas(Geom::Point const &world) const; // desktop-events.cpp
    Geom::Point canvas_to_world(Geom::Point const &window) const;
    Geom::IntRect get_area_world() const;

    // State
    bool is_dragging() const { return _is_dragging; } // selection-chemistry.cpp

    // Encapsulation leaks
    Cairo::RefPtr<Cairo::ImageSurface> get_backing_store() const; // canvas-item-rotate.cpp
    Cairo::RefPtr<Cairo::Pattern>      get_background_pattern() const { return _background; } // canvas-item-rect.cpp, canvas-item-ctrl.cpp

    /* Methods */

    // Invalidation
    void redraw_all();                                // Mark everything as having changed.
    void redraw_area(Geom::Rect& area);               // Mark a rectangle of world space as having changed.
    void redraw_area(int x0, int y0, int x1, int y1);
    void redraw_area(Geom::Coord x0, Geom::Coord y0, Geom::Coord x1, Geom::Coord y1);
    void request_update();                            // Mark geometry as needing recalculation.

    // Gobblers (tool-base.cpp)
    int gobble_key_events(guint keyval, guint mask);
    void gobble_motion_events(guint mask);

    // Callback run on destructor of any canvas item
    void canvas_item_destructed(Inkscape::CanvasItem *item);

    // State
    Inkscape::CanvasItem *get_current_canvas_item() const { return _current_canvas_item; }
    void                  set_current_canvas_item(Inkscape::CanvasItem *item) {
        _current_canvas_item = item;
    }
    Inkscape::CanvasItem *get_grabbed_canvas_item() const { return _grabbed_canvas_item; }
    void                  set_grabbed_canvas_item(Inkscape::CanvasItem *item, Gdk::EventMask mask) {
        _grabbed_canvas_item = item;
        _grabbed_event_mask = mask;
    }
    void set_drawing_disabled(bool disable); // Disable during path ops, etc.
    void set_all_enter_events(bool on) { _all_enter_events = on; }

protected:

    void get_preferred_width_vfunc (int &minimum_width,  int &natural_width ) const override;
    void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;

    // Event handlers
    bool on_scroll_event        (GdkEventScroll*  ) override;
    bool on_button_event        (GdkEventButton*  );
    bool on_button_press_event  (GdkEventButton*  ) override;
    bool on_button_release_event(GdkEventButton*  ) override;
    bool on_enter_notify_event  (GdkEventCrossing*) override;
    bool on_leave_notify_event  (GdkEventCrossing*) override;
    bool on_focus_in_event      (GdkEventFocus*   ) override;
    bool on_key_press_event     (GdkEventKey*     ) override;
    bool on_key_release_event   (GdkEventKey*     ) override;
    bool on_motion_notify_event (GdkEventMotion*  ) override;

    void on_realize() override;
    void on_unrealize() override;
    void on_size_allocate(Gtk::Allocation&) override;
    bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

private:

    /* Configuration */

    // Desktop
    SPDesktop *_desktop = nullptr;

    // Drawing
    Inkscape::Drawing *_drawing = nullptr;

    // Canvas item root
    CanvasItemGroup *_canvas_item_root = nullptr;

    // Geometry
    Geom::IntPoint _pos = {0, 0}; ///< Coordinates of top-left pixel of canvas view within canvas.
    Geom::Affine _affine; ///< The affine that we have been requested to draw at.

    // Background
    Cairo::RefPtr<Cairo::Pattern> _background; ///< The background of the widget.

    // Rendering modes
    Inkscape::RenderMode _render_mode = Inkscape::RenderMode::NORMAL;
    Inkscape::SplitMode _split_mode = Inkscape::SplitMode::NORMAL;
    Inkscape::ColorMode _color_mode = Inkscape::ColorMode::NORMAL;

    // CMS
    std::string _cms_key;
    bool _cms_active = false;

    /* Internal state */

    // Event handling/item picking
    GdkEvent _pick_event;         ///< Event used to find currently selected item.
    bool     _in_repick;          ///< For tracking recursion of pick_current_item().
    bool     _left_grabbed_item;  ///< ?
    bool     _all_enter_events;   ///< Keep all enter events. Only set true in connector-tool.cpp.
    bool     _is_dragging;        ///< Used in selection-chemistry to block undo/redo.
    int      _state;              ///< Last known modifier state (SHIFT, CTRL, etc.).

    Inkscape::CanvasItem *_current_canvas_item;      ///< Item containing cursor, nullptr if none.
    Inkscape::CanvasItem *_current_canvas_item_new;  ///< Item to become _current_item, nullptr if none.
    Inkscape::CanvasItem *_grabbed_canvas_item;      ///< Item that holds a pointer grab; nullptr if none.
    Gdk::EventMask _grabbed_event_mask;

    // Drawing
    bool _drawing_disabled = false;  ///< Disable drawing during critical operations
    bool _need_update = true; // Set true so setting CanvasItem bounds are calculated at least once.

    // Split view
    Inkscape::SplitDirection _split_direction;
    Geom::Point _split_position;
    Inkscape::SplitDirection _hover_direction;
    bool _split_dragging;
    Geom::Point _split_drag_start;

    void add_clippath(const Cairo::RefPtr<Cairo::Context>&);
    void set_cursor();

    // Opaque pointer to implementation
    friend class CanvasPrivate;
    std::unique_ptr<CanvasPrivate> d;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_CANVAS_H

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
