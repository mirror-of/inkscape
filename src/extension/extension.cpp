#define __SP_MODULE_C__
/**
	\file extension.cpp
 
	The ability to have features that are more modular so that they
	can be added and removed easily.  This is the basis for defining
	those actions.
*/

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>

#include "xml/repr.h"
#include "helper/sp-intl.h"
#include "dir-util.h"
#include "inkscape.h"
#include "sp-object.h"
#include "document.h"
#include "extension.h"

#include "db.h"

namespace Inkscape {
namespace Extension {

/* Inkscape::Extension::Extension */

/**
	\return  none
	\brief   Constructs an Extension from a SPRepr
	\param   in_repr    The repr that should be used to build it

	This function is the basis of building an extension for Inkscape.  It
	currently extracts the fields from the Repr that are used in the
	extension.  The Repr will likely include other children that are
	not related to the module directly.  If the Repr does not include
	a name and an ID the module will be left in an errored state.
*/
Extension::Extension (SPRepr * in_repr, Implementation::Implementation * in_imp)
{
	repr = in_repr;
	sp_repr_ref(in_repr);

	id = NULL;
	name = NULL;
	state = STATE_UNLOADED;

	if (in_imp == NULL) {
		imp = new Implementation::Implementation();
	} else {
		imp = in_imp;
	}

	printf("Extension Constructor: ");
	if (repr != NULL) {
		const gchar *val;
		gchar c[256];
		SPRepr * child_repr;

		child_repr = sp_repr_children(repr);
		/* TODO: Handle what happens if we don't have these two */
		while (child_repr != NULL) {
			if (!strcmp(sp_repr_name(child_repr), "id")) {
				val = sp_repr_content (sp_repr_children(child_repr));
				val = sp_module_db_get_unique_id (c, 256, val);
				id = g_strdup (val);
			} /* id */
			if (!strcmp(sp_repr_name(child_repr), "name")) {
				name = g_strdup (sp_repr_content(sp_repr_children(child_repr)));
			} /* name */
			child_repr = sp_repr_next(child_repr);
		}

		sp_module_db_register (this);
	}
	printf("%s\n", name);

	return;
}

/**
	\return   none
	\brief    Destroys the Extension

	This function frees all of the strings that could be attached
	to the extension and also unreferences the repr.  This is better
	than freeing it because it may (I wouldn't know why) be referenced
	in another place.
*/
Extension::~Extension (void)
{
	sp_repr_unref(repr);
	g_free(id);
	g_free(name);
	return;
}

/**
	\return   none
	\brief    A function to set whether the extension should be loaded
	          or unloaded
	\param    in_state  Which state should the extension be in?

	It checks to see if this is a state change or not.  If we're changing
	states it will call the appropriate function in the implementation,
	load or unload.  Currently, there is no error checking in this
	function.  There should be.
*/
void
Extension::set_state (state_t in_state)
{
	if (in_state != state) {
		/* TODO: Need some error checking here! */
		if (in_state == STATE_LOADED) {
			if (imp->load(this))
				state = STATE_LOADED;
		} else {
			imp->unload(this);
			state = STATE_UNLOADED;
		}
	}
	return;
}

/**
	\return   The state the extension is in
	\brief    A getter for the state variable.
*/
Extension::state_t
Extension::get_state (void)
{
	return state;
}

/**
    \return  Whether the extension is loaded or not
	\brief   A quick function to test the state of the extension
*/
bool
Extension::loaded (void)
{
	return state == STATE_LOADED;
}

/**
	\return  The XML tree that is used to define the extension
	\brief   A getter for the internal Repr, does not add a reference.
*/
SPRepr *
Extension::get_repr (void)
{
	return repr;
}

/*
	\return  The textual id of this extension
	\brief   Get the ID of this extension - not a copy don't delete!
*/
gchar *
Extension::get_id (void)
{
	return id;
}

/*
	\return  The textual name of this extension
	\brief   Get the name of this extension - not a copy don't delete!
*/
gchar *
Extension::get_name (void)
{
	return name;
}

/* Inkscape::Extension::Input */

/**
	\return   None
	\brief    Builds a SPModuleInput object from a XML description
	\param    module  The module to be initialized
	\param    repr    The XML description in a SPRepr tree

	Okay, so you want to build a SPModuleInput object.

	This function first takes and does the build of the parent class,
	which is SPModule.  Then, it looks for the <input> section of the
	XML description.  Under there should be several fields which
	describe the input module to excruciating detail.  Those are parsed,
	copied, and put into the structure that is passed in as module.
	Overall, there are many levels of indentation, just to handle the
	levels of indentation in the XML file.
*/
Input::Input (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
	mimetype = NULL;
	extension = NULL;
	filetypename = NULL;
	filetypetooltip = NULL;

	if (repr != NULL) {
		SPRepr * child_repr;

		child_repr = sp_repr_children(repr);

		while (child_repr != NULL) {
			if (!strcmp(sp_repr_name(child_repr), "input")) {
				child_repr = sp_repr_children(child_repr);
				while (child_repr != NULL) {
					if (!strcmp(sp_repr_name(child_repr), "extension")) {
						g_free (extension);
						extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "mimetype")) {
						g_free (mimetype);
						mimetype = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypename")) {
						g_free (filetypename);
						filetypename = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypetooltip")) {
						g_free (filetypetooltip);
						filetypetooltip = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}

					child_repr = sp_repr_next(child_repr);
				}

				break;
			}

			child_repr = sp_repr_next(child_repr);
		}

	}

	return;
}

Input::~Input (void)
{
	g_free(mimetype);
	g_free(extension);
	g_free(filetypename);
	g_free(filetypetooltip);
	return;
}

SPDocument *
Input::open (const gchar *uri)
{
	return imp->open(this, uri);
}

gchar *
Input::get_extension(void)
{
	return extension;
}

gchar *
Input::get_filetypename(void)
{
	return filetypename;
}

gchar *
Input::get_filetypetooltip(void)
{
	return filetypetooltip;
}

GtkDialog *
Input::prefs (const gchar *uri)
{
	return imp->prefs(this, uri);
}


/* Inkscape::Extension::Output */

/**
	\return   None
	\brief    Builds a SPModuleOutput object from a XML description
	\param    module  The module to be initialized
	\param    repr    The XML description in a SPRepr tree

	Okay, so you want to build a SPModuleOutput object.

	This function first takes and does the build of the parent class,
	which is SPModule.  Then, it looks for the <output> section of the
	XML description.  Under there should be several fields which
	describe the output module to excruciating detail.  Those are parsed,
	copied, and put into the structure that is passed in as module.
	Overall, there are many levels of indentation, just to handle the
	levels of indentation in the XML file.
*/
Output::Output (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
	mimetype = NULL;
	extension = NULL;
	filetypename = NULL;
	filetypetooltip = NULL;

	if (repr != NULL) {
		SPRepr * child_repr;

		child_repr = sp_repr_children(repr);

		while (child_repr != NULL) {
			if (!strcmp(sp_repr_name(child_repr), "output")) {
				child_repr = sp_repr_children(child_repr);
				while (child_repr != NULL) {
					if (!strcmp(sp_repr_name(child_repr), "extension")) {
						g_free (extension);
						extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "mimetype")) {
						g_free (mimetype);
						mimetype = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypename")) {
						g_free (filetypename);
						filetypename = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}
					if (!strcmp(sp_repr_name(child_repr), "filetypetooltip")) {
						g_free (filetypetooltip);
						filetypetooltip = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
					}

					child_repr = sp_repr_next(child_repr);
				}

				break;
			}

			child_repr = sp_repr_next(child_repr);
		}

	}
}

Output::~Output (void)
{
	g_free(mimetype);
	g_free(extension);
	g_free(filetypename);
	g_free(filetypetooltip);
	return;
}

gchar *
Output::get_extension(void)
{
	return extension;
}

gchar *
Output::get_filetypename(void)
{
	return filetypename;
}

gchar *
Output::get_filetypetooltip(void)
{
	return filetypetooltip;
}

GtkDialog *
Output::prefs (void)
{
	return imp->prefs(this);
}

void
Output::save (SPDocument * doc, const gchar * uri)
{
	return imp->save(this, doc, uri);
}

/* Inkscape::Extension::Filter */

Filter::Filter (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
	return;
}

Filter::~Filter (void)
{
	return;
}

GtkDialog *
Filter::prefs (void)
{
	return imp->prefs(this);
}

void
Filter::filter (SPDocument * doc)
{
	return imp->filter(this, doc);
}

/* Inkscape::Extension::Print */

Print::Print (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
	base = NULL;
	arena = NULL;
	root = NULL;
	dkey = 0;

	return;
}

Print::~Print (void)
{
	return;
}

unsigned int
Print::setup (void)
{
	return imp->setup(this);
}

unsigned int
Print::set_preview (void)
{
	return imp->set_preview(this);
}

unsigned int
Print::begin (SPDocument *doc)
{
	return imp->begin(this, doc);
}

unsigned int
Print::finish (void)
{
	return imp->finish(this);
}

unsigned int
Print::bind (const NRMatrix *transform, float opacity)
{
	return imp->bind (this, transform, opacity);
}

unsigned int
Print::release (void)
{
	return imp->release(this);
}

unsigned int
Print::fill (const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			       const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	return imp->fill (this, bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
Print::stroke (const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
				 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	return imp->stroke (this, bpath, transform, style, pbox, dbox, bbox);
}

unsigned int
Print::image (unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				const NRMatrix *transform, const SPStyle *style)
{
	return imp->image (this, px, w, h, rs, transform, style);
}


}; /* namespace Extension */
}; /* namespace Inkscape */

