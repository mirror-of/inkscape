// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SP_MESH_CONTEXT_H
#define SEEN_SP_MESH_CONTEXT_H

/*
 * Mesh drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org.
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2012 Tavmjong Bah
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005,2010 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstddef>
#include <sigc++/sigc++.h>
#include "ui/tools/tool-base.h"

#include "object/sp-mesh-array.h"

#define SP_MESH_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::MeshTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_MESH_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::MeshTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

class GrDrag;

namespace Inkscape {

class Selection;
class CanvasItemCurve;

namespace UI {
namespace Tools {

class MeshTool : public ToolBase {
public:
    MeshTool(SPDesktop *desktop);
    ~MeshTool() override;

    Geom::Point origin;

    Geom::Point mousepoint_doc; // stores mousepoint when over_line in doc coords

    sigc::connection *selcon;
    sigc::connection *subselcon;

    void set(const Inkscape::Preferences::Entry& val) override;
    bool root_handler(GdkEvent* event) override;
    void fit_mesh_in_bbox();
    void corner_operation(MeshCornerOperation operation);

private:
    bool cursor_addnode;
    bool show_handles;
    bool edit_fill;
    bool edit_stroke;

    void selection_changed(Inkscape::Selection *sel);
    void select_next();
    void select_prev();
    void new_default();
    void split_near_point(SPItem *item, Geom::Point mouse_p, guint32 /*etime*/);
    std::vector<CanvasItemCurve *> over_curve(Geom::Point event_p, bool first = true);
};

}
}
}

#endif // SEEN_SP_MESH_CONTEXT_H


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
