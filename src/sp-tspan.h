#ifndef __SP_TSPAN_H__
#define __SP_TSPAN_H__

/*
 * tspan and textpath, based on the flowtext routines
 */

#include <glib.h>

#include <sigc++/sigc++.h>

#define SP_TYPE_TSPAN (sp_tspan_get_type ())
#define SP_TSPAN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TSPAN, SPTSpan))
#define SP_TSPAN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TSPAN, SPTSpanClass))
#define SP_IS_TSPAN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TSPAN))
#define SP_IS_TSPAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TSPAN))

#define SP_TYPE_TEXTPATH (sp_textpath_get_type ())
#define SP_TEXTPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TEXTPATH, SPTextPath))
#define SP_TEXTPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TEXTPATH, SPTextPathClass))
#define SP_IS_TEXTPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TEXTPATH))
#define SP_IS_TEXTPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TEXTPATH))

#include <libnr/nr-point.h>
#include "svg/svg-types.h"
#include "libnrtype/FlowSrc.h"

class SPUsePath;
class Path;
class flow_src;
/* SPTSpan */

enum {
	SP_TSPAN_ROLE_UNSPECIFIED,
	SP_TSPAN_ROLE_PARAGRAPH,
	SP_TSPAN_ROLE_LINE
};

struct SPTSpan {
	SPItem       item;
	guint        role : 2;
	
	div_flow_src     contents;

	SPSVGLength		 x,y;
};

struct SPTSpanClass {
	SPItemClass parent_class;
};

GType sp_tspan_get_type ();

/* SPTextPath */

struct SPTextPath {
	SPItem        item;

	div_flow_src	contents;
	SPSVGLength		 x,y;
	
  Path           *originalPath;
	bool           isUpdating;	
	SPUsePath      *sourcePath;
};

struct SPTextPathClass {
	SPItemClass  parent_class;
};

GType sp_textpath_get_type();


#endif
