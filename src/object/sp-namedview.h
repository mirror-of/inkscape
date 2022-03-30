// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef INKSCAPE_SP_NAMEDVIEW_H
#define INKSCAPE_SP_NAMEDVIEW_H

/*
 * <sodipodi:namedview> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "attributes.h"
#include "sp-object-group.h"
#include "snap.h"
#include "document.h"
#include "util/units.h"
#include "svg/svg-bool.h"
#include <vector>

namespace Inkscape {
    class CanvasPage;
    class CanvasGrid;
    namespace Util {
        class Unit;
    }
}

typedef unsigned int guint32;
typedef guint32 GQuark;

enum {
    SP_BORDER_LAYER_BOTTOM,
    SP_BORDER_LAYER_TOP
};

class SPNamedView : public SPObjectGroup {
public:
    SPNamedView();
    ~SPNamedView() override;

    unsigned int editable : 1;

    SVGBool showguides;
    SVGBool lockguides;
    SVGBool grids_visible;

    guint32 desk_color;
    SVGBool desk_checkerboard;

    double zoom;
    double rotation; // Document rotation in degrees (positive is clockwise)
    double cx;
    double cy;
    int window_width;
    int window_height;
    int window_x;
    int window_y;
    int window_maximized;

    SnapManager snap_manager;
    std::vector<Inkscape::CanvasGrid *> grids;

    Inkscape::Util::Unit const *display_units;   // Units used for the UI (*not* the same as units of SVG coordinates)
    // Inkscape::Util::Unit const *page_size_units; // Only used in "Custom size" part of Document Properties dialog 
    
    GQuark default_layer_id;

    double connector_spacing;

    guint32 guidecolor;
    guint32 guidehicolor;

    std::vector<SPGuide *> guides;
    std::vector<SPDesktop *> views;

    int viewcount;

    void show(SPDesktop *desktop);
    void hide(SPDesktop const *desktop);
    void setDefaultAttribute(std::string attribute, std::string preference, std::string fallback);
    void activateGuides(void* desktop, bool active);
    char const *getName() const;
    std::vector<SPDesktop *> const getViewList() const;
    Inkscape::Util::Unit const * getDisplayUnit() const;
    void setDisplayUnit(std::string unit);
    void setDisplayUnit(Inkscape::Util::Unit const *unit);

    void translateGuides(Geom::Translate const &translation);
    void translateGrids(Geom::Translate const &translation);
    void scrollAllDesktops(double dx, double dy, bool is_scrolling);
    void writeNewGrid(SPDocument *document,int gridtype);
    void toggleGuides();
    void setGuides(bool v);
    bool getGuides();
    void lockGuides();
    void updateViewPort();
    // page background, border, desk colors
    void change_color(unsigned int rgba, SPAttr color_key, SPAttr opacity_key = SPAttr::INVALID);
    // show border, border on top, anti-aliasing, ...
    void change_bool_setting(SPAttr key, bool value);
    // sync desk colors
    void set_desk_color(SPDesktop* desktop);

private:
    double getMarginLength(gchar const * const key,Inkscape::Util::Unit const * const margin_units,Inkscape::Util::Unit const * const return_units,double const width,double const height,bool const use_width);
    friend class SPDocument;

    Inkscape::CanvasPage *_viewport = nullptr;

protected:
	void build(SPDocument *document, Inkscape::XML::Node *repr) override;
	void release() override;
    void modified(unsigned int flags) override;
    void update(SPCtx *ctx, unsigned int flags) override;
    void set(SPAttr key, char const* value) override;

	void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) override;
	void remove_child(Inkscape::XML::Node* child) override;
    void order_changed(Inkscape::XML::Node *child, Inkscape::XML::Node *old_repr,
                       Inkscape::XML::Node *new_repr) override;

    Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags) override;
};


void sp_namedview_window_from_document(SPDesktop *desktop);
void sp_namedview_zoom_and_view_from_document(SPDesktop *desktop);
void sp_namedview_document_from_window(SPDesktop *desktop);
void sp_namedview_update_layers_from_document (SPDesktop *desktop);

void sp_namedview_toggle_guides(SPDocument *doc, SPNamedView *namedview);
void sp_namedview_guides_toggle_lock(SPDocument *doc, SPNamedView *namedview);
void sp_namedview_show_grids(SPNamedView *namedview, bool show, bool dirty_document);
Inkscape::CanvasGrid * sp_namedview_get_first_enabled_grid(SPNamedView *namedview);

MAKE_SP_OBJECT_DOWNCAST_FUNCTIONS(SP_NAMEDVIEW, SPNamedView)

#endif /* !INKSCAPE_SP_NAMEDVIEW_H */


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
