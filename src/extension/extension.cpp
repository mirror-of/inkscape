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
#include "dependency.h"

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
    _state = STATE_UNLOADED;
    parameters = NULL;

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
            if (!strcmp(sp_repr_name(child_repr), "dependency")) {
                _deps.push_back(new Dependency(child_repr));
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

	for (unsigned int i = 0 ; i < _deps.size(); i++) {
		delete _deps[i];
	}
	_deps.clear();

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
	if (_state == STATE_DEACTIVATED) return;
    if (in_state != _state) {
        /** \todo Need some more error checking here! */
		switch (in_state) {
			case STATE_LOADED:
				if (imp->load(this))
					_state = STATE_LOADED;
				break;
			case STATE_UNLOADED:
				imp->unload(this);
				_state = STATE_UNLOADED;
				break;
			case STATE_DEACTIVATED:
				_state = STATE_DEACTIVATED;
				break;
			default:
				break;
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
    return _state;
}

/**
    \return  Whether the extension is loaded or not
    \brief   A quick function to test the state of the extension
*/
bool
Extension::loaded (void)
{
    return get_state() == STATE_LOADED;
}

/**
    \return  A boolean saying whether the extension passed the checks
	\brief   A function to check the validity of the extension

	This function chekcs to make sure that there is an id, a name, a
	repr and an implemenation for this extension.  Then it checks all
	of the dependencies to see if they pass.  Finally, it asks the
	implmentation to do a check of itself.
*/
bool
Extension::check (void)
{
	// static int i = 0;
	// std::cout << "Checking module[" << i++ << "]: " << name << std::endl;
	if (id == NULL)
		return FALSE;
	if (name == NULL)
		return FALSE;
	if (repr == NULL)
		return FALSE;
	if (imp == NULL)
		return FALSE;

	for (unsigned int i = 0 ; i < _deps.size(); i++) {
		if (_deps[i]->check() == FALSE) {
			// std::cout << "Failed: " << *(_deps[i]) << std::endl;
			return FALSE;
		}
	}

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
	not delete it completely.  It sets the state to \c STATE_DEACTIVATED to
	mark to the world that it has been deactivated.  It also removes
	the current implementation and replaces it with a standard one.  This
	makes it so that we don't have to continually check if there is an
	implementation, but we are gauranteed to have a benign one.

	\warning It is important to note that there is no 'activate' function.
	Running this function is irreversable.
*/
void
Extension::deactivate (void)
{
	set_state(STATE_DEACTIVATED);

	/* Removing the old implementation, and making this use the default. */
	/* This should save some memory */
	delete imp;
	imp = new Implementation::Implementation();

	return;
}

/**
    \return  Whether the extension has been deactivated
	\brief   Find out the status of the extension
*/
bool
Extension::deactivated (void)
{
	return get_state() == STATE_DEACTIVATED;
}

/**
    \return None
    \brief  This function creates a parameter that can be used later.  This
            is typically done in the creation of the extension and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  paramrepr  The XML describing the parameter

    This function first grabs all of the data out of the Repr and puts
    it into local variables.  Actually, these are just pointers, and the
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
	gchar * param_name;
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
	param_name = g_strdup_printf("%s.%s", id, name);
    if (!strcmp(type, "boolean")) {
        param->type = Extension::PARAM_BOOL;
        if (defaultval != NULL && !strcmp(defaultval, "TRUE")) {
            param->val.t_bool = TRUE;
        } else {
            param->val.t_bool = FALSE;
        }
		param->val.t_bool = prefs_get_int_attribute("extensions", param_name, (gint)param->val.t_bool);
    } else if (!strcmp(type, "int")) { 
        param->type = Extension::PARAM_INT;
        if (defaultval != NULL) {
            param->val.t_int = atoi(defaultval);
        } else {
            param->val.t_int = 0;
        }
		param->val.t_int = prefs_get_int_attribute("extensions", param_name, (gint)param->val.t_int);
    } else if (!strcmp(type, "string")) { 
		const gchar * temp_str;
        param->type = Extension::PARAM_STRING;

		temp_str = prefs_get_string_attribute("extensions", param_name);
		if (temp_str == NULL)
			temp_str = defaultval;

        param->val.t_string = g_strdup(temp_str);
    } else {
		g_free(param_name);
        return;
    }
	g_free(param_name);

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
Extension::param_shared (const gchar * name, GSList * list)
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
    \return   A constant pointer to the string held by the parameters.
    \brief    Gets a parameter identified by name with the string placed
              in value.  It isn't duplicated into the value string.
    \param    name    The name of the parameter to get
	\param    doc    The document to look in for document specific parameters

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
const gchar *
Extension::get_param_string (const gchar * name, const SPReprDoc * doc)
{
    Extension::param_t * param;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_STRING) {
        throw Extension::param_wrong_type();
    }

    return param->val.t_string;
}

/**
    \return   The value of the parameter identified by the name
    \brief    Gets a parameter identified by name with the bool placed
              in value.
    \param    name    The name of the parameter to get
	\param    doc    The document to look in for document specific parameters

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
bool
Extension::get_param_bool (const gchar * name, const SPReprDoc * doc)
{
    Extension::param_t * param;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_BOOL) {
        throw Extension::param_wrong_type();
    }

    return param->val.t_bool;
}

/**
    \return   The integer value for the parameter specified
    \brief    Gets a parameter identified by name with the integer placed
              in value.
    \param    name    The name of the parameter to get
	\param    doc    The document to look in for document specific parameters

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
int
Extension::get_param_int (const gchar * name, const SPReprDoc * doc)
{
    Extension::param_t * param;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_INT) {
        throw Extension::param_wrong_type();
    }

    return param->val.t_int;
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the boolean
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
	\param    doc    The document to look in for document specific parameters

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
Extension::set_param_bool (const gchar * name, bool value, SPReprDoc * doc)
{
    Extension::param_t * param;
	gchar * param_name;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_BOOL) {
        throw Extension::param_wrong_type();
    }

    param->val.t_bool = value;

	param_name = g_strdup_printf("%s.%s", id, name);
	prefs_set_int_attribute("extensions", param_name, value == TRUE ? 1 : 0);
	g_free(param_name);

    return value;
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the integer
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
	\param    doc    The document to look in for document specific parameters

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
Extension::set_param_int (const gchar * name, int value, SPReprDoc * doc)
{
    Extension::param_t * param;
	gchar * param_name;
    
    param = Extension::param_shared(name, parameters);

    if (param->type != Extension::PARAM_INT) {
        throw Extension::param_wrong_type();
    }

    param->val.t_int = value;

	param_name = g_strdup_printf("%s.%s", id, name);
	prefs_set_int_attribute("extensions", param_name, value);
	g_free(param_name);

    return value;
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the string
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
	\param    doc    The document to look in for document specific parameters

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
const gchar *
Extension::set_param_string (const gchar * name, const gchar * value, SPReprDoc * doc)
{
    Extension::param_t * param;
	gchar * param_name;
    
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

	param_name = g_strdup_printf("%s.%s", id, name);
	prefs_set_string_attribute("extensions", param_name, value);
	g_free(param_name);

    return value;
}

}; /* namespace Extension */
}; /* namespace Inkscape */

