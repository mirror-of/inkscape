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
#include "prefs-utils.h"
#include "extension.h"
#include <extension/implementation/implementation.h>
#include <libnr/nr-point.h>

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
    parameters = NULL;
	_deactivated = FALSE;

    if (in_imp == NULL) {
        imp = new Implementation::Implementation();
    } else {
        imp = in_imp;
    }

//  printf("Extension Constructor: ");
    if (repr != NULL) {
        SPRepr *child_repr = sp_repr_children(repr);
        /* TODO: Handle what happens if we don't have these two */
        while (child_repr != NULL) {
            if (!strcmp(sp_repr_name(child_repr), "id")) {
                gchar const *val = sp_repr_content(sp_repr_children(child_repr));
                id = g_strdup (val);
            } /* id */
            if (!strcmp(sp_repr_name(child_repr), "name")) {
                name = g_strdup (sp_repr_content(sp_repr_children(child_repr)));
            } /* name */
            if (!strcmp(sp_repr_name(child_repr), "param")) {
                make_param(child_repr);
            } /* param */
            child_repr = sp_repr_next(child_repr);
        }

        db.register_ext (this);
    }
//  printf("%s\n", name);

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
//	printf("Extension Destructor: %s\n", name);
	set_state(STATE_UNLOADED);
	db.unregister_ext(this);
    sp_repr_unref(repr);
    g_free(id);
    g_free(name);
    /** \todo Need to do parameters here */
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
    \return  A boolean saying whether the extension passed the checks
	\brief   A function to check the validity of the extension

	This function chekcs to make sure that there is an id, a name, a
	repr and an implemenation for this extension.  Then it asks the
	implmentation to do a check of itself.
*/
bool
Extension::check (void)
{
	if (id == NULL)
		return FALSE;
	if (name == NULL)
		return FALSE;
	if (repr == NULL)
		return FALSE;
	if (imp == NULL)
		return FALSE;

	return imp->check(this);
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

/**
    \return  The textual id of this extension
    \brief   Get the ID of this extension - not a copy don't delete!
*/
gchar *
Extension::get_id (void)
{
    return id;
}

/**
    \return  The textual name of this extension
    \brief   Get the name of this extension - not a copy don't delete!
*/
gchar *
Extension::get_name (void)
{
    return name;
}

/**
    \return  None
	\brief   This function diactivates the extension (which makes it
	         unusable, but not deleted)
	
    This function is used to removed an extension from functioning, but
	not delete it completely.  It sets the \c _deactivated variable to
	mark to the world that it has been deactivated.  It also removes
	the current implementation and replaces it with a standard one.  This
	makes it so that we don't have to continually check if there is an
	implementation, but we are gauranteed to have a benign one.

	It is important to note that there is no 'activate' function.
	Running this function is irreversable.
*/
void
Extension::deactivate (void)
{
	_deactivated = TRUE;

	/* Removing the old implementation, and making this use the default. */
	/* This should save some memory */
	delete imp;
	imp = new Implementation::Implementation();

	return;
}

/**
    \return  Whether the extension has been deactivated
	\breif   Find out the status of the extension
*/
bool
Extension::deactivated (void)
{
	return _deactivated;
}

/**
    \return None
    \brief  This function creates a parameter that can be used later.  This
            is typically done in the creation of the extension and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  paramrepr  The XML describing the parameter

    This function first grabs all of the data out of the Repr and puts
    it into local variables.  Actually these are just pointers, and the
    data is not duplicated so we need to be careful with it.  If there
    isn't a name or a type in the XML, then no parameter is created as
    the function just returns.

    From this point on, we're pretty committed as we've allocated an
    object and we're starting to fill it.  The name is set first, and
    is created with a strdup to actually allocate memory for it.  Then
    there is a case statement (roughly because strcmp requires 'ifs')
    based on what type of parameter this is.  Depending which type it
    is, the value is interpreted differently, but they are relatively
    straight forward.  In all cases the value is set to the default
    value from the XML and the type is set to the interpreted type.

    Finally the allocated parameter is put into the GSList that is called
    parameteres.

    \todo This function should pull up parameters that are stored
    in the preferences somewhere.  This needs to be figured out.
*/
void
Extension::make_param (SPRepr * paramrepr)
{
    const char * name;
    const char * type;
    const char * defaultval;
    Extension::param_t * param;

    name = sp_repr_attr(paramrepr, "name");
    type = sp_repr_attr(paramrepr, "type");
    // defaultval = sp_repr_content(paramrepr);
    defaultval = sp_repr_content(sp_repr_children(paramrepr));

    /* In this case we just don't have enough information */
    if (name == NULL || type == NULL) {
        return;
    }

    param = new param_t();
    param->name = g_strdup(name);
    if (!strcmp(type, "boolean")) {
        param->type = Extension::PARAM_BOOL;
        if (defaultval != NULL && !strcmp(defaultval, "TRUE")) {
            param->val.t_bool = TRUE;
        } else {
            param->val.t_bool = FALSE;
        }
    } else if (!strcmp(type, "int")) { 
        param->type = Extension::PARAM_INT;
        if (defaultval != NULL) {
            param->val.t_int = atoi(defaultval);
        } else {
            param->val.t_int = 0;
        }
    } else if (!strcmp(type, "string")) { 
        param->type = Extension::PARAM_STRING;
        param->val.t_string = g_strdup(defaultval);
    } else {
        return;
    }

    parameters = g_slist_append(parameters, param);
    return;
}

/**
    \return    Parameter structure with a name of 'name'
    \brief     This function looks through the linked list for a parameter
               structure with the name of the passed in name
    \param     name   The name to search for
    \param     list   The list to look for

    This is an inline function that is used by all the get_param and
    set_param functions to find a param_t in the linked list with
    the passed in name.  It is done as an inline so that it will be
    optimized into a 'jump' by the compiler.

    This function can throw a 'param_not_exist' exception if the
    name is not found.

    The first thing that this function checks is if the list is NULL.
    It could be NULL because there are no parameters for this extension
    or because all of them have been checked (I'll spoil the ending and
    tell you that this function is called recursively).  If the list
    is NULL then the 'param_not_exist' exception is thrown.

    Otherwise, the function looks at the current param_t that the element
    list points to.  If the name of that param_t matches the passed in
    name then that param_t is returned.  Otherwise, this function is
    called again with g_slist_next as a parameter.
*/
inline Extension::param_t *
Extension::param_shared (gchar * name, GSList * list)
{
    Extension::param_t * output;
    
    if (name == NULL) {
        throw Extension::param_not_exist();
    }
    if (list == NULL) {
        throw Extension::param_not_exist();
    }

    output = static_cast<Extension::param_t *>(list->data);
    if (!strcmp(output->name, name)) {
        return output;
    }

    return Extension::param_shared(name, g_slist_next(list));
}

/**
    \return   None
    \brief    Gets a parameter identified by name with the string placed
              in value.  It isn't duplicated into the value string.
    \param    name    The name of the parameter to get
    \param    value   Place to put a pointer to the string

    This function first checks to make sure that the value parameter
    is not passed in as NULL.  If so, the function returns.  This
    doesn't emit a warning - this will probably cause some sort of
    error in the calling function (as it wouldn't be calling this function
    if it didn't want the value) so it can be debugged there.

    To get the parameter to be used the function param_shared is called.
    This function is inline so it shouldn't cause the stack to build
    or anything like that.  If it can't find the parameter, it will
    throw and exception - we aren't catching that because we want
    the calling function to catch it.

    Next up, the parameter that we got, we're making sure that it is
    a string parameter.  If it isn't, then we throw a param_wrong_type
    exception.
    
    Finally, if everything is okay, the string value that is stored in
    the parameter is placed in value.
*/
void
Extension::get_param (gchar * name, gchar ** value)
{
    Extension::param_t * param;
    
    if (value == NULL) {
        /* This probably isn't a good error, but the calling function
           will find out soon enough it doesn't have data there ;) */
        return;
    }

    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_STRING) {
        throw Extension::param_wrong_type();
    }

    *value = param->val.t_string;
    return;
}

/**
    \return   None
    \brief    Gets a parameter identified by name with the bool placed
              in value.
    \param    name    The name of the parameter to get
    \param    value   Place to put the bool value

    This function first checks to make sure that the value parameter
    is not passed in as NULL.  If so, the function returns.  This
    doesn't emit a warning - this will probably cause some sort of
    error in the calling function (as it wouldn't be calling this function
    if it didn't want the value) so it can be debugged there.

    To get the parameter to be used the function param_shared is called.
    This function is inline so it shouldn't cause the stack to build
    or anything like that.  If it can't find the parameter, it will
    throw and exception - we aren't catching that because we want
    the calling function to catch it.

    Next up, the parameter that we got, we're making sure that it is
    a bool parameter.  If it isn't, then we throw a param_wrong_type
    exception.
    
    Finally, if everything is okay, the boolean value that is stored in
    the parameter is placed in value.
*/
void
Extension::get_param (gchar * name, bool * value)
{
    Extension::param_t * param;
    
    if (value == NULL) {
        /* This probably isn't a good error, but the calling function
           will find out soon enough it doesn't have data there ;) */
        return;
    }

    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_BOOL) {
        throw Extension::param_wrong_type();
    }

    *value = param->val.t_bool;
    return;
}

/**
    \return   None
    \brief    Gets a parameter identified by name with the integer placed
              in value.
    \param    name    The name of the parameter to get
    \param    value   Place to put the integer value

    This function first checks to make sure that the value parameter
    is not passed in as NULL.  If so, the function returns.  This
    doesn't emit a warning - this will probably cause some sort of
    error in the calling function (as it wouldn't be calling this function
    if it didn't want the value) so it can be debugged there.

    To get the parameter to be used the function param_shared is called.
    This function is inline so it shouldn't cause the stack to build
    or anything like that.  If it can't find the parameter, it will
    throw and exception - we aren't catching that because we want
    the calling function to catch it.

    Next up, the parameter that we got, we're making sure that it is
    a integer parameter.  If it isn't, then we throw a param_wrong_type
    exception.
    
    Finally, if everything is okay, the integer value that is stored in
    the parameter is placed in value.
*/
void
Extension::get_param (gchar * name, int * value)
{
    Extension::param_t * param;
    
    if (value == NULL) {
        /* This probably isn't a good error, but the calling function
           will find out soon enough it doesn't have data there ;) */
        return;
    }

    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_INT) {
        throw Extension::param_wrong_type();
    }

    *value = param->val.t_int;
    return;
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the boolean
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to

    To get the parameter to be used the function param_shared is called.
    This function is inline so it shouldn't cause the stack to build
    or anything like that.  If it can't find the parameter, it will
    throw and exception - we aren't catching that because we want
    the calling function to catch it.

    Next up, the parameter that we got, we're making sure that it is
    a boolean parameter.  If it isn't, then we throw a param_wrong_type
    exception.
    
    Finally, if everything is okay, the boolean value that was passed
	in is placed in the param.
*/
bool
Extension::set_param (gchar * name, bool value)
{
    Extension::param_t * param;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_BOOL) {
        throw Extension::param_wrong_type();
    }

    param->val.t_bool = value;
    return value;
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the integer
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to

    To get the parameter to be used the function param_shared is called.
    This function is inline so it shouldn't cause the stack to build
    or anything like that.  If it can't find the parameter, it will
    throw and exception - we aren't catching that because we want
    the calling function to catch it.

    Next up, the parameter that we got, we're making sure that it is
    a integer parameter.  If it isn't, then we throw a param_wrong_type
    exception.
    
    Finally, if everything is okay, the integer value that was passed
	in is placed in the param.
*/
int
Extension::set_param (gchar * name, int value)
{
    Extension::param_t * param;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_INT) {
        throw Extension::param_wrong_type();
    }

    param->val.t_int = value;
    return value;
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the string
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to

	First this function makes sure that the incoming string is not
	NULL.  The value can't be set to NULL.

    To get the parameter to be used the function param_shared is called.
    This function is inline so it shouldn't cause the stack to build
    or anything like that.  If it can't find the parameter, it will
    throw and exception - we aren't catching that because we want
    the calling function to catch it.

    Next up, the parameter that we got, we're making sure that it is
    a string parameter.  If it isn't, then we throw a param_wrong_type
    exception.
    
    Finally, if everything is okay, the previous value is free'd and
	the incoming value is duplicated and placed in the parameter.
*/
gchar *
Extension::set_param (gchar * name, gchar * value)
{
    Extension::param_t * param;
    
    if (value == NULL) {
        /* This probably isn't a good error, but the calling function
           will find out soon enough it doesn't have data there ;) */
        return value;
    }

    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_STRING) {
        throw Extension::param_wrong_type();
    }

    g_free(param->val.t_string);
    param->val.t_string = g_strdup(value);
    return value;
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
	dataloss = TRUE;

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
                    if (!strcmp(sp_repr_name(child_repr), "dataloss")) {
                        if (!strcmp(sp_repr_content(sp_repr_children(child_repr)), "FALSE")) {
							dataloss = FALSE;
						}
					}

                    child_repr = sp_repr_next(child_repr);
                }

                break;
            }

            child_repr = sp_repr_next(child_repr);
        }

    }
}

/**
    \brief  Destroy an output extension
*/
Output::~Output (void)
{
    g_free(mimetype);
    g_free(extension);
    g_free(filetypename);
    g_free(filetypetooltip);
    return;
}

/**
    \return  Whether this extension checks out
	\brief   Validate this extension

	This function checks to make sure that the output extension has
	a filename extension and a MIME type.  Then it calls the parent
	class' check function which also checks out the implmentation.
*/
bool
Output::check (void)
{
	if (extension == NULL)
		return FALSE;
	if (mimetype == NULL)
		return FALSE;

	return Extension::check();
}

/**
    \return  Filename extension for the extension
	\brief   Get the filename extension for this extension
*/
gchar *
Output::get_extension(void)
{
    return extension;
}

/**
    \return  The name of the filetype supported
	\brief   Get the name of the filetype supported
*/
gchar *
Output::get_filetypename(void)
{
    return filetypename;
}

/**
    \return  Tooltip giving more information on the filetype
	\brief   Get the tooltip for more information on the filetype
*/
gchar *
Output::get_filetypetooltip(void)
{
    return filetypetooltip;
}

/**
    \return  A dialog to get settings for this extension
	\brief   Create a dialog for preference for this extension

	Calls the implementation to get the preferences.
*/
GtkDialog *
Output::prefs (void)
{
    return imp->prefs(this);
}

/**
    \return  None
	\brief   Save a document as a file
	\param   doc  Document to save
	\param   uri  File to save the document as

	This function does a little of the dirty work involved in saving
	a document so that the implementation only has to worry about geting
	bits on the disk.

	The big thing that it does is remove and readd the fields that are
	only used at runtime and shouldn't be saved.  One that may surprise
	people is the output extension.  This is not saved so that the IDs
	could be changed, and old files will still work properly.

	After the file is saved by the implmentation the output_extension
	and dataloss variables are recreated.  The output_extension is set
	to this extension so that future saves use this extension.  Dataloss
	is set so that a warning will occur on closing the document that
	there may be some dataloss from this extension.
*/
void
Output::save (SPDocument * doc, const gchar * uri)
{
	SPRepr * repr;

	repr = sp_document_repr_root(doc);

	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_attr(repr, "inkscape:output_extension", NULL);
	sp_repr_set_attr(repr, "inkscape:dataloss", NULL);
	sp_repr_set_attr(repr, "sodipodi:modified", NULL);
	sp_document_set_undo_sensitive (doc, TRUE);

    imp->save(this, doc, uri);

	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_attr(repr, "inkscape:output_extension", get_id());
	if (dataloss) {
		sp_repr_set_attr(repr, "inkscape:dataloss", "TRUE");
	}
	sp_document_set_undo_sensitive (doc, TRUE);

	return;
}

/* Inkscape::Extension::Effect */

Effect::Effect (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
    return;
}

Effect::~Effect (void)
{
    return;
}

bool
Effect::check (void)
{
	return Extension::check();
}

GtkDialog *
Effect::prefs (void)
{
    return imp->prefs(this);
}

void
Effect::effect (SPDocument * doc)
{
    return imp->effect(this, doc);
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

bool
Print::check (void)
{
	return Extension::check();
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

unsigned int
Print::text (const char* text, NR::Point p, const SPStyle* style)
{
    return imp->text (this, text, p, style);
}


}; /* namespace Extension */
}; /* namespace Inkscape */

