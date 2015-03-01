/*
 * Authors:
 *   Liam P. White
 *
 * Copyright (C) 2015 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "canvas-guidable.h"
#include "display/guideline.h"

CanvasGuidable::CanvasGuidable(SPCanvasGroup *parent)
    : Guidable()
{
    _key = NULL;
    guide = (SPGuideLine *) sp_guideline_new(parent, NULL, Geom::Point(), Geom::Point());
}

CanvasGuidable::~CanvasGuidable()
{
    sp_guideline_delete(guide);
}

void CanvasGuidable::hide()
{
    sp_canvas_item_hide(SP_CANVAS_ITEM(guide));
}

void CanvasGuidable::hide_origin()
{
    sp_canvas_item_hide(SP_CANVAS_ITEM(guide->origin));
}

void CanvasGuidable::show()
{
    sp_canvas_item_show(SP_CANVAS_ITEM(guide));
}

void CanvasGuidable::show_origin()
{
    sp_canvas_item_show(SP_CANVAS_ITEM(guide->origin));
}

void CanvasGuidable::set_color(guint32 color)
{
    sp_guideline_set_color(guide, color);
}

void CanvasGuidable::set_label(char const *label)
{
    sp_guideline_set_label(guide, label);
}

void CanvasGuidable::set_position(Geom::Point const& pos)
{
    sp_guideline_set_position(guide, pos);
}

void CanvasGuidable::set_normal(Geom::Point const& normal)
{
    sp_guideline_set_normal(guide, normal);
}

void CanvasGuidable::set_sensitive(bool sensitive)
{
    sp_guideline_set_sensitive(guide, sensitive);
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
