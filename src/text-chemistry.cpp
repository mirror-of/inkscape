#define __SP_TEXT_CHEMISTRY_C__

/*
 * Text commands
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2004 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include "libnr/nr-macros.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "svg/svg.h"
#include "display/curve.h"
#include "helper/sp-intl.h"
#include "sp-object.h"
#include "sp-path.h"
#include "sp-text.h"
#include "style.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-handles.h"


SPItem *
text_in_selection (SPSelection *selection)
{
	for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
		if (SP_IS_TEXT (items->data))
			return ((SPItem *) items->data);
	}
	return NULL;
}

SPItem *
shape_in_selection (SPSelection *selection)
{
	for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
		if (SP_IS_SHAPE (items->data))
			return ((SPItem *) items->data);
	}
	return NULL;
}

void 
text_put_on_path (void)
{
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
	if (!desktop)
		return;

	SPSelection *selection = SP_DT_SELECTION(desktop);

	SPItem *text = text_in_selection (selection);
	SPItem *shape = shape_in_selection (selection);

	if (!text || !shape || g_slist_length ((GSList *) selection->itemList()) != 2) {
		desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text and a path</b> to put text on path."));
		return;
	}

	// fixme: check that the text is not textpath already

	// fixme: remove transform from text, but scale fontsize by expansion

	// fixme in transform_selectiion: treat textpath and its path the same as clone and its original

	// make a list of text children
	GSList *text_reprs = NULL;
	for (SPObject *o = SP_OBJECT(text)->children; o != NULL; o = o->next) {
		text_reprs = g_slist_prepend (text_reprs, SP_OBJECT_REPR (o));
	}

	// create textPath and put it into the text
	SPRepr *textpath = sp_repr_new ("textPath");
	// reference the shape
	sp_repr_set_attr (textpath, "xlink:href", g_strdup_printf ("#%s", sp_repr_attr (SP_OBJECT_REPR(shape), "id")));
	sp_repr_add_child (SP_OBJECT_REPR(text), textpath, NULL);

	for ( GSList *i = text_reprs ; i ; i = i->next ) {
		// make a copy of each text children
		SPRepr *copy = sp_repr_duplicate((SPRepr *) i->data);
		// We cannot have multiline in textpath, so remove line attrs from tspans
		if (!strcmp (sp_repr_name (copy), "tspan")) {
			sp_repr_set_attr (copy, "sodipodi:role", NULL);
			sp_repr_set_attr (copy, "x", NULL);
			sp_repr_set_attr (copy, "y", NULL);
		}
		// remove the old repr from under text
		sp_repr_remove_child(SP_OBJECT_REPR(text), (SPRepr *) i->data); 
		// put its copy into under textPath
		sp_repr_add_child (textpath, copy, NULL); // fixme: copy id
	}

	sp_document_done(SP_DT_DOCUMENT(desktop));
	g_slist_free(text_reprs);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
