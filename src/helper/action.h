#ifndef __SP_ACTION_H__
#define __SP_ACTION_H__

/*
 * Inkscape UI action implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Lauris Kaplinski
 *
 * This code is in public domain
 */

/** A macro to get the GType for actions */
#define SP_TYPE_ACTION (sp_action_get_type ())
/** A macro to cast and check the cast of changing an object to an action */
#define SP_ACTION(o) (NR_CHECK_INSTANCE_CAST ((o), SP_TYPE_ACTION, SPAction))
/** A macro to check whether or not something is an action */
#define SP_IS_ACTION(o) (NR_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ACTION))

#include <helper/helper-forward.h>
#include <libnr/nr-object.h>
#include <forward.h>
#include <view.h>


/** This is a structure that is used to hold all the possible
    actions that can be taken with an action.  These are the
	function pointers available */
struct SPActionEventVector {
	NRObjectEventVector object_vector;                                             /**< Parent class */
	void (* perform) (SPAction *action, void *ldata, void *pdata);                 /**< Actually do the action of the event.  Called by sp_perform_action */
	void (* set_active) (SPAction *action, unsigned int active, void *data);       /**< Callback for activation change */
	void (* set_sensitive) (SPAction *action, unsigned int sensitive, void *data); /**< Callback for a change in sensitivity */
	void (* set_shortcut) (SPAction *action, unsigned int shortcut, void *data);   /**< Callback for setting the shortcut for this function */
};

typedef int sp_verb_t;

/** All the data that is required to be an action.  This
    structure identifies the action and has the data to
	create menus and toolbars for the action */
struct SPAction : public NRActiveObject {
	unsigned int sensitive : 1;  /**< Value to track whether the action is sensitive */
	unsigned int active : 1;     /**< Value to track whether the action is active */
	SPView *view;                /**< The SPView to which this action is attached */
	gchar *id;                   /**< The identifier for the action */
	gchar *name;                 /**< Full text name of the action */
	gchar *tip;                  /**< A tooltip to describe the action */
	gchar *image;                /**< An image to visually identify the action */
	sp_verb_t verb;             /** The verb that produced this action */
};

/** The action class is the same as its parent */
struct SPActionClass {
	NRActiveObjectClass parent_class; /**< Parent Class */
};

NRType sp_action_get_type (void);

SPAction *sp_action_new(SPView *view,
			const gchar *id,
			const gchar *name,
			const gchar *tip,
			const gchar *image,
                   sp_verb_t verb);

void sp_action_perform (SPAction *action, void * data);
void sp_action_set_active (SPAction *action, unsigned int active);
void sp_action_set_sensitive (SPAction *action, unsigned int sensitive);
SPView *sp_action_get_view (SPAction *action);

#endif
