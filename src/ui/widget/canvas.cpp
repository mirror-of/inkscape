// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Authors:
 *   Tavmjong Bah
 *   PBS <pbs3141@gmail.com>
 *
 * Copyright (C) 2022 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <iostream> // Logging
#include <algorithm> // Sort
#include <set> // Coarsener

#include <glibmm/i18n.h>

#include <2geom/rect.h>

#include "canvas.h"
#include "canvas-grid.h"

#include "color.h"          // Background color
#include "cms-system.h"     // Color correction
#include "desktop.h"
#include "preferences.h"

#include "display/cairo-utils.h"     // Checkerboard background
#include "display/drawing.h"
#include "display/control/canvas-item-group.h"
#include "display/control/snap-indicator.h"

#include "ui/tools/tool-base.h"      // Default cursor

#include "framecheck.h"    // For frame profiling
#define framecheck_whole_function(D) auto framecheckobj = D->prefs.debug_framecheck ? FrameCheck::Event(__func__) : FrameCheck::Event();

/*
 *   The canvas is responsible for rendering the SVG drawing with various "control"
 *   items below and on top of the drawing. Rendering is triggered by a call to one of:
 *
 *
 *   * redraw_all()     Redraws the entire canvas by calling redraw_area() with the canvas area.
 *
 *   * redraw_area()    Redraws the indicated area. Use when there is a change that doesn't affect
 *                      a CanvasItem's geometry or size.
 *
 *   * request_update() Redraws after recalculating bounds for changed CanvasItems. Use if a
 *                      CanvasItem's geometry or size has changed.
 *
 *   The first three functions add a request to the Gtk's "idle" list via
 *
 *   * add_idle()       Which causes Gtk to call when resources are available:
 *
 *   * on_idle()        Which sets up the backing stores, divides the area of the canvas that has been marked
 *                      unclean into rectangles that are small enough to render quickly, and renders them outwards
 *                      from the mouse with a call to:
 *
 *   * paint_rect_internal() Which paints the rectangle using paint_single_buffer(). It renders onto a Cairo
 *                           surface "backing_store". After a piece is rendered there is a call to:
 *
 *   * queue_draw_area() A Gtk function for marking areas of the window as needing a repaint, which when
 *                       the time is right calls:
 *
 *   * on_draw()        Which blits the Cairo surface to the screen.
 *
 *   The other responsibility of the canvas is to determine where to send GUI events. It does this
 *   by determining which CanvasItem is "picked" and then forwards the events to that item. Not all
 *   items can be picked. As a last resort, the "CatchAll" CanvasItem will be picked as it is the
 *   lowest CanvasItem in the stack (except for the "root" CanvasItem). With a small be of work, it
 *   should be possible to make the "root" CanvasItem a "CatchAll" eliminating the need for a
 *   dedicated "CatchAll" CanvasItem. There probably could be efficiency improvements as some
 *   items that are not pickable probably should be which would save having to effectively pick
 *   them "externally" (e.g. gradient CanvasItemCurves).
 */

namespace Inkscape {
namespace UI {
namespace Widget {

/*
 * GDK event utilities
 */

// GdkEvents can only be safely copied using gdk_event_copy. However, this function allocates. Therefore, we need the following smart pointer to wrap the result.
struct GdkEventFreer {void operator()(GdkEvent *ev) const {gdk_event_free(ev);}};
using GdkEventUniqPtr = std::unique_ptr<GdkEvent, GdkEventFreer>;

// Copies a GdkEvent, returning the result as a smart pointer.
auto make_unique_copy(const GdkEvent &ev) {return GdkEventUniqPtr(gdk_event_copy(&ev));}

/*
 * Preferences
 */

template<typename T>
struct Pref {};

template<typename T>
struct PrefBase
{
    const char *path;
    T t, def;
    std::unique_ptr<Preferences::PreferencesObserver> obs;
    std::function<void()> action;
    operator T() const {return t;}
    PrefBase(const char *path, T def) : path(path), def(def) {enable();}
    void act() {if (action) action();}
    void enable() {t = static_cast<Pref<T>*>(this)->read(); act(); obs = Inkscape::Preferences::get()->createObserver(path, [this] (const Preferences::Entry &e) {t = static_cast<Pref<T>*>(this)->changed(e); act();});}
    void disable() {t = def; act(); obs.reset();}
    void set_enabled(bool enabled) {enabled ? enable() : disable();}
};

template<>
struct Pref<bool> : PrefBase<bool>
{
    Pref(const char *path, bool def = false) : PrefBase(path, def) {}
    bool read() {return Inkscape::Preferences::get()->getBool(path, def);}
    bool changed(const Preferences::Entry &e) {return e.getBool(def);}
};

template<>
struct Pref<int> : PrefBase<int>
{
    int min, max;
    Pref(const char *path, int def, int min, int max) : min(min), max(max), PrefBase(path, def) {}
    int read() {return Inkscape::Preferences::get()->getIntLimited(path, def, min, max);}
    int changed(const Preferences::Entry &e) {return e.getIntLimited(def, min, max);}
};

template<>
struct Pref<double> : PrefBase<double>
{
    double min, max;
    Pref(const char *path, double def, double min, double max) : min(min), max(max), PrefBase(path, def) {}
    double read() {return Inkscape::Preferences::get()->getDoubleLimited(path, def, min, max);}
    double changed(const Preferences::Entry &e) {return e.getDoubleLimited(def, min, max);}
};

struct Prefs
{
    // Original parameters
    Pref<int>    tile_size                = Pref<int>   ("/options/rendering/tile-size", 16, 1, 10000);
    Pref<int>    tile_multiplier          = Pref<int>   ("/options/rendering/tile-multiplier", 16, 1, 512);
    Pref<int>    x_ray_radius             = Pref<int>   ("/options/rendering/xray-radius", 100, 1, 1500);
    Pref<bool>   from_display             = Pref<bool>  ("/options/displayprofile/from_display");
    Pref<int>    grabsize                 = Pref<int>   ("/options/grabsize/value", 3, 1, 15);
    Pref<int>    outline_overlay_opacity  = Pref<int>   ("/options/rendering/outline-overlay-opacity", 50, 1, 100);

    // New parameters
    Pref<int>    update_strategy          = Pref<int>   ("/options/rendering/update_strategy", 3, 1, 3);
    Pref<int>    render_time_limit        = Pref<int>   ("/options/rendering/render_time_limit", 1000, 100, 1000000);
    Pref<bool>   use_new_bisector         = Pref<bool>  ("/options/rendering/use_new_bisector", true);
    Pref<int>    new_bisector_size        = Pref<int>   ("/options/rendering/new_bisector_size", 500, 1, 10000);
    Pref<double> max_affine_diff          = Pref<double>("/options/rendering/max_affine_diff", 1.8, 0.0, 100.0);
    Pref<int>    pad                      = Pref<int>   ("/options/rendering/pad", 200, 0, 1000);
    Pref<int>    coarsener_min_size       = Pref<int>   ("/options/rendering/coarsener_min_size", 200, 0, 1000);
    Pref<int>    coarsener_glue_size      = Pref<int>   ("/options/rendering/coarsener_glue_size", 80, 0, 1000);
    Pref<double> coarsener_min_fullness   = Pref<double>("/options/rendering/coarsener_min_fullness", 0.3, 0.0, 1.0);

    // Debug switches
    Pref<bool>   debug_framecheck         = Pref<bool>  ("/options/rendering/debug_framecheck");
    Pref<bool>   debug_logging            = Pref<bool>  ("/options/rendering/debug_logging");
    Pref<bool>   debug_slow_redraw        = Pref<bool>  ("/options/rendering/debug_slow_redraw");
    Pref<int>    debug_slow_redraw_time   = Pref<int>   ("/options/rendering/debug_slow_redraw_time", 50, 0, 1000000);
    Pref<bool>   debug_show_redraw        = Pref<bool>  ("/options/rendering/debug_show_redraw");
    Pref<bool>   debug_show_unclean       = Pref<bool>  ("/options/rendering/debug_show_unclean");
    Pref<bool>   debug_show_snapshot      = Pref<bool>  ("/options/rendering/debug_show_snapshot");
    Pref<bool>   debug_show_clean         = Pref<bool>  ("/options/rendering/debug_show_clean");
    Pref<bool>   debug_disable_redraw     = Pref<bool>  ("/options/rendering/debug_disable_redraw");
    Pref<bool>   debug_sticky_decoupled   = Pref<bool>  ("/options/rendering/debug_sticky_decoupled");

    // Developer mode
    Pref<bool> devmode = Pref<bool>("/options/rendering/devmode");
    void set_devmode(bool on);
};

void Prefs::set_devmode(bool on)
{
    tile_size.set_enabled(on);
    render_time_limit.set_enabled(on);
    use_new_bisector.set_enabled(on);
    new_bisector_size.set_enabled(on);
    max_affine_diff.set_enabled(on);
    pad.set_enabled(on);
    coarsener_min_size.set_enabled(on);
    coarsener_glue_size.set_enabled(on);
    coarsener_min_fullness.set_enabled(on);
    debug_framecheck.set_enabled(on);
    debug_logging.set_enabled(on);
    debug_slow_redraw.set_enabled(on);
    debug_slow_redraw_time.set_enabled(on);
    debug_show_redraw.set_enabled(on);
    debug_show_unclean.set_enabled(on);
    debug_show_snapshot.set_enabled(on);
    debug_show_clean.set_enabled(on);
    debug_disable_redraw.set_enabled(on);
    debug_sticky_decoupled.set_enabled(on);
}

/*
 * Conversion functions
 */

auto geom_to_cairo(Geom::IntRect rect)
{
    return Cairo::RectangleInt{rect.left(), rect.top(), rect.width(), rect.height()};
}

auto cairo_to_geom(Cairo::RectangleInt rect)
{
    return Geom::IntRect::from_xywh(rect.x, rect.y, rect.width, rect.height);
}

auto geom_to_cairo(Geom::Affine affine)
{
    return Cairo::Matrix(affine[0], affine[1], affine[2], affine[3], affine[4], affine[5]);
}

auto geom_act(Geom::Affine a, Geom::IntPoint p)
{
    Geom::Point p2 = p;
    p2 *= a;
    return Geom::IntPoint(std::round(p2.x()), std::round(p2.y()));
}

void region_to_path(const Cairo::RefPtr<Cairo::Context> &cr, const Cairo::RefPtr<Cairo::Region> &reg)
{
    for (int i = 0; i < reg->get_num_rectangles(); i++) {
        auto rect = reg->get_rectangle(i);
        cr->rectangle(rect.x, rect.y, rect.width, rect.height);
    }
}

/*
 * Update strategy
 */

// A class hierarchy for controlling what order to update invalidated regions.
class Updater
{
public:
    // The subregion of the store with up-to-date content.
    Cairo::RefPtr<Cairo::Region> clean_region;

    Updater(Cairo::RefPtr<Cairo::Region> clean_region) : clean_region(std::move(clean_region)) {}

    virtual void reset()                               {clean_region = Cairo::Region::create();}       // Reset the clean region to empty.
    virtual void intersect (const Geom::IntRect &rect) {clean_region->intersect(geom_to_cairo(rect));} // Called when the store changes position; clip everything to the new store rectangle.
    virtual void mark_dirty(const Geom::IntRect &rect) {clean_region->subtract (geom_to_cairo(rect));} // Called on every invalidate event.
    virtual void mark_clean(const Geom::IntRect &rect) {clean_region->do_union (geom_to_cairo(rect));} // Called on every rectangle redrawn.

    virtual Cairo::RefPtr<Cairo::Region> get_next_clean_region() {return clean_region;}; // Called by on_idle to determine what regions to consider clean for the current redraw.
    virtual bool                         report_finished      () {return false;}         // Called in on_idle if the redraw has finished. Returns true to indicate that further redraws are required with a different clean region.
    virtual void                         frame                () {}                      // Called by on_draw to notify the updater of the display of the frame.
    virtual ~Updater() = default;
};

// Responsive updater: As soon as a region is invalidated, redraw it.
using ResponsiveUpdater = Updater;

// Full redraw updater: When a region is invalidated, delay redraw until after the current redraw is completed.
class FullredrawUpdater : public Updater
{
    // Whether we are currently in the middle of a redraw.
    bool inprogress = false;

    // Contains a copy of the old clean region if damage events occurred during the current redraw, otherwise null.
    Cairo::RefPtr<Cairo::Region> old_clean_region;

public:

    FullredrawUpdater(Cairo::RefPtr<Cairo::Region> clean_region) : Updater(std::move(clean_region)) {}

    void reset() override
    {
        Updater::reset();
        inprogress = false;
        old_clean_region.clear();
    }

    void intersect(const Geom::IntRect &rect) override
    {
        Updater::intersect(rect);
        if (old_clean_region) old_clean_region->intersect(geom_to_cairo(rect));
    }

    void mark_dirty(const Geom::IntRect &rect) override
    {
        if (inprogress && !old_clean_region) old_clean_region = clean_region->copy();
        Updater::mark_dirty(rect);
    }

    void mark_clean(const Geom::IntRect &rect) override
    {
        Updater::mark_clean(rect);
        if (old_clean_region) old_clean_region->do_union(geom_to_cairo(rect));
    }

    Cairo::RefPtr<Cairo::Region> get_next_clean_region() override
    {
        inprogress = true;
        if (!old_clean_region) {
            return clean_region;
        } else {
            return old_clean_region;
        }
    }

    bool report_finished() override
    {
        assert(inprogress);
        if (!old_clean_region) {
            // Completed redraw without being damaged => finished.
            inprogress = false;
            return false;
        } else {
            // Completed redraw but damage events arrived => ask for another redraw, using the up-to-date clean region.
            old_clean_region.clear();
            return true;
        }
    }
};

// Multiscale updater: Updates tiles near the mouse faster. Gives the best of both.
class MultiscaleUpdater : public Updater
{
    // Whether we are currently in the middle of a redraw.
    bool inprogress = false;

    // Whether damage events occurred during the current redraw.
    bool activated = false;

    int counter; // A steadily incrementing counter from which the current scale is derived.
    int scale; // The current scale to process updates at.
    int elapsed; // How much time has been spent at the current scale.
    std::vector<Cairo::RefPtr<Cairo::Region>> blocked; // The region blocked from being updated at each scale.

public:

    MultiscaleUpdater(Cairo::RefPtr<Cairo::Region> clean_region) : Updater(std::move(clean_region)) {}

    void reset() override
    {
        Updater::reset();
        inprogress = activated = false;
    }

    void intersect(const Geom::IntRect &rect) override
    {
        Updater::intersect(rect);
        if (activated) {
            for (auto &reg : blocked) {
                reg->intersect(geom_to_cairo(rect));
            }
        }
    }

    void mark_dirty(const Geom::IntRect &rect) override
    {
        Updater::mark_dirty(rect);
        if (inprogress && !activated) {
            counter = scale = elapsed = 0;
            blocked = {Cairo::Region::create()};
            activated = true;
        }
    }

    void mark_clean(const Geom::IntRect &rect) override
    {
        Updater::mark_clean(rect);
        if (activated) blocked[scale]->do_union(geom_to_cairo(rect));
    }

    Cairo::RefPtr<Cairo::Region> get_next_clean_region() override
    {
        inprogress = true;
        if (!activated) {
            return clean_region;
        } else {
            auto result = clean_region->copy();
            result->do_union(blocked[scale]);
            return result;
        }
    }

    bool report_finished() override
    {
        assert(inprogress);
        if (!activated) {
            // Completed redraw without damage => finished.
            inprogress = false;
            return false;
        } else {
            // Completed redraw but damage events arrived => begin updating any remaining damaged regions.
            activated = false;
            blocked.clear();
            return true;
        }
    }

    void frame() override
    {
        if (!activated) return;

        // Stay at the current scale for 2^scale frames.
        elapsed++;
        if (elapsed < (1 << scale)) return;
        elapsed = 0;

        // Adjust the counter, which causes scale to hop around the values 0, 1, 2... spending half as much time at each subsequent scale.
        counter++;
        scale = 0;
        for (int tmp = counter; tmp % 2 == 1; tmp /= 2) {
            scale++;
        }

        // Ensure sufficiently many blocked zones exist.
        if (scale == blocked.size()) {
            blocked.emplace_back();
        }

        // Recreate the current blocked zone as the union of the clean region and lower-scale blocked zones.
        blocked[scale] = clean_region->copy();
        for (int i = 0; i < scale; i++) {
            blocked[scale]->do_union(blocked[i]);
        }
    }
};

std::unique_ptr<Updater>
make_updater(int type, Cairo::RefPtr<Cairo::Region> clean_region = Cairo::Region::create())
{
    switch (type) {
        case 1: return std::make_unique<ResponsiveUpdater>(std::move(clean_region));
        case 2: return std::make_unique<FullredrawUpdater>(std::move(clean_region));
        default:
        case 3: return std::make_unique<MultiscaleUpdater>(std::move(clean_region));
    }
}

/*
 * Implementation class
 */

class CanvasPrivate
{
public:

    friend class Canvas;
    Canvas *q;
    CanvasPrivate(Canvas *q) : q(q) {}

    // Lifecycle
    bool active = false;
    void update_active();
    void activate();
    void deactivate();

    // Preferences
    Prefs prefs;

    // Update strategy; tracks the unclean region and decides how to redraw it.
    std::unique_ptr<Updater> updater;

    // Event processor. Events that interact with the Canvas are buffered here until the start of the next frame. They are processed by a separate object so that deleting the Canvas mid-event can be done safely.
    struct EventProcessor
    {
        std::vector<GdkEventUniqPtr> events;
        int pos;
        GdkEvent *ignore = nullptr;
        CanvasPrivate *canvasprivate; // Nulled on destruction.
        bool in_processing = false; // For handling recursion due to nested GTK main loops.
        void process();
        int gobble_key_events(guint keyval, guint mask);
        void gobble_motion_events(guint mask);
    };
    std::shared_ptr<EventProcessor> eventprocessor; // Usually held by CanvasPrivate, but temporarily also held by itself while processing so that it is not deleted mid-event.
    bool add_to_bucket(GdkEvent*);
    bool process_bucketed_event(const GdkEvent&);
    bool pick_current_item(const GdkEvent&);
    bool emit_event(const GdkEvent&);

    // State for determining when to run event processor.
    bool pending_draw = false;
    sigc::connection bucket_emptier;
    std::optional<guint> bucket_emptier_tick_callback;
    void schedule_bucket_emptier();

    // Idle system. The high priority idle ensures at least one idle cycle between add_idle and on_draw.
    void add_idle();
    sigc::connection hipri_idle;
    sigc::connection lopri_idle;
    bool on_hipri_idle();
    bool on_lopri_idle();
    bool idle_running = false;

    // Important global properties of all the stores. If these change, all the stores must be recreated.
    int _device_scale = 1;
    bool _store_solid_background;

    // The backing store.
    Geom::IntRect _store_rect;
    Geom::Affine _store_affine;
    Cairo::RefPtr<Cairo::ImageSurface> _backing_store, _outline_store;

    // The snapshot store. Used to mask redraw delay on zoom/rotate.
    Geom::IntRect _snapshot_rect;
    Geom::Affine _snapshot_affine;
    Cairo::RefPtr<Cairo::ImageSurface> _snapshot_store, _snapshot_outline_store;
    Cairo::RefPtr<Cairo::Region> _snapshot_clean_region;

    Geom::Affine geom_affine; // The affine the geometry was last imbued with.
    bool decoupled_mode = false;

    bool solid_background; // Whether the last background set is solid.
    bool need_outline_store() const {return q->_split_mode != Inkscape::SplitMode::NORMAL || q->_render_mode == Inkscape::RenderMode::OUTLINE_OVERLAY;}

    // Drawing
    bool on_idle();
    void paint_rect_internal(Geom::IntRect const &rect);
    void paint_single_buffer(Geom::IntRect const &paint_rect, Cairo::RefPtr<Cairo::ImageSurface> const &store, bool is_backing_store, bool outline_overlay_pass);
    std::optional<Geom::Dim2> old_bisector(const Geom::IntRect &rect);
    std::optional<Geom::Dim2> new_bisector(const Geom::IntRect &rect);

    // Trivial overload of GtkWidget function.
    void queue_draw_area(Geom::IntRect &rect);

    // For tracking the last known mouse position. (The function Gdk::Window::get_device_position cannot be used because of slow X11 round-trips. Remove this workaround when X11 dies.)
    std::optional<Geom::Point> last_mouse;
};

/*
 * Lifecycle
 */

Canvas::Canvas()
    : d(std::make_unique<CanvasPrivate>(this))
{
    set_name("InkscapeCanvas");

    // Events
    add_events(Gdk::BUTTON_PRESS_MASK   |
               Gdk::BUTTON_RELEASE_MASK |
               Gdk::ENTER_NOTIFY_MASK   |
               Gdk::LEAVE_NOTIFY_MASK   |
               Gdk::FOCUS_CHANGE_MASK   |
               Gdk::KEY_PRESS_MASK      |
               Gdk::KEY_RELEASE_MASK    |
               Gdk::POINTER_MOTION_MASK |
               Gdk::SCROLL_MASK         |
               Gdk::SMOOTH_SCROLL_MASK  );

    // Set up EventProcessor
    d->eventprocessor = std::make_shared<CanvasPrivate::EventProcessor>();
    d->eventprocessor->canvasprivate = d.get();

    // Updater
    d->updater = make_updater(d->prefs.update_strategy);

    // Preferences
    d->prefs.grabsize.action = [=] {_canvas_item_root->update_canvas_item_ctrl_sizes(d->prefs.grabsize);};
    d->prefs.debug_show_unclean.action = [=] {queue_draw();};
    d->prefs.debug_show_clean.action = [=] {queue_draw();};
    d->prefs.debug_disable_redraw.action = [=] {d->add_idle();};
    d->prefs.debug_sticky_decoupled.action = [=] {d->add_idle();};
    d->prefs.update_strategy.action = [=] {d->updater = make_updater(d->prefs.update_strategy, std::move(d->updater->clean_region));};
    d->prefs.outline_overlay_opacity.action = [=] {queue_draw();};

    // Developer mode master switch
    d->prefs.devmode.action = [=] {d->prefs.set_devmode(d->prefs.devmode);};
    d->prefs.devmode.action();

    // Cavas item root
    _canvas_item_root = new Inkscape::CanvasItemGroup(nullptr);
    _canvas_item_root->set_name("CanvasItemGroup:Root");
    _canvas_item_root->set_canvas(this);

    // Background
    _background = Cairo::SolidPattern::create_rgb(1.0, 1.0, 1.0);
    d->solid_background = true;
}

void CanvasPrivate::activate()
{
    // Event handling/item picking
    q->_pick_event.type = GDK_LEAVE_NOTIFY;
    q->_pick_event.crossing.x = 0;
    q->_pick_event.crossing.y = 0;

    q->_in_repick         = false;
    q->_left_grabbed_item = false;
    q->_all_enter_events  = false;
    q->_is_dragging       = false;
    q->_state             = 0;

    q->_current_canvas_item     = nullptr;
    q->_current_canvas_item_new = nullptr;
    q->_grabbed_canvas_item     = nullptr;
    q->_grabbed_event_mask = (Gdk::EventMask)0;

    // Drawing
    q->_drawing_disabled = false;
    q->_need_update = true;

    // Split view
    q->_split_direction = Inkscape::SplitDirection::EAST;
    q->_split_position = {-1, -1}; // initialize with off-canvas coordinates
    q->_hover_direction = Inkscape::SplitDirection::NONE;
    q->_split_dragging = false;

    add_idle();
}

void CanvasPrivate::deactivate()
{
    // Disconnect signals and timeouts. (Note: They will never be rescheduled while inactive.)
    hipri_idle.disconnect();
    lopri_idle.disconnect();
    bucket_emptier.disconnect();
    if (bucket_emptier_tick_callback) q->remove_tick_callback(*bucket_emptier_tick_callback);
}

Canvas::~Canvas()
{
    // Not necessary as GTK guarantees realization is always followed by unrealization. But just in case that invariant breaks, we deal with it.
    if (d->active) {
        std::cerr << "Canvas destructed while realized!" << std::endl;
        d->deactivate();
    }

    // Disconnect from EventProcessor.
    d->eventprocessor->canvasprivate = nullptr;

    // Remove entire CanvasItem tree.
    delete _canvas_item_root;
}

void CanvasPrivate::update_active()
{
    bool new_active = q->_drawing && q->get_realized();
    if (new_active != active) {
        active = new_active;
        active ? activate() : deactivate();
    }
}

void Canvas::set_drawing(Drawing *drawing)
{
    _drawing = drawing;
    d->update_active();
}

void
Canvas::on_realize()
{
    parent_type::on_realize();
    assert(get_realized());
    d->update_active();
}

void Canvas::on_unrealize()
{
    parent_type::on_unrealize();
    assert(!get_realized());
    d->update_active();
}

/*
 * Events system
 */

// The following protected functions of Canvas are where all incoming events initially arrive.
// Those that do not interact with the Canvas are processed instantaneously, while the rest are
// delayed by placing them into the bucket.

bool
Canvas::on_scroll_event(GdkEventScroll *scroll_event)
{
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(scroll_event));
}

bool
Canvas::on_button_press_event(GdkEventButton *button_event)
{
    return on_button_event(button_event);
}

bool
Canvas::on_button_release_event(GdkEventButton *button_event)
{
    return on_button_event(button_event);
}

// Unified handler for press and release events.
bool
Canvas::on_button_event(GdkEventButton *button_event)
{
    // Sanity-check event type.
    switch (button_event->type) {
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            break; // Good
        default:
            std::cerr << "Canvas::on_button_event: illegal event type!" << std::endl;
            return false;
    }

    // Drag the split view controller.
    switch (button_event->type) {
        case GDK_BUTTON_PRESS:
            if (_hover_direction != Inkscape::SplitDirection::NONE) {
                _split_dragging = true;
                _split_drag_start = Geom::Point(button_event->x, button_event->y);
                return true;
            }
            break;
        case GDK_2BUTTON_PRESS:
            if (_hover_direction != Inkscape::SplitDirection::NONE) {
                _split_direction = _hover_direction;
                _split_dragging = false;
                queue_draw();
                return true;
            }
            break;
        case GDK_BUTTON_RELEASE:
            _split_dragging = false;
            break;
    }

    // Otherwise, handle as a delayed event.
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(button_event));
}

bool
Canvas::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (crossing_event->window != get_window()->gobj()) {
        std::cout << "  WHOOPS... this does really happen" << std::endl;
        return false;
    }
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(crossing_event));
}

bool
Canvas::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    if (crossing_event->window != get_window()->gobj()) {
        std::cout << "  WHOOPS... this does really happen" << std::endl;
        return false;
    }
    d->last_mouse = {};
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(crossing_event));
}

bool
Canvas::on_focus_in_event(GdkEventFocus *focus_event)
{
    grab_focus();
    return false;
}

bool
Canvas::on_key_press_event(GdkEventKey *key_event)
{
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(key_event));
}

bool
Canvas::on_key_release_event(GdkEventKey *key_event)
{
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(key_event));
}

bool
Canvas::on_motion_notify_event(GdkEventMotion *motion_event)
{
    // Record the last mouse position.
    d->last_mouse = Geom::Point(motion_event->x, motion_event->y);

    // Handle interactions with the split view controller.
    Geom::IntPoint cursor_position = Geom::IntPoint(motion_event->x, motion_event->y);

    // Check if we are near the edge. If so, revert to normal mode.
    if (_split_mode == Inkscape::SplitMode::SPLIT && _split_dragging) {
        if (cursor_position.x() < 5                                  ||
            cursor_position.y() < 5                                  ||
            cursor_position.x() - get_allocation().get_width()  > -5 ||
            cursor_position.y() - get_allocation().get_height() > -5 ) {

            // Reset everything.
            _split_mode = Inkscape::SplitMode::NORMAL;
            _split_position = Geom::Point(-1, -1);
            _hover_direction = Inkscape::SplitDirection::NONE;
            set_cursor();
            queue_draw();

            // Update action (turn into utility function?).
            auto window = dynamic_cast<Gtk::ApplicationWindow *>(get_toplevel());
            if (!window) {
                std::cerr << "Canvas::on_motion_notify_event: window missing!" << std::endl;
                return true;
            }

            auto action = window->lookup_action("canvas-split-mode");
            if (!action) {
                std::cerr << "Canvas::on_motion_notify_event: action 'canvas-split-mode' missing!" << std::endl;
                return true;
            }

            auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
            if (!saction) {
                std::cerr << "Canvas::on_motion_notify_event: action 'canvas-split-mode' not SimpleAction!" << std::endl;
                return true;
            }

            saction->change_state((int)Inkscape::SplitMode::NORMAL);

            return true;
        }
    }

    if (_split_mode == Inkscape::SplitMode::XRAY) {
        _split_position = cursor_position;
        queue_draw();
    }

    if (_split_mode == Inkscape::SplitMode::SPLIT) {
        Inkscape::SplitDirection hover_direction = Inkscape::SplitDirection::NONE;
        Geom::Point difference(cursor_position - _split_position);

        // Move controller
        if (_split_dragging) {
            Geom::Point delta = cursor_position - _split_drag_start; // We don't use _split_position
            if (_hover_direction == Inkscape::SplitDirection::HORIZONTAL) {
                _split_position += Geom::Point(0, delta.y());
            } else if (_hover_direction == Inkscape::SplitDirection::VERTICAL) {
                _split_position += Geom::Point(delta.x(), 0);
            } else {
                _split_position += delta;
            }
            _split_drag_start = cursor_position;
            queue_draw();
            return true;
        }

        if (Geom::distance(cursor_position, _split_position) < 20 * d->_device_scale) {
            // We're hovering over circle, figure out which direction we are in.
            if (difference.y() - difference.x() > 0) {
                if (difference.y() + difference.x() > 0) {
                    hover_direction = Inkscape::SplitDirection::SOUTH;
                } else {
                    hover_direction = Inkscape::SplitDirection::WEST;
                }
            } else {
                if (difference.y() + difference.x() > 0) {
                    hover_direction = Inkscape::SplitDirection::EAST;
                } else {
                    hover_direction = Inkscape::SplitDirection::NORTH;
                }
            }
        } else if (_split_direction == Inkscape::SplitDirection::NORTH ||
                   _split_direction == Inkscape::SplitDirection::SOUTH) {
            if (std::abs(difference.y()) < 3 * d->_device_scale) {
                // We're hovering over horizontal line
                hover_direction = Inkscape::SplitDirection::HORIZONTAL;
            }
        } else {
            if (std::abs(difference.x()) < 3 * d->_device_scale) {
               // We're hovering over vertical line
                hover_direction = Inkscape::SplitDirection::VERTICAL;
            }
        }

        if (_hover_direction != hover_direction) {
            _hover_direction = hover_direction;
            set_cursor();
            queue_draw();
        }

        if (_hover_direction != Inkscape::SplitDirection::NONE) {
            // We're hovering, don't pick or emit event.
            return true;
        }
    }

    // Otherwise, handle as a delayed event.
    return d->add_to_bucket(reinterpret_cast<GdkEvent*>(motion_event));
}

// Most events end up here. We store them in the bucket, and process them as soon as possible after
// the next 'on_draw'. If 'on_draw' isn't pending, we use the 'tick_callback' signal to process them
// when 'on_draw' would have run anyway. If 'on_draw' later becomes pending, we remove this signal.

// Add an event to the bucket and ensure it will be emptied in the near future.
bool
CanvasPrivate::add_to_bucket(GdkEvent *event)
{
    framecheck_whole_function(this)

    if (!active) {
        std::cerr << "Canvas::add_to_bucket: Called while not active!" << std::endl;
        return false;
    }

    // Prevent re-fired events from going through again.
    if (event == eventprocessor->ignore) {
        return false;
    }

    // If this is the first event, ensure event processing will run on the main loop as soon as possible after the next frame has started.
    if (eventprocessor->events.empty() && !pending_draw) {
#ifndef NDEBUG
        if (bucket_emptier_tick_callback) {
            g_warning("bucket_emptier_tick_callback not empty");
        }
#endif
        bucket_emptier_tick_callback = q->add_tick_callback([this] (const Glib::RefPtr<Gdk::FrameClock>&) {
            assert(active);
            bucket_emptier_tick_callback.reset();
            schedule_bucket_emptier();
            return false;
        });
    }

    // Add a copy to the queue.
    eventprocessor->events.emplace_back(gdk_event_copy(event));

    // Tell GTK the event was handled.
    return true;
}

void CanvasPrivate::schedule_bucket_emptier()
{
    if (!active) {
        std::cerr << "Canvas::schedule_bucket_emptier: Called while not active!" << std::endl;
        return;
    }

    if (!bucket_emptier.connected()) {
        bucket_emptier = Glib::signal_idle().connect([this] {
            assert(active);
            eventprocessor->process();
            return false;
        }, G_PRIORITY_HIGH_IDLE + 14); // before hipri_idle
    }
}

// The following functions run at the start of the next frame on the GTK main loop.
// (Note: It is crucial that it runs on the main loop and not in any frame clock tick callbacks. GTK does not allow widgets to be deleted in the latter; only the former.)

// Process bucketed events.
void
CanvasPrivate::EventProcessor::process()
{
    framecheck_whole_function(canvasprivate)

    // Ensure the EventProcessor continues to live even if the Canvas is destroyed during event processing.
    auto self = canvasprivate->eventprocessor;

    // Check if toplevel or recursive. (Recursive calls happen if processing an event starts its own nested GTK main loop.)
    bool toplevel = !in_processing;
    in_processing = true;

    // If toplevel, initialise the iteration index. It may be incremented externally by gobblers or recursive calls.
    if (toplevel) {
        pos = 0;
    }

    while (pos < events.size()) {
        // Extract next event.
        auto event = std::move(events[pos]);
        pos++;

        // Fire the event at the CanvasItems and see if it was handled.
        bool handled = canvasprivate->process_bucketed_event(*event);

        if (!handled) {
            // Re-fire the event at the window, and ignore it when it comes back here again.
            ignore = event.get();
            canvasprivate->q->get_toplevel()->event(event.get());
            ignore = nullptr;
        }

        // If the Canvas was destroyed or deactivated during event processing, exit now.
        if (!canvasprivate || !canvasprivate->active) return;
    }

    // Otherwise, clear the list of events that was just processed.
    events.clear();

    // Reset the variable to track recursive calls.
    if (toplevel) {
        in_processing = false;
    }
}

// Called during event processing by some tools to batch backlogs of key events that may have built up after a freeze.
int
Canvas::gobble_key_events(guint keyval, guint mask)
{
    return d->eventprocessor->gobble_key_events(keyval, mask);
}

int
CanvasPrivate::EventProcessor::gobble_key_events(guint keyval, guint mask)
{
    int count = 0;

    while (pos < events.size()) {
        auto &event = events[pos];
        if ((event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE) && event->key.keyval == keyval && (!mask || (event->key.state & mask))) {
            // Discard event and continue.
            if (event->type == GDK_KEY_PRESS) count++;
            pos++;
        }
        else {
            // Stop discarding.
            break;
        }
    }

    if (count > 0 && canvasprivate->prefs.debug_logging) std::cout << "Gobbled " << count << " key press(es)" << std::endl;

    return count;
}

// Called during event processing by some tools to ignore backlogs of motion events that may have built up after a freeze.
void
Canvas::gobble_motion_events(guint mask)
{
    d->eventprocessor->gobble_motion_events(mask);
}

void
CanvasPrivate::EventProcessor::gobble_motion_events(guint mask)
{
    int count = 0;

    while (pos < events.size()) {
        auto &event = events[pos];
        if (event->type == GDK_MOTION_NOTIFY && (event->motion.state & mask)) {
            // Discard event and continue.
            count++;
            pos++;
        }
        else {
            // Stop discarding.
            break;
        }
    }

    if (count > 0 && canvasprivate->prefs.debug_logging) std::cout << "Gobbled " << count << " motion event(s)" << std::endl;
}

// From now on Inkscape's regular event processing logic takes place. The only thing to remember is that
// all of this happens at a slight delay after the original GTK events. Therefore, it's important to make
// sure that stateful variables like '_current_canvas_item' and friends are ONLY read/written within these
// functions, not during the earlier GTK event handlers. Otherwise state confusion will ensue.

bool
CanvasPrivate::process_bucketed_event(const GdkEvent &event)
{
    auto calc_button_mask = [&] () -> int {
        switch (event.button.button) {
            case 1:  return GDK_BUTTON1_MASK; break; // Fixme: These all used to be GDK_BUTTON1_MASK! Was that intentional? I changed it just in case. Revert on breakage.
            case 2:  return GDK_BUTTON2_MASK; break;
            case 3:  return GDK_BUTTON3_MASK; break;
            case 4:  return GDK_BUTTON4_MASK; break;
            case 5:  return GDK_BUTTON5_MASK; break;
            default: return 0;  // Buttons can range at least to 9 but mask defined only to 5.
        }
    };

    // Do event-specific processing.
    switch (event.type) {

        case GDK_SCROLL:
            return emit_event(event);

        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        {
            // Pick the current item as if the button were not pressed...
            q->_state = event.button.state;
            pick_current_item(event);

            // ...then process the event.
            q->_state ^= calc_button_mask();
            bool retval = emit_event(event);

            return retval;
        }

        case GDK_BUTTON_RELEASE:
        {
            // Process the event as if the button were pressed...
            q->_state = event.button.state;
            bool retval = emit_event(event);

            // ...then repick after the button has been released.
            auto event_copy = make_unique_copy(event);
            event_copy->button.state ^= calc_button_mask();
            q->_state = event_copy->button.state;
            pick_current_item(*event_copy);

            return retval;
        }

        case GDK_ENTER_NOTIFY:
            q->_state = event.crossing.state;
            return pick_current_item(event);

        case GDK_LEAVE_NOTIFY:
            q->_state = event.crossing.state;
            // This is needed to remove alignment or distribution snap indicators.
            if (q->_desktop) {
                q->_desktop->snapindicator->remove_snaptarget();
            }
            return pick_current_item(event);

        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
            return emit_event(event);

        case GDK_MOTION_NOTIFY:
            q->_state = event.motion.state;
            pick_current_item(event);
            return emit_event(event);

        default:
            return false;
    }
}

// This function is called by 'process_bucketed_event' to manipulate the state variables relating
// to the current object under the mouse, for example, to generate enter and leave events.
// (A more detailed explanation by Tavmjong follows.)
// --------
// This routine reacts to events from the canvas. It's main purpose is to find the canvas item
// closest to the cursor where the event occurred and then send the event (sometimes modified) to
// that item. The event then bubbles up the canvas item tree until an object handles it. If the
// widget is redrawn, this routine may be called again for the same event.
//
// Canvas items register their interest by connecting to the "event" signal.
// Example in desktop.cpp:
//   canvas_catchall->connect_event(sigc::bind(sigc::ptr_fun(sp_desktop_root_handler), this));
bool
CanvasPrivate::pick_current_item(const GdkEvent &event)
{
    // Ensure geometry is correct.
    auto affine = decoupled_mode ? _store_affine : q->_affine;
    if (q->_need_update || geom_affine != affine) {
        q->_canvas_item_root->update(affine);
        geom_affine = affine;
        q->_need_update = false;
    }

    int button_down = 0;
    if (!q->_all_enter_events) {
        // Only set true in connector-tool.cpp.

        // If a button is down, we'll perform enter and leave events on the
        // current item, but not enter on any other item.  This is more or
        // less like X pointer grabbing for canvas items.
        button_down = q->_state & (GDK_BUTTON1_MASK |
                                   GDK_BUTTON2_MASK |
                                   GDK_BUTTON3_MASK |
                                   GDK_BUTTON4_MASK |
                                   GDK_BUTTON5_MASK);
        if (!button_down) q->_left_grabbed_item = false;
    }

    // Save the event in the canvas.  This is used to synthesize enter and
    // leave events in case the current item changes.  It is also used to
    // re-pick the current item if the current one gets deleted.  Also,
    // synthesize an enter event.
    if (&event != &q->_pick_event) {
        if (event.type == GDK_MOTION_NOTIFY || event.type == GDK_BUTTON_RELEASE) {
            // Convert to GDK_ENTER_NOTIFY

            // These fields have the same offsets in both types of events.
            q->_pick_event.crossing.type       = GDK_ENTER_NOTIFY;
            q->_pick_event.crossing.window     = event.motion.window;
            q->_pick_event.crossing.send_event = event.motion.send_event;
            q->_pick_event.crossing.subwindow  = nullptr;
            q->_pick_event.crossing.x          = event.motion.x;
            q->_pick_event.crossing.y          = event.motion.y;
            q->_pick_event.crossing.mode       = GDK_CROSSING_NORMAL;
            q->_pick_event.crossing.detail     = GDK_NOTIFY_NONLINEAR;
            q->_pick_event.crossing.focus      = false;
            q->_pick_event.crossing.state      = event.motion.state;

            // These fields don't have the same offsets in both types of events.
            if (event.type == GDK_MOTION_NOTIFY) {
                q->_pick_event.crossing.x_root = event.motion.x_root;
                q->_pick_event.crossing.y_root = event.motion.y_root;
            } else {
                q->_pick_event.crossing.x_root = event.button.x_root;
                q->_pick_event.crossing.y_root = event.button.y_root;
            }

        } else {
            q->_pick_event = event;
        }
    }

    if (q->_in_repick) {
        // Don't do anything else if this is a recursive call.
        return false;
    }

    // Find new item
    q->_current_canvas_item_new = nullptr;

    if (q->_pick_event.type != GDK_LEAVE_NOTIFY && q->_canvas_item_root->is_visible()) {
        // Leave notify means there is no current item.
        // Find closest item.
        double x = 0.0;
        double y = 0.0;

        if (q->_pick_event.type == GDK_ENTER_NOTIFY) {
            x = q->_pick_event.crossing.x;
            y = q->_pick_event.crossing.y;
        } else {
            x = q->_pick_event.motion.x;
            y = q->_pick_event.motion.y;
        }

        // If in split mode, look at where cursor is to see if one should pick with outline mode.
        if (q->_split_mode == Inkscape::SplitMode::SPLIT && q->_render_mode != Inkscape::RenderMode::OUTLINE_OVERLAY) {
            if ((q->_split_direction == Inkscape::SplitDirection::NORTH && y > q->_split_position.y()) ||
                (q->_split_direction == Inkscape::SplitDirection::SOUTH && y < q->_split_position.y()) ||
                (q->_split_direction == Inkscape::SplitDirection::WEST  && x > q->_split_position.x()) ||
                (q->_split_direction == Inkscape::SplitDirection::EAST  && x < q->_split_position.x()) ) {
                q->_drawing->setRenderMode(Inkscape::RenderMode::OUTLINE);
            }
        }
        // Convert to world coordinates.
        auto p = Geom::Point(x, y) + q->_pos;
        if (decoupled_mode) {
            p *= _store_affine * q->_affine.inverse();
        }

        q->_current_canvas_item_new = q->_canvas_item_root->pick_item(p);
        // if (q->_current_canvas_item_new) {
        //     std::cout << "  PICKING: FOUND ITEM: " << q->_current_canvas_item_new->get_name() << std::endl;
        // } else {
        //     std::cout << "  PICKING: DID NOT FIND ITEM" << std::endl;
        // }

        // Reset the drawing back to the requested render mode.
        q->_drawing->setRenderMode(q->_render_mode);
    }

    if (q->_current_canvas_item_new == q->_current_canvas_item && !q->_left_grabbed_item) {
        // Current item did not change!
        return false;
    }

    // Synthesize events for old and new current items.
    bool retval = false;
    if (q->_current_canvas_item_new != q->_current_canvas_item &&
        q->_current_canvas_item != nullptr                     &&
        !q->_left_grabbed_item                                 ) {

        GdkEvent new_event;
        new_event = q->_pick_event;
        new_event.type = GDK_LEAVE_NOTIFY;
        new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
        new_event.crossing.subwindow = nullptr;
        q->_in_repick = true;
        retval = emit_event(new_event);
        q->_in_repick = false;
    }

    if (q->_all_enter_events == false) {
        // new_current_item may have been set to nullptr during the call to emitEvent() above.
        if (q->_current_canvas_item_new != q->_current_canvas_item && button_down) {
            q->_left_grabbed_item = true;
            return retval;
        }
    }

    // Handle the rest of cases
    q->_left_grabbed_item = false;
    q->_current_canvas_item = q->_current_canvas_item_new;

    if (q->_current_canvas_item != nullptr) {
        GdkEvent new_event;
        new_event = q->_pick_event;
        new_event.type = GDK_ENTER_NOTIFY;
        new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
        new_event.crossing.subwindow = nullptr;
        retval = emit_event(new_event);
    }

    return retval;
}

// Fires an event at the canvas, after a little pre-processing. Returns true if handled.
bool
CanvasPrivate::emit_event(const GdkEvent &event)
{
    // Handle grabbed items.
    if (q->_grabbed_canvas_item) {
        auto mask = (Gdk::EventMask)0;

        switch (event.type) {
            case GDK_ENTER_NOTIFY:
                mask = Gdk::ENTER_NOTIFY_MASK;
                break;
            case GDK_LEAVE_NOTIFY:
                mask = Gdk::LEAVE_NOTIFY_MASK;
                break;
            case GDK_MOTION_NOTIFY:
                mask = Gdk::POINTER_MOTION_MASK;
                break;
            case GDK_BUTTON_PRESS:
            case GDK_2BUTTON_PRESS:
            case GDK_3BUTTON_PRESS:
                mask = Gdk::BUTTON_PRESS_MASK;
                break;
            case GDK_BUTTON_RELEASE:
                mask = Gdk::BUTTON_RELEASE_MASK;
                break;
            case GDK_KEY_PRESS:
                mask = Gdk::KEY_PRESS_MASK;
                break;
            case GDK_KEY_RELEASE:
                mask = Gdk::KEY_RELEASE_MASK;
                break;
            case GDK_SCROLL:
                mask = Gdk::SCROLL_MASK;
                mask |= Gdk::SMOOTH_SCROLL_MASK;
                break;
            default:
                break;
        }

        if (!(mask & q->_grabbed_event_mask)) {
            return false;
        }
    }

    // Convert to world coordinates. We have two different cases due to different event structures.
    auto conv = [&, this] (double &x, double &y) {
       auto p = Geom::Point(x, y) + q->_pos;
       if (decoupled_mode) {
           p *= _store_affine * q->_affine.inverse();
       }
       x = p.x();
       y = p.y();
    };

    auto event_copy = make_unique_copy(event);

    switch (event.type) {
        case GDK_ENTER_NOTIFY:
        case GDK_LEAVE_NOTIFY:
            conv(event_copy->crossing.x, event_copy->crossing.y);
            break;
        case GDK_MOTION_NOTIFY:
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            conv(event_copy->motion.x, event_copy->motion.y);
            break;
        default:
            break;
    }

    // Block undo/redo while anything is dragged.
    if (event.type == GDK_BUTTON_PRESS && event.button.button == 1) {
        q->_is_dragging = true;
    } else if (event.type == GDK_BUTTON_RELEASE) {
        q->_is_dragging = false;
    }

    if (q->_current_canvas_item) {
        // Choose where to send event.
        auto item = q->_current_canvas_item;

        if (q->_grabbed_canvas_item && !q->_current_canvas_item->is_descendant_of(q->_grabbed_canvas_item)) {
            item = q->_grabbed_canvas_item;
        }

        // Propagate the event up the canvas item hierarchy until handled.
        while (item) {
            if (item->handle_event(event_copy.get())) return true;
            item = item->get_parent();
        }
    }

    return false;
}

/*
 * Protected functions
 */

Geom::IntPoint
Canvas::get_dimensions() const
{
    Gtk::Allocation allocation = get_allocation();
    return {allocation.get_width(), allocation.get_height()};
}

/**
 * Is world point inside canvas area?
 */
bool
Canvas::world_point_inside_canvas(Geom::Point const &world) const
{
    return get_area_world().contains(world.floor());
}

/**
 * Translate point in canvas to world coordinates.
 */
Geom::Point
Canvas::canvas_to_world(Geom::Point const &point) const
{
    return point + _pos;
}

/**
 * Return the area shown in the canvas in world coordinates.
 */
Geom::IntRect
Canvas::get_area_world() const
{
    return Geom::IntRect(_pos, _pos + get_dimensions());
}

/**
 * Set the affine for the canvas.
 */
void
Canvas::set_affine(Geom::Affine const &affine)
{
    if (_affine == affine) {
        return;
    }

    _affine = affine;

    d->add_idle();
    queue_draw();
}

void CanvasPrivate::queue_draw_area(Geom::IntRect &rect)
{
    q->queue_draw_area(rect.left(), rect.top(), rect.width(), rect.height());
}

/**
 * Invalidate drawing and redraw during idle.
 */
void
Canvas::redraw_all()
{
    if (!d->active) {
        // CanvasItems redraw their area when being deleted... which happens when the Canvas is destroyed.
        // We need to ignore their requests!
        return;
    }
    d->updater->reset(); // Empty region (i.e. everything is dirty).
    d->add_idle();
    if (d->prefs.debug_show_unclean) queue_draw();
}

/**
 * Redraw the given area during idle.
 */
void
Canvas::redraw_area(int x0, int y0, int x1, int y1)
{
    if (!d->active) {
        // CanvasItems redraw their area when being deleted... which happens when the Canvas is destroyed.
        // We need to ignore their requests!
        return;
    }

    // Clamp area to Cairo's technically supported max size (-2^30..+2^30-1).
    // This ensures that the rectangle dimensions don't overflow and wrap around.
    constexpr int min_coord = -(1 << 30);
    constexpr int max_coord = (1 << 30) - 1;

    x0 = std::clamp(x0, min_coord, max_coord);
    y0 = std::clamp(y0, min_coord, max_coord);
    x1 = std::clamp(x1, min_coord, max_coord);
    y1 = std::clamp(y1, min_coord, max_coord);

    if (x0 >= x1 || y0 >= y1) {
        return;
    }

    auto rect = Geom::IntRect::from_xywh(x0, y0, x1 - x0, y1 - y0);
    d->updater->mark_dirty(rect);
    d->add_idle();
    if (d->prefs.debug_show_unclean) queue_draw();
}

void
Canvas::redraw_area(Geom::Coord x0, Geom::Coord y0, Geom::Coord x1, Geom::Coord y1)
{
    // Handle overflow during conversion gracefully.
    // Round outward to make sure integral coordinates cover the entire area.
    constexpr Geom::Coord min_int = std::numeric_limits<int>::min();
    constexpr Geom::Coord max_int = std::numeric_limits<int>::max();

    redraw_area(
        (int)std::floor(std::clamp(x0, min_int, max_int)),
        (int)std::floor(std::clamp(y0, min_int, max_int)),
        (int)std::ceil (std::clamp(x1, min_int, max_int)),
        (int)std::ceil (std::clamp(y1, min_int, max_int))
    );
}

void
Canvas::redraw_area(Geom::Rect &area)
{
    redraw_area(area.left(), area.top(), area.right(), area.bottom());
}

/**
 * Redraw after changing canvas item geometry.
 */
void
Canvas::request_update()
{
    // Flag geometry as needing update.
    _need_update = true;

    // Trigger the idle process to perform the update.
    d->add_idle();
}

/**
 * Scroll window so drawing point 'pos' is at upper left corner of canvas.
 */
void
Canvas::set_pos(Geom::IntPoint const &pos)
{
    if (pos == _pos) {
        return;
    }

    _pos = pos;

    d->add_idle();
    queue_draw();

    if (auto grid = dynamic_cast<Inkscape::UI::Widget::CanvasGrid*>(get_parent())) {
        grid->UpdateRulers();
    }
}

/**
 * Set canvas background color (display only).
 */
void
Canvas::set_background_color(guint32 rgba)
{
    double r = SP_RGBA32_R_F(rgba);
    double g = SP_RGBA32_G_F(rgba);
    double b = SP_RGBA32_B_F(rgba);

    _background = Cairo::SolidPattern::create_rgb(r, g, b);
    d->solid_background = true;

    redraw_all();
}

/**
 * Set canvas background to a checkerboard pattern.
 */
void
Canvas::set_background_checkerboard(guint32 rgba, bool use_alpha)
{
    auto pattern = ink_cairo_pattern_create_checkerboard(rgba, use_alpha);
    _background = Cairo::RefPtr<Cairo::Pattern>(new Cairo::Pattern(pattern));
    d->solid_background = false;
    redraw_all();
}

void Canvas::set_drawing_disabled(bool disable)
{
    _drawing_disabled = disable;
    if (!disable) {
        d->add_idle();
    }
}

void
Canvas::set_render_mode(Inkscape::RenderMode mode)
{
    if (_render_mode != mode) {
        _render_mode = mode;
        _drawing->setRenderMode(_render_mode);
        redraw_all();
    }
    if (_desktop) {
        _desktop->setWindowTitle(); // Mode is listed in title.
    }
}

void
Canvas::set_color_mode(Inkscape::ColorMode mode)
{
    if (_color_mode != mode) {
        _color_mode = mode;
        redraw_all();
    }
    if (_desktop) {
        _desktop->setWindowTitle(); // Mode is listed in title.
    }
}

void
Canvas::set_split_mode(Inkscape::SplitMode mode)
{
    if (_split_mode != mode) {
        _split_mode = mode;
        redraw_all();
    }
}

Cairo::RefPtr<Cairo::ImageSurface>
Canvas::get_backing_store() const
{
    return d->_backing_store;
}

/**
 * Clear current and grabbed items.
 */
void
Canvas::canvas_item_destructed(Inkscape::CanvasItem* item)
{
    if (item == _current_canvas_item) {
        _current_canvas_item = nullptr;
    }

    if (item == _current_canvas_item_new) {
        _current_canvas_item_new = nullptr;
    }

    if (item == _grabbed_canvas_item) {
        _grabbed_canvas_item = nullptr;
        auto const display = Gdk::Display::get_default();
        auto const seat    = display->get_default_seat();
        seat->ungrab();
    }
}

// Change cursor
void
Canvas::set_cursor() {

    if (!_desktop) {
        return;
    }

    auto display = Gdk::Display::get_default();

    switch (_hover_direction) {

        case Inkscape::SplitDirection::NONE:
            _desktop->event_context->use_tool_cursor();
            break;

        case Inkscape::SplitDirection::NORTH:
        case Inkscape::SplitDirection::EAST:
        case Inkscape::SplitDirection::SOUTH:
        case Inkscape::SplitDirection::WEST:
        {
            auto cursor = Gdk::Cursor::create(display, "pointer");
            get_window()->set_cursor(cursor);
            break;
        }

        case Inkscape::SplitDirection::HORIZONTAL:
        {
            auto cursor = Gdk::Cursor::create(display, "ns-resize");
            get_window()->set_cursor(cursor);
            break;
        }

        case Inkscape::SplitDirection::VERTICAL:
        {
            auto cursor = Gdk::Cursor::create(display, "ew-resize");
            get_window()->set_cursor(cursor);
            break;
        }

        default:
            // Shouldn't reach.
            std::cerr << "Canvas::set_cursor: Unknown hover direction!" << std::endl;
    }
}

void
Canvas::get_preferred_width_vfunc(int &minimum_width,  int &natural_width) const
{
    minimum_width = natural_width = 256;
}

void
Canvas::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    minimum_height = natural_height = 256;
}

void
Canvas::on_size_allocate(Gtk::Allocation &allocation)
{
    parent_type::on_size_allocate(allocation);
    assert(allocation == get_allocation());
    d->add_idle(); // Trigger the size update to be applied to the stores before the next call to on_draw.
}

/*
 * Drawing
 */

/*
 * The on_draw() function is called whenever Gtk wants to update the window. This function:
 *
 * 1. Ensures that if the idle process was started, at least one cycle has run.
 *
 * 2. Blits the store(s) onto the canvas, clipping the outline store as required.
 *    (Or composites them with the transformed snapshot store(s) in decoupled mode.)
 *
 * 3. Draws the "controller" in the 'split' split mode.
 */
bool
Canvas::on_draw(const Cairo::RefPtr<::Cairo::Context> &cr)
{
    auto f = FrameCheck::Event();

    if (!d->active) {
        std::cerr << "Canvas::on_draw: Called while not active!" << std::endl;
        return true;
    }

    // sp_canvas_item_recursive_print_tree(0, _root);
    // canvas_item_print_tree(_canvas_item_root);

    assert(_drawing);

    // Although hipri_idle is scheduled at a priority higher than draw, and should therefore always be called first if asked, there are times when GTK simply decides to call on_draw anyway.
    // Here we ensure that that call has taken place. This is problematic because if hipri_idle does rendering, enlarging the damage rect, then our drawing will still be clipped to the old
    // damage rect. It was precisely this problem that lead to the introduction of hipri_idle. Fortunately, the following failsafe only seems to execute once during initialisation, and
    // once on further resize events. Both these events seem to trigger a full damage, hence we are ok.
    if (d->hipri_idle.connected()) {
        d->hipri_idle.disconnect();
        d->on_hipri_idle();
    }

    // Blit background if not solid. (If solid, it is baked into the stores.)
    if (!d->solid_background) {
        if (d->prefs.debug_framecheck) f = FrameCheck::Event("background");
        cr->save();
        cr->set_operator(Cairo::OPERATOR_SOURCE);
        cr->set_source(_background);
        cr->paint();
        cr->restore();
    }

    auto draw_store = [&, this] (const Cairo::RefPtr<Cairo::ImageSurface> &store, const Cairo::RefPtr<Cairo::ImageSurface> &snapshot_store, bool is_backing_store) {
        if (!d->decoupled_mode) {
            // Blit store to screen.
            if (d->prefs.debug_framecheck) f = FrameCheck::Event("draw");
            cr->save();
            cr->set_operator(is_backing_store && d->solid_background ? Cairo::OPERATOR_SOURCE : Cairo::OPERATOR_OVER);
            cr->set_source(store, d->_store_rect.left() - _pos.x(), d->_store_rect.top() - _pos.y());
            cr->paint();
            cr->restore();
        } else {
            // Turn off anti-aliasing for huge performance gains. Only applies to this compositing step.
            cr->set_antialias(Cairo::ANTIALIAS_NONE);

            // Blit background to complement of both clean regions, if solid (and therefore not already drawn).
            if (is_backing_store && d->solid_background) {
                if (d->prefs.debug_framecheck) f = FrameCheck::Event("composite", 2);
                cr->save();
                cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
                cr->rectangle(0, 0, get_allocation().get_width(), get_allocation().get_height());
                cr->translate(-_pos.x(), -_pos.y());
                cr->transform(geom_to_cairo(_affine * d->_store_affine.inverse()));
                region_to_path(cr, d->updater->clean_region);
                cr->transform(geom_to_cairo(d->_store_affine * d->_snapshot_affine.inverse()));
                region_to_path(cr, d->_snapshot_clean_region);
                cr->clip();
                cr->set_source(_background);
                cr->set_operator(Cairo::OPERATOR_SOURCE);
                Cairo::SurfacePattern(cr->get_source()->cobj()).set_filter(Cairo::FILTER_FAST);
                cr->paint();
                cr->restore();
            }

            // Draw transformed snapshot, clipped to its clean region and the complement of the store's clean region.
            if (d->prefs.debug_framecheck) f = FrameCheck::Event("composite", 1);
            cr->save();
            cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
            cr->rectangle(0, 0, get_allocation().get_width(), get_allocation().get_height());
            cr->translate(-_pos.x(), -_pos.y());
            cr->transform(geom_to_cairo(_affine * d->_store_affine.inverse()));
            region_to_path(cr, d->updater->clean_region);
            cr->clip();
            cr->transform(geom_to_cairo(d->_store_affine * d->_snapshot_affine.inverse()));
            region_to_path(cr, d->_snapshot_clean_region);
            cr->clip();
            cr->set_source(snapshot_store, d->_snapshot_rect.left(), d->_snapshot_rect.top());
            cr->set_operator(is_backing_store && d->solid_background ? Cairo::OPERATOR_SOURCE : Cairo::OPERATOR_OVER);
            Cairo::SurfacePattern(cr->get_source()->cobj()).set_filter(Cairo::FILTER_FAST);
            cr->paint();
            if (d->prefs.debug_show_snapshot) {
                cr->set_source_rgba(0, 0, 1, 0.2);
                cr->set_operator(Cairo::OPERATOR_OVER);
                cr->paint();
            }
            cr->restore();

            // Draw transformed store, clipped to clean region.
            if (d->prefs.debug_framecheck) f = FrameCheck::Event("composite", 0);
            cr->save();
            cr->translate(-_pos.x(), -_pos.y());
            cr->transform(geom_to_cairo(_affine * d->_store_affine.inverse()));
            region_to_path(cr, d->updater->clean_region);
            cr->clip();
            cr->set_source(store, d->_store_rect.left(), d->_store_rect.top());
            cr->set_operator(is_backing_store && d->solid_background ? Cairo::OPERATOR_SOURCE : Cairo::OPERATOR_OVER);
            Cairo::SurfacePattern(cr->get_source()->cobj()).set_filter(Cairo::FILTER_FAST);
            cr->paint();
            cr->restore();
        }
    };

    // Draw the backing store.
    draw_store(d->_backing_store, d->_snapshot_store, true);

    // Draw overlay if required.
    if (_render_mode == Inkscape::RenderMode::OUTLINE_OVERLAY) {
        assert(d->_outline_store);

        double outline_overlay_opacity = 1.0 - d->prefs.outline_overlay_opacity / 100.0;

        // Partially obscure drawing by painting semi-transparent white.
        cr->set_source_rgb(1.0, 1.0, 1.0);
        cr->paint_with_alpha(outline_overlay_opacity);

        // Overlay outline.
        draw_store(d->_outline_store, d->_snapshot_outline_store, false);
    }

    // Draw split if required.
    if (_split_mode != Inkscape::SplitMode::NORMAL) {
        assert(d->_outline_store);

        // Move split position to center if not in canvas.
        auto const rect = Geom::Rect(Geom::Point(), get_dimensions());
        if (!rect.contains(_split_position)) {
            _split_position = rect.midpoint();
        }

        // Add clipping path and blit background.
        cr->save();
        cr->set_operator(Cairo::OPERATOR_SOURCE);
        cr->set_source(_background);
        add_clippath(cr);
        cr->paint();
        cr->restore();

        // Add clipping path and draw outline store.
        cr->save();
        add_clippath(cr);
        draw_store(d->_outline_store, d->_snapshot_outline_store, false);
        cr->restore();
    }

    // Paint unclean regions in red.
    if (d->prefs.debug_show_unclean) {
        if (d->prefs.debug_framecheck) f = FrameCheck::Event("paint_unclean");
        auto reg = Cairo::Region::create(geom_to_cairo(d->_store_rect));
        reg->subtract(d->updater->clean_region);
        cr->save();
        cr->translate(-_pos.x(), -_pos.y());
        if (d->decoupled_mode) {
            cr->transform(geom_to_cairo(_affine * d->_store_affine.inverse()));
        }
        cr->set_source_rgba(1, 0, 0, 0.2);
        region_to_path(cr, reg);
        cr->fill();
        cr->restore();
    }

    // Paint internal edges of clean region in green.
    if (d->prefs.debug_show_clean) {
        if (d->prefs.debug_framecheck) f = FrameCheck::Event("paint_clean");
        cr->save();
        cr->translate(-_pos.x(), -_pos.y());
        if (d->decoupled_mode) {
            cr->transform(geom_to_cairo(_affine * d->_store_affine.inverse()));
        }
        cr->set_source_rgba(0, 0.7, 0, 0.4);
        region_to_path(cr, d->updater->clean_region);
        cr->stroke();
        cr->restore();
    }

    if (_split_mode == Inkscape::SplitMode::SPLIT) {
        // Add dividing line.
        cr->save();
        cr->set_source_rgb(0, 0, 0);
        cr->set_line_width(1);
        if (_split_direction == Inkscape::SplitDirection::EAST ||
            _split_direction == Inkscape::SplitDirection::WEST) {
            cr->move_to((int)_split_position.x() + 0.5,                    0);
            cr->line_to((int)_split_position.x() + 0.5, get_dimensions().y());
            cr->stroke();
        } else {
            cr->move_to(                   0, (int)_split_position.y() + 0.5);
            cr->line_to(get_dimensions().x(), (int)_split_position.y() + 0.5);
            cr->stroke();
        }
        cr->restore();

        // Add controller image.
        double a = _hover_direction == Inkscape::SplitDirection::NONE ? 0.5 : 1.0;
        cr->save();
        cr->set_source_rgba(0.2, 0.2, 0.2, a);
        cr->arc(_split_position.x(), _split_position.y(), 20 * d->_device_scale, 0, 2 * M_PI);
        cr->fill();
        cr->restore();

        cr->save();
        for (int i = 0; i < 4; ++i) {
            // The four direction triangles.
            cr->save();

            // Position triangle.
            cr->translate(_split_position.x(), _split_position.y());
            cr->rotate((i + 2) * M_PI / 2.0);

            // Draw triangle.
            cr->move_to(-5 * d->_device_scale,  8 * d->_device_scale);
            cr->line_to( 0,                    18 * d->_device_scale);
            cr->line_to( 5 * d->_device_scale,  8 * d->_device_scale);
            cr->close_path();

            double b = (int)_hover_direction == (i + 1) ? 0.9 : 0.7;
            cr->set_source_rgba(b, b, b, a);
            cr->fill();

            cr->restore();
        }
        cr->restore();
    }

    // Process bucketed events as soon as possible after draw. We cannot process them now, because we have
    // a frame to get out as soon as possible, and processing events may take a while. Instead, we schedule
    // it with a signal callback on the main loop that runs as soon as this function is completed.
    if (!d->eventprocessor->events.empty()) d->schedule_bucket_emptier();

    // Record the fact that a draw is no longer pending.
    d->pending_draw = false;

    // Notify the update strategy that another frame has passed.
    d->updater->frame();

    return true;
}

// Sets clip path for Split and X-Ray modes.
void
Canvas::add_clippath(const Cairo::RefPtr<Cairo::Context>& cr)
{
    double width  = get_allocation().get_width();
    double height = get_allocation().get_height();
    double sx     = _split_position.x();
    double sy     = _split_position.y();

    if (_split_mode == Inkscape::SplitMode::SPLIT) {
        // We're clipping the outline region... so it's backwards.
        switch (_split_direction) {
            case Inkscape::SplitDirection::SOUTH:
                cr->rectangle(0,   0, width,               sy);
                break;
            case Inkscape::SplitDirection::NORTH:
                cr->rectangle(0,  sy, width,      height - sy);
                break;
            case Inkscape::SplitDirection::EAST:
                cr->rectangle(0,   0,         sx, height     );
                break;
            case Inkscape::SplitDirection::WEST:
                cr->rectangle(sx,  0, width - sx, height     );
                break;
            default:
                // no clipping (for NONE, HORIZONTAL, VERTICAL)
                break;
        }
    } else {
        cr->arc(sx, sy, d->prefs.x_ray_radius, 0, 2 * M_PI);
    }

    cr->clip();
}

void
CanvasPrivate::add_idle()
{
    framecheck_whole_function(this)

    if (!active) {
        // We can safely discard events until active, because we will run add_idle on activation later in initialisation.
        return;
    }

    if (!hipri_idle.connected()) {
        hipri_idle = Glib::signal_idle().connect(sigc::mem_fun(this, &CanvasPrivate::on_hipri_idle), G_PRIORITY_HIGH_IDLE + 15); // after resize, before draw
    }

    if (!lopri_idle.connected()) {
        lopri_idle = Glib::signal_idle().connect(sigc::mem_fun(this, &CanvasPrivate::on_lopri_idle), G_PRIORITY_DEFAULT_IDLE);
    }

    idle_running = true;
}

auto
distSq(const Geom::IntPoint pt, const Geom::IntRect &rect)
{
    auto v = rect.clamp(pt) - pt;
    return v.x() * v.x() + v.y() * v.y();
}

auto
calc_affine_diff(const Geom::Affine &a, const Geom::Affine &b) {
    auto c = a.inverse() * b;
    return std::abs(c[0] - 1) + std::abs(c[1]) + std::abs(c[2]) + std::abs(c[3] - 1);
}

// Replace a region with a larger region consisting of fewer, larger rectangles. (Allowed to slightly overlap.)
auto
coarsen(const Cairo::RefPtr<Cairo::Region> &region, int min_size, int glue_size, double min_fullness)
{
    // Sort the rects by minExtent.
    struct Compare
    {
        bool operator()(const Geom::IntRect &a, const Geom::IntRect &b) const {
            return a.minExtent() < b.minExtent();
        }
    };
    std::multiset<Geom::IntRect, Compare> rects;
    int nrects = region->get_num_rectangles();
    for (int i = 0; i < nrects; i++) {
        rects.emplace(cairo_to_geom(region->get_rectangle(i)));
    }

    // List of processed rectangles.
    std::vector<Geom::IntRect> processed;
    processed.reserve(nrects);

    // Removal lists.
    std::vector<decltype(rects)::iterator> remove_rects;
    std::vector<int> remove_processed;

    // Repeatedly expand small rectangles by absorbing their nearby small rectangles.
    while (!rects.empty() && rects.begin()->minExtent() < min_size) {
        // Extract the smallest unprocessed rectangle.
        auto rect = *rects.begin();
        rects.erase(rects.begin());

        // Initialise the effective glue size.
        int effective_glue_size = glue_size;

        while (true) {
            // Find the glue zone.
            auto glue_zone = rect;
            glue_zone.expandBy(effective_glue_size);

            // Absorb rectangles in the glue zone. We could do better algorithmically speaking, but in real life it's already plenty fast.
            auto newrect = rect;
            int absorbed_area = 0;

            remove_rects.clear();
            for (auto it = rects.begin(); it != rects.end(); ++it) {
                if (glue_zone.contains(*it)) {
                    newrect.unionWith(*it);
                    absorbed_area += it->area();
                    remove_rects.emplace_back(it);
                }
            }

            remove_processed.clear();
            for (int i = 0; i < processed.size(); i++) {
                auto &r = processed[i];
                if (glue_zone.contains(r)) {
                    newrect.unionWith(r);
                    absorbed_area += r.area();
                    remove_processed.emplace_back(i);
                }
            }

            // If the result was too empty, try again with a smaller glue size.
            double fullness = (double)(rect.area() + absorbed_area) / newrect.area();
            if (fullness < min_fullness) {
                effective_glue_size /= 2;
                continue;
            }

            // Commit the change.
            rect = newrect;

            for (auto &it : remove_rects) {
                rects.erase(it);
            }

            for (int j = (int)remove_processed.size() - 1; j >= 0; j--) {
                int i = remove_processed[j];
                processed[i] = processed.back();
                processed.pop_back();
            }

            // Stop growing if not changed or now big enough.
            bool finished = absorbed_area == 0 || rect.minExtent() >= min_size;
            if (finished) {
                break;
            }

            // Otherwise, continue normally.
            effective_glue_size = glue_size;
        }

        // Put the finished rectangle in processed.
        processed.emplace_back(rect);
    }

    // Put any remaining rectangles in processed.
    for (auto &rect : rects) {
        processed.emplace_back(rect);
    }

    return processed;
}

std::optional<Geom::Dim2>
CanvasPrivate::old_bisector(const Geom::IntRect &rect)
{
    int bw = rect.width();
    int bh = rect.height();

    /*
     * Determine redraw strategy:
     *
     * bw < bh (strips mode): Draw horizontal strips starting from cursor position.
     *                        Seems to be faster for drawing many smaller objects zoomed out.
     *
     * bw > hb (chunks mode): Splits across the larger dimension of the rectangle, painting
     *                        in almost square chunks (from the cursor.
     *                        Seems to be faster for drawing a few blurred objects across the entire screen.
     *                        Seems to be somewhat psychologically faster.
     *
     * Default is for strips mode.
     */

    int max_pixels;
    if (q->_render_mode != Inkscape::RenderMode::OUTLINE) {
        // Can't be too small or large gradient will be rerendered too many times!
        max_pixels = 65536 * prefs.tile_multiplier;
    } else {
        // Paths only. 1M is catched buffer and we need four channels.
        max_pixels = 262144;
    }

    if (bw * bh > max_pixels) {
        if (bw < bh || bh < 2 * prefs.tile_size) {
            return Geom::X;
        } else {
            return Geom::Y;
        }
    }

    return {};
}

std::optional<Geom::Dim2>
CanvasPrivate::new_bisector(const Geom::IntRect &rect)
{
    int bw = rect.width();
    int bh = rect.height();

    // Chop in half along the bigger dimension if the bigger dimension is too big.
    if (bw > bh) {
        if (bw > prefs.new_bisector_size) {
            return Geom::X;
        }
    } else {
        if (bh > prefs.new_bisector_size) {
            return Geom::Y;
        }
    }

    return {};
}

bool
CanvasPrivate::on_hipri_idle()
{
    assert(active);
    if (idle_running) {
        idle_running = on_idle();
    }
    return false;
}

bool
CanvasPrivate::on_lopri_idle()
{
    assert(active);
    if (idle_running) {
        idle_running = on_idle();
    }
    return idle_running;
}

bool
CanvasPrivate::on_idle()
{
    framecheck_whole_function(this)

    assert(q->_canvas_item_root);

    // Quit idle process if not supposed to be drawing.
    if (!q->_drawing || q->_drawing_disabled) {
        return false;
    }

    const Geom::IntPoint pad(prefs.pad, prefs.pad);
    auto recreate_store = [&, this] {
        // Recreate the store at the current affine so that it covers the visible region.
        _store_rect = q->get_area_world();
        _store_rect.expandBy(pad);
        _store_affine = q->_affine;
        int desired_width  = _store_rect.width()  * _device_scale;
        int desired_height = _store_rect.height() * _device_scale;
        if (!_backing_store || _backing_store->get_width() != desired_width || _backing_store->get_height() != desired_height) {
            _backing_store = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, desired_width, desired_height);
            cairo_surface_set_device_scale(_backing_store->cobj(), _device_scale, _device_scale); // No C++ API!
        }
        auto cr = Cairo::Context::create(_backing_store);
        if (solid_background) {
            cr->set_operator(Cairo::OPERATOR_SOURCE);
            cr->set_source(q->_background);
        } else {
            cr->set_operator(Cairo::OPERATOR_CLEAR);
        }
        cr->paint();
        if (need_outline_store()) {
            if (!_outline_store || _outline_store->get_width() != desired_width || _outline_store->get_height() != desired_height) {
                _outline_store = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, desired_width, desired_height);
                cairo_surface_set_device_scale(_outline_store->cobj(), _device_scale, _device_scale); // No C++ API!
            }
            auto cr = Cairo::Context::create(_outline_store);
            cr->set_operator(Cairo::OPERATOR_CLEAR);
            cr->paint();
        }
        updater->reset();
        if (prefs.debug_show_unclean) q->queue_draw();
    };

    // Determine whether the rendering parameters have changed, and reset if so.
    if (!_backing_store || (need_outline_store() && !_outline_store) || _device_scale != q->get_scale_factor() || _store_solid_background != solid_background) {
        _device_scale = q->get_scale_factor();
        _store_solid_background = solid_background;
        recreate_store();
        decoupled_mode = false;
        if (prefs.debug_logging) std::cout << "Full reset" << std::endl;
    }

    // Make sure to clear the outline store when not in use, so we don't accidentally re-use it when it is required again.
    if (!need_outline_store()) {
        _outline_store.clear();
    }

    auto shift_store = [&, this] {
        // Recreate the store, but keep re-usable content from the old store.
        auto store_rect = q->get_area_world();
        store_rect.expandBy(pad);
        auto backing_store = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, store_rect.width() * _device_scale, store_rect.height() * _device_scale);
        cairo_surface_set_device_scale(backing_store->cobj(), _device_scale, _device_scale); // No C++ API!

        // Determine the geometry of the shift.
        auto shift = store_rect.min() - _store_rect.min();
        auto reuse_rect = store_rect & _store_rect;
        assert(reuse_rect); // Should not be called if there is no overlap.
        auto cr = Cairo::Context::create(backing_store);

        // Paint background into region not covered by next operation.
        if (solid_background) {
            auto reg = Cairo::Region::create(geom_to_cairo(store_rect));
            reg->subtract(geom_to_cairo(*reuse_rect));
            reg->translate(-store_rect.left(), -store_rect.top());
            cr->save();
            if (solid_background) {
                cr->set_operator(Cairo::OPERATOR_SOURCE);
                cr->set_source(q->_background);
            }
            region_to_path(cr, reg);
            cr->fill();
            cr->restore();
        }

        // Copy re-usuable contents of old store into new store, shifted.
        cr->rectangle(reuse_rect->left() - store_rect.left(), reuse_rect->top() - store_rect.top(), reuse_rect->width(), reuse_rect->height());
        cr->clip();
        cr->set_source(_backing_store, -shift.x(), -shift.y());
        cr->set_operator(Cairo::OPERATOR_SOURCE);
        cr->paint();

        // Set the result as the new backing store.
        _store_rect = store_rect;
        assert(_store_affine == q->_affine); // Should not be called if the affine has changed.
        _backing_store = std::move(backing_store);

        // Do the same for the outline store
        if (_outline_store) {
            auto outline_store = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, store_rect.width() * _device_scale, store_rect.height() * _device_scale);
            cairo_surface_set_device_scale(outline_store->cobj(), _device_scale, _device_scale); // No C++ API!
            auto cr = Cairo::Context::create(outline_store);
            cr->rectangle(reuse_rect->left() - store_rect.left(), reuse_rect->top() - store_rect.top(), reuse_rect->width(), reuse_rect->height());
            cr->clip();
            cr->set_source(_outline_store, -shift.x(), -shift.y());
            cr->set_operator(Cairo::OPERATOR_SOURCE);
            cr->paint();
            _outline_store = std::move(outline_store);
        }

        updater->intersect(_store_rect);
        if (prefs.debug_show_unclean) q->queue_draw();
    };

    auto take_snapshot = [&, this] {
        // Copy the backing store to the snapshot, leaving us temporarily in an invalid state.
        std::swap(_snapshot_store, _backing_store); // This will re-use the old snapshot store later if possible.
        _snapshot_rect = _store_rect;
        _snapshot_affine = _store_affine;
        _snapshot_clean_region = updater->clean_region->copy();

        // Do the same for the outline store
        std::swap(_snapshot_outline_store, _outline_store);

        // Recreate the backing store, making the state valid again.
        recreate_store();
    };

    // Handle transitions and actions in response to viewport changes.
    if (!decoupled_mode) {
        // Enter decoupled mode if the affine has changed from what the backing store was drawn at.
        if (q->_affine != _store_affine) {
            // Snapshot and reset the backing store.
            take_snapshot();

            // Enter decoupled mode.
            if (prefs.debug_logging) std::cout << "Entering decoupled mode" << std::endl;
            decoupled_mode = true;

            // Note: If redrawing is fast enough to finish during the frame, then going into decoupled mode, drawing, and leaving
            // it again performs exactly the same rendering operations as if we had not gone into it at all. Also, no extra copies
            // or blits are performed, and the drawing operations done on the screen are the same. Hence this feature comes at zero cost.
        } else {
            // Get visible rectangle in canvas coordinates.
            auto const visible = q->get_area_world();
            if (!_store_rect.intersects(visible)) {
                // If the store has gone completely off-screen, recreate it.
                recreate_store();
                if (prefs.debug_logging) std::cout << "Recreated store" << std::endl;
            } else if (!_store_rect.contains(visible)) {
                // If the store has gone partially off-screen, shift it.
                shift_store();
                if (prefs.debug_logging) std::cout << "Shifted store" << std::endl;
            }
            // After these operations, the store should now be fully on-screen.
            assert(_store_rect.contains(visible));
        }
    } else { // if (decoupled_mode)
        // Completely cancel the previous redraw and start again if the viewing parameters have changed too much.
        if (!prefs.debug_sticky_decoupled) {
            auto pl = Geom::Parallelogram(q->get_area_world());
            pl *= _store_affine * q->_affine.inverse();
            if (!pl.intersects(_store_rect)) {
                // Store has gone off the screen.
                recreate_store();
                if (prefs.debug_logging) std::cout << "Restarting redraw (store off-screen)" << std::endl;
            } else {
                auto diff = calc_affine_diff(q->_affine, _store_affine);
                if (diff > prefs.max_affine_diff) {
                    // Affine has changed too much.
                    recreate_store();
                    if (prefs.debug_logging) std::cout << "Restarting redraw (affine changed too much)" << std::endl;
                }
            }
        }
    }

    // Assert that _clean_region is a subregion of _store_rect.
    #ifndef NDEBUG
    auto tmp = updater->clean_region->copy();
    tmp->subtract(geom_to_cairo(_store_rect));
    assert(tmp->empty());
    #endif

    // Ensure the geometry is up-to-date and in the right place.
    auto affine = decoupled_mode ? _store_affine : q->_affine;
    if (q->_need_update || geom_affine != affine) {
        q->_canvas_item_root->update(affine);
        geom_affine = affine;
        q->_need_update = false;
    }

    // If asked to, don't paint anything and instead halt the idle process.
    if (prefs.debug_disable_redraw) {
        return false;
    }

    // Get the subrectangle of store that is visible.
    Geom::OptIntRect visible_rect;
    if (!decoupled_mode) {
        // By a previous assertion, this always lies within the store.
        visible_rect = q->get_area_world();
    } else {
        // Get the window rectangle transformed into canvas space.
        auto pl = Geom::Parallelogram(q->get_area_world());
        pl *= _store_affine * q->_affine.inverse();

        // Get its bounding box, rounded outwards.
        auto b = pl.bounds();
        auto bi = Geom::IntRect(b.min().floor(), b.max().ceil());

        // The visible rect is the intersection of this with the store
        visible_rect = bi & _store_rect;
    }
    // The visible rectangle must be a subrectangle of store.
    assert(_store_rect.contains(visible_rect));

    // Get the mouse position in screen space.
    Geom::IntPoint mouse_loc = (last_mouse ? *last_mouse : Geom::Point(q->get_dimensions()) / 2).round();

    // Map the mouse to canvas space.
    mouse_loc += q->_pos;
    if (decoupled_mode) {
        mouse_loc = geom_act(_store_affine * q->_affine.inverse(), mouse_loc);
    }

    // Begin processing redraws.
    auto start_time = g_get_monotonic_time();
    while (true) {
        // Get the clean region for the next redraw as reported by the updater.
        auto clean_region = updater->get_next_clean_region();

        // Get the region to paint, which is the visible rectangle minus the clean region (both subregions of store).
        Cairo::RefPtr<Cairo::Region> paint_region;
        if (visible_rect) {
            paint_region = Cairo::Region::create(geom_to_cairo(*visible_rect));
            paint_region->subtract(clean_region);
        } else {
            paint_region = Cairo::Region::create();
        }

        // Get the list of rectangles to paint, coarsened to avoid fragmentation.
        auto rects = coarsen(paint_region,
                             std::min<int>(prefs.coarsener_min_size, prefs.new_bisector_size / 2),
                             std::min<int>(prefs.coarsener_glue_size, prefs.new_bisector_size / 2),
                             prefs.coarsener_min_fullness);

        // Ensure that all the rectangles lie within the visible rect (and therefore within the store).
        #ifndef NDEBUG
        for (auto &rect : rects) {
            assert(visible_rect.contains(rect));
        }
        #endif

        // Put the rectangles into a heap sorted by distance from mouse.
        auto cmp = [&] (const Geom::IntRect &a, const Geom::IntRect &b) {
            return distSq(mouse_loc, a) > distSq(mouse_loc, b);
        };
        std::make_heap(rects.begin(), rects.end(), cmp);

        // Process rectangles until none left or timed out.
        while (!rects.empty()) {
            // Extract the closest rectangle to the mouse.
            std::pop_heap(rects.begin(), rects.end(), cmp);
            auto rect = rects.back();
            rects.pop_back();

            // Cull empty rectangles.
            if (rect.width() == 0 || rect.height() == 0) {
                continue;
            }

            // Cull rectangles that lie entirely inside the clean region.
            // (These can be generated by coarsening; they must be discarded to avoid getting stuck re-rendering the same rectangles.)
            if (clean_region->contains_rectangle(geom_to_cairo(rect)) == Cairo::REGION_OVERLAP_IN) {
                continue;
            }

            // Lambda to add a rectangle to the heap.
            auto add_rect = [&] (const Geom::IntRect &rect) {
                rects.emplace_back(rect);
                std::push_heap(rects.begin(), rects.end(), cmp);
            };

            // If the rectangle needs bisecting, bisect it and put it back on the heap.
            auto axis = prefs.use_new_bisector ? new_bisector(rect) : old_bisector(rect);
            if (axis) {
                int mid = rect[*axis].middle();
                auto lo = rect; lo[*axis].setMax(mid); add_rect(lo);
                auto hi = rect; hi[*axis].setMin(mid); add_rect(hi);
                continue;
            }

            // Paint the rectangle.
            paint_rect_internal(rect);

            // Check for timeout.
            auto now = g_get_monotonic_time();
            auto elapsed = now - start_time;
            if (elapsed > prefs.render_time_limit) {
                // Timed out. Temporarily return to GTK main loop, and come back here when next idle.
                if (prefs.debug_logging) std::cout << "Timed out: " << g_get_monotonic_time() - start_time << " us" << std::endl;
                framecheckobj.subtype = 1;
                return true;
            }
        }

        // Report the redraw as finished. Exit if there's no more redraws to process.
        bool keep_going = updater->report_finished();
        if (!keep_going) break;
    }

    // Finished drawing. Handle transitions out of decoupled mode, by checking if we need to do a final redraw at the correct affine.
    if (decoupled_mode) {
        if (prefs.debug_sticky_decoupled) {
            // Debug feature: quit idle process, but stay in decoupled mode.
            return false;
        } else if (_store_affine == q->_affine) {
            // Content is rendered at the correct affine - exit decoupled mode and quit idle process.
            if (prefs.debug_logging) std::cout << "Finished drawing - exiting decoupled mode" << std::endl;
            // Exit decoupled mode.
            decoupled_mode = false;
            // Quit idle process.
            return false;
        } else {
            // Content is rendered at the wrong affine - take a new snapshot and continue idle process to continue rendering at the new affine.
            if (prefs.debug_logging) std::cout << "Scheduling final redraw" << std::endl;
            // Snapshot and reset the backing store.
            take_snapshot();
            // Continue idle process.
            return true;
        }
    } else {
        // All done, quit the idle process.
        framecheckobj.subtype = 3;
        return false;
    }
}

void
CanvasPrivate::paint_rect_internal(Geom::IntRect const &rect)
{
    // Paint the rectangle.
    q->_drawing->setColorMode(q->_color_mode);
    paint_single_buffer(rect, _backing_store, true, false);

    if (_outline_store) {
        q->_drawing->setRenderMode(Inkscape::RenderMode::OUTLINE);
        paint_single_buffer(rect, _outline_store, false, q->_render_mode == Inkscape::RenderMode::OUTLINE_OVERLAY);
        q->_drawing->setRenderMode(q->_render_mode); // Leave the drawing in the requested render mode.
    }

    // Introduce an artificial delay for each rectangle.
    if (prefs.debug_slow_redraw) g_usleep(prefs.debug_slow_redraw_time);

    // Mark the rectangle as clean.
    updater->mark_clean(rect);

    // Mark the screen dirty.
    if (!decoupled_mode) {
        // Get rectangle needing repaint
        auto repaint_rect = rect - q->_pos;

        // Assert that a repaint actually occurs (guaranteed because we are only asked to paint fully on-screen rectangles)
        auto screen_rect = Geom::IntRect(0, 0, q->get_allocation().get_width(), q->get_allocation().get_height());
        assert(repaint_rect & screen_rect);

        // Schedule repaint
        queue_draw_area(repaint_rect); // Guarantees on_draw will be called in the future.
        if (bucket_emptier_tick_callback) {q->remove_tick_callback(*bucket_emptier_tick_callback); bucket_emptier_tick_callback.reset();}
        pending_draw = true;
    } else {
        // Get rectangle needing repaint (transform into screen space, take bounding box, round outwards)
        auto pl = Geom::Parallelogram(rect);
        pl *= q->_affine * _store_affine.inverse();
        pl *= Geom::Translate(-q->_pos);
        auto b = pl.bounds();
        auto repaint_rect = Geom::IntRect(b.min().floor(), b.max().ceil());

        // Check if repaint is necessary - some rectangles could be entirely off-screen.
        auto screen_rect = Geom::IntRect(0, 0, q->get_allocation().get_width(), q->get_allocation().get_height());
        if (repaint_rect & screen_rect) {
            // Schedule repaint
            queue_draw_area(repaint_rect);
            if (bucket_emptier_tick_callback) {q->remove_tick_callback(*bucket_emptier_tick_callback); bucket_emptier_tick_callback.reset();}
            pending_draw = true;
        }
    }
}

void
CanvasPrivate::paint_single_buffer(Geom::IntRect const &paint_rect, const Cairo::RefPtr<Cairo::ImageSurface> &store, bool is_backing_store, bool outline_overlay_pass)
{
    // Make sure the following code does not go outside of store's data.
    assert(store);
    assert(store->get_format() == Cairo::FORMAT_ARGB32);
    assert(_store_rect.contains(paint_rect)); // FIXME: Observed to fail once when hitting Ctrl+O while Canvas was busy. Haven't managed to reproduce it. Doesn't mean it's fixed.

    // Create temporary surface that draws directly to store.
    store->flush();
    unsigned char *data = store->get_data();
    int stride = store->get_stride();

    // Check we are using the correct device scale.
    double x_scale = 1.0;
    double y_scale = 1.0;
    cairo_surface_get_device_scale(store->cobj(), &x_scale, &y_scale); // No C++ API!
    assert (_device_scale == (int) x_scale);
    assert (_device_scale == (int) y_scale);

    // Move to the correct row.
    data += stride * (paint_rect.top() - _store_rect.top()) * (int)y_scale;
    // Move to the correct column.
    data += 4 * (paint_rect.left() - _store_rect.left()) * (int)x_scale;
    auto imgs = Cairo::ImageSurface::create(data, Cairo::FORMAT_ARGB32,
                                            paint_rect.width()  * _device_scale,
                                            paint_rect.height() * _device_scale,
                                            stride);

    cairo_surface_set_device_scale(imgs->cobj(), _device_scale, _device_scale); // No C++ API!

    auto cr = Cairo::Context::create(imgs);

    // Clear background
    cr->save();
    if (is_backing_store && solid_background) {
        cr->set_source(q->_background);
        cr->set_operator(Cairo::OPERATOR_SOURCE);
    } else {
        cr->set_operator(Cairo::OPERATOR_CLEAR);
    }
    cr->paint();
    cr->restore();

    // Render drawing on top of background.
    if (q->_canvas_item_root->is_visible()) {
        auto buf = Inkscape::CanvasItemBuffer{ paint_rect, _device_scale, outline_overlay_pass, cr };
        q->_canvas_item_root->render(&buf);
    }

    // Paint over newly drawn content with a translucent random colour.
    if (prefs.debug_show_redraw) {
        cr->set_source_rgba((rand() % 255) / 255.0, (rand() % 255) / 255.0, (rand() % 255) / 255.0, 0.2);
        cr->set_operator(Cairo::OPERATOR_OVER);
        cr->rectangle(0, 0, imgs->get_width(), imgs->get_height());
        cr->fill();
    }

    if (q->_cms_active) {
        auto transf = prefs.from_display
                    ? Inkscape::CMSSystem::getDisplayPer(q->_cms_key)
                    : Inkscape::CMSSystem::getDisplayTransform();

        if (transf) {
            imgs->flush();
            auto px = imgs->get_data();
            int stride = imgs->get_stride();
            for (int i = 0; i < paint_rect.height(); i++) {
                auto row = px + i * stride;
                Inkscape::CMSSystem::doTransform(transf, row, row, paint_rect.width());
            }
            imgs->mark_dirty();
        }
    }

    store->mark_dirty();
}

} // namespace Widget
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
