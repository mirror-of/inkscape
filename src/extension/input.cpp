/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "implementation/implementation.h"
#include "input.h"

/* Inkscape::Extension::Input */

namespace Inkscape {
namespace Extension {

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
	output_extension = NULL;

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
                    if (!strcmp(sp_repr_name(child_repr), "output_extension")) {
                        g_free (output_extension);
                        output_extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
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

/**
	\return  None
	\brief   Destroys an Input extension
*/
Input::~Input (void)
{
    g_free(mimetype);
    g_free(extension);
    g_free(filetypename);
    g_free(filetypetooltip);
	g_free(output_extension);
    return;
}

/**
    \return  Whether this extension checks out
	\brief   Validate this extension

	This function checks to make sure that the input extension has
	a filename extension and a MIME type.  Then it calls the parent
	class' check function which also checks out the implmentation.
*/
bool
Input::check (void)
{
	if (extension == NULL)
		return FALSE;
	if (mimetype == NULL)
		return FALSE;

	return Extension::check();
}

/**
    \return  A new document
	\brief   This function creates a document from a file
	\param   uri  The filename to create the document from

	This function acts as the first step in creating a new document
	from a file.  The first thing that this does is make sure that the
	file actually exists.  If it doesn't, a NULL is returned.  If the
	file exits, then it is opened using the implmentation of this extension.

	After opening the document the output_extension is set.  What this
	accomplishes is that save can try to use an extension that supports
	the same fileformat.  So something like opening and saveing an 
	Adobe Illustrator file can be transparent (not recommended, but
	transparent).  This is all done with undo being turned off.
*/
SPDocument *
Input::open (const gchar *uri)
{
	SPDocument * doc;
	SPRepr * repr;

	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* local_uri = g_filename_from_utf8 ( uri,
                                 -1,  &bytesRead,  &bytesWritten, &error);

        // TODO fix this to work on WindowsXP
	if (!g_file_test(local_uri, G_FILE_TEST_EXISTS)) {
		g_free(local_uri);
		return NULL;
	}
	g_free(local_uri);

	doc = imp->open(this, uri);

	if (doc != NULL) {
		repr = sp_document_repr_root(doc);
		sp_document_set_undo_sensitive (doc, FALSE);
		sp_repr_set_attr(repr, "inkscape:output_extension", output_extension);
		sp_document_set_undo_sensitive (doc, TRUE);
	}

	return doc;
}

/**
    \return  IETF mime-type for the extension
	\brief   Get the mime-type that describes this extension
*/
gchar *
Input::get_mimetype(void)
{
    return mimetype;
}

/**
    \return  Filename extension for the extension
	\brief   Get the filename extension for this extension
*/
gchar *
Input::get_extension(void)
{
    return extension;
}

/**
    \return  The name of the filetype supported
	\brief   Get the name of the filetype supported
*/
gchar *
Input::get_filetypename(void)
{
    return filetypename;
}

/**
    \return  Tooltip giving more information on the filetype
	\brief   Get the tooltip for more information on the filetype
*/
gchar *
Input::get_filetypetooltip(void)
{
    return filetypetooltip;
}

/**
    \return  A dialog to get settings for this extension
	\brief   Create a dialog for preference for this extension

	Calls the implementation to get the preferences.
*/
GtkDialog *
Input::prefs (const gchar *uri)
{
    return imp->prefs_input(this, uri);
}

}; }; /* namespace Inkscape, Extension */

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
