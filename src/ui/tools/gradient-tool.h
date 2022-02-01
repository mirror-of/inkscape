// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef __SP_GRADIENT_CONTEXT_H__
#define __SP_GRADIENT_CONTEXT_H__

/*
 * Gradient drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org.
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005,2010 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstddef>
#include <sigc++/sigc++.h>
#include "ui/tools/tool-base.h"

#define SP_GRADIENT_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::GradientTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_GRADIENT_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::GradientTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

class GrDrag;

namespace Inkscape {

class Selection;

namespace UI {
namespace Tools {

class GradientTool : public ToolBase {
public:
    GradientTool(SPDesktop *desktop);
    ~GradientTool() override;

    bool root_handler(GdkEvent *event) override;
    void add_stops_between_selected_stops();

    void select_next();
    void select_prev();

private:
    Geom::Point mousepoint_doc; // stores mousepoint when over_line in doc coords
    Geom::Point origin;
    bool cursor_addnode;
    bool node_added;

    sigc::connection *selcon;
    sigc::connection *subselcon;

    void selection_changed(Inkscape::Selection *);
    void simplify(double tolerance);
    void add_stop_near_point(SPItem *item, Geom::Point mouse_p, guint32 etime);
    void drag(Geom::Point const pt, guint state, guint32 etime);
    SPItem *is_over_curve(Geom::Point event_p);
};

}
}
}

#endif


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
