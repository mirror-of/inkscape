#ifndef SEEN_PENCIL_CONTEXT_H
#define SEEN_PENCIL_CONTEXT_H

/** \file
 * Pencil context
 */

#include "draw-context.h"


#define SP_TYPE_PENCIL_CONTEXT (sp_pencil_context_get_type())
#define SP_PENCIL_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_PENCIL_CONTEXT, SPPencilContext))
#define SP_PENCIL_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_PENCIL_CONTEXT, SPPencilContextClass))
#define SP_IS_PENCIL_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_PENCIL_CONTEXT))
#define SP_IS_PENCIL_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_PENCIL_CONTEXT))

enum PencilState {
    SP_PENCIL_CONTEXT_IDLE,
    SP_PENCIL_CONTEXT_ADDLINE,
    SP_PENCIL_CONTEXT_FREEHAND
};

/**
 * Pencil context.
 */
struct SPPencilContext : public SPDrawContext {
    NR::Point p[16];
    gint npoints;
    PencilState state;
    NR::Point req_tangent;

    bool is_drawing;
};

/// The SPPencilContext vtable (empty).
struct SPPencilContextClass : public SPEventContextClass { };

GType sp_pencil_context_get_type();


#endif /* !SEEN_PENCIL_CONTEXT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
