#ifndef __SP_STRING_H__
#define __SP_STRING_H__

/*
 * string elements
 * extracted from sp-text
 */

#include <glib.h>

#include <sigc++/sigc++.h>

#include "sp-object.h"

#include "libnrtype/FlowSrc.h"
#include "libnrtype/FlowSrcText.h"


#define SP_TYPE_STRING (sp_string_get_type ())
#define SP_STRING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_STRING, SPString))
#define SP_STRING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_STRING, SPStringClass))
#define SP_IS_STRING(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_STRING))
#define SP_IS_STRING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_STRING))


struct SPString : public SPObject {
	partial_text     svg_contents;
	text_flow_src    contents;
};

struct SPStringClass {
	SPObjectClass parent_class;
};

#define SP_STRING_TEXT(s) (sp_string_get_text(SP_STRING(s)))

GType sp_string_get_type ();

char*          sp_string_get_text(SPString* string);

#endif
