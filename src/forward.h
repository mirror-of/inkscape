#ifndef __FORWARD_H__
#define __FORWARD_H__

/*
 * Forward declarations of most used objects
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib-object.h>

#include <glib.h>



/* Generic containers */

namespace Inkscape {
struct Application;
struct ApplicationClass;
};

/* Editing window */

class SPDesktop;
class SPDesktopClass;

#define SP_TYPE_DESKTOP (sp_desktop_get_type ())
#define SP_DESKTOP(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_DESKTOP, SPDesktop))
#define SP_IS_DESKTOP(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_DESKTOP))

GType sp_desktop_get_type ();

class SPEventContext;
class SPEventContextClass;

#define SP_TYPE_EVENT_CONTEXT (sp_event_context_get_type ())
#define SP_EVENT_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_EVENT_CONTEXT, SPEventContext))
#define SP_IS_EVENT_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_EVENT_CONTEXT))

GType sp_event_context_get_type ();

/* Document tree */

class SPDocument;
class SPDocumentClass;

#define SP_TYPE_DOCUMENT (sp_document_get_type ())
#define SP_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_DOCUMENT, SPDocument))
#define SP_IS_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_DOCUMENT))

GType sp_document_get_type ();

/* Objects */

class SPObject;
class SPObjectClass;

#define SP_TYPE_OBJECT (sp_object_get_type ())
#define SP_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_OBJECT, SPObject))
#define SP_IS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_OBJECT))

GType sp_object_get_type ();

class SPItem;
class SPItemClass;

#define SP_TYPE_ITEM (sp_item_get_type ())
#define SP_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_ITEM, SPItem))
#define SP_IS_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_ITEM))

GType sp_item_get_type ();

class SPGroup;
class SPGroupClass;

class SPDefs;
class SPDefsClass;

class SPRoot;
class SPRootClass;

class SPHeader;
class SPHeaderClass;

class SPNamedView;
class SPNamedViewClass;

class SPGuide;
class SPGuideClass;

class SPObjectGroup;
class SPObjectGroupClass;

class SPPath;
class SPPathClass;

class SPShape;
class SPShapeClass;

class SPPolygon;
class SPPolygonClass;

class SPEllipse;
class SPEllipseClass;

class SPCircle;
class SPCircleClass;

class SPArc;
class SPArcClass;

class SPChars;
class SPCharsClass;

class SPSelection;

class SPText;
class SPTextClass;

class SPTSpan;
class SPTSpanClass;

class SPString;
class SPStringClass;

class SPPaintServer;
class SPPaintServerClass;

class SPStop;
class SPStopClass;

class SPGradient;
class SPGradientClass;
class SPGradientReference;

class SPLinearGradient;
class SPLinearGradientClass;

class SPRadialGradient;
class SPRadialGradientClass;

class SPPattern;

class SPClipPath;
class SPClipPathClass;
class SPClipPathReference;

class SPMaskReference;

class SPAnchor;
class SPAnchorClass;

/* Misc */

class ColorRGBA;

class SPColorSpace;
class SPColor;

class SPStyle;

class SPEvent;

class SPPrintContext;

class StopOnTrue;

struct box_solution;

#endif /* !__FORWARD_H__ */

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
