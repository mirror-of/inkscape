#define __SP_ACTION_C__

/*
 * Sodipodi UI action implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <string.h>
#include <malloc.h>

#include <libnr/nr-macros.h>

#include "action.h"

static void sp_action_class_init (SPActionClass *klass);
static void sp_action_init (SPAction *action);
static void sp_action_finalize (NRObject *object);

static NRActiveObjectClass *parent_class;

NRType
sp_action_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ACTIVE_OBJECT,
						"SPAction",
						sizeof (SPActionClass),
						sizeof (SPAction),
						(void (*) (NRObjectClass *)) sp_action_class_init,
						(void (*) (NRObject *)) sp_action_init);
	}
	return type;
}

static void
sp_action_class_init (SPActionClass *klass)
{
	NRObjectClass * object_class;

	object_class = (NRObjectClass *) klass;

	parent_class = (NRActiveObjectClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = sp_action_finalize;
}

static void
sp_action_init (SPAction *action)
{
}

static void
sp_action_finalize (NRObject *object)
{
	SPAction *action;

	action = (SPAction *) object;

	if (action->image) free (action->image);
	if (action->tip) free (action->tip);
	if (action->name) free (action->name);
	if (action->id) free (action->id);

	((NRObjectClass *) (parent_class))->finalize (object);
}

SPAction *
sp_action_setup (SPAction *action,
		 const unsigned char *id,
		 const unsigned char *name,
		 const unsigned char *tip,
		 const unsigned char *image)
{
	nr_object_setup ((NRObject *) action, SP_TYPE_ACTION);

	action->sensitive = TRUE;
	if (id) action->id = strdup (id);
	if (name) action->name = strdup (name);
	if (tip) action->tip = strdup (tip);
	if (image) action->image = strdup (image);

	return action;
}

void
sp_action_perform (SPAction *action)
{
	NRActiveObject *aobject;

	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

	aobject = (NRActiveObject *) action;
	if (aobject->callbacks) {
		int i;
		for (i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener;
			SPActionEventVector *avector;
			listener = aobject->callbacks->listeners + i;
			avector = (SPActionEventVector *) listener->vector;
			if ((listener->size >= sizeof (SPActionEventVector)) && avector->perform) {
				avector->perform (action, listener->data);
			}
		}
	}
}

void
sp_action_set_active (SPAction *action, unsigned int active)
{
	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

	if (active != action->active) {
		NRActiveObject *aobject;
		action->active = active;
		aobject = (NRActiveObject *) action;
		if (aobject->callbacks) {
			int i;
			for (i = 0; i < aobject->callbacks->length; i++) {
				NRObjectListener *listener;
				SPActionEventVector *avector;
				listener = aobject->callbacks->listeners + i;
				avector = (SPActionEventVector *) listener->vector;
				if ((listener->size >= sizeof (SPActionEventVector)) && avector->set_active) {
					avector->set_active (action, active, listener->data);
				}
			}
		}
	}
}

void
sp_action_set_sensitive (SPAction *action, unsigned int sensitive)
{
	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

	if (sensitive != action->sensitive) {
		NRActiveObject *aobject;
		action->sensitive = sensitive;
		aobject = (NRActiveObject *) action;
		if (aobject->callbacks) {
			int i;
			for (i = 0; i < aobject->callbacks->length; i++) {
				NRObjectListener *listener;
				SPActionEventVector *avector;
				listener = aobject->callbacks->listeners + i;
				avector = (SPActionEventVector *) listener->vector;
				if ((listener->size >= sizeof (SPActionEventVector)) && avector->set_sensitive) {
					avector->set_sensitive (action, sensitive, listener->data);
				}
			}
		}
	}
}

void
sp_action_set_shortcut (SPAction *action, unsigned int shortcut)
{
	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

	if (shortcut != action->shortcut) {
		NRActiveObject *aobject;
		action->shortcut = shortcut;
		aobject = (NRActiveObject *) action;
		if (aobject->callbacks) {
			int i;
			for (i = 0; i < aobject->callbacks->length; i++) {
				NRObjectListener *listener;
				SPActionEventVector *avector;
				listener = aobject->callbacks->listeners + i;
				avector = (SPActionEventVector *) listener->vector;
				if ((listener->size >= sizeof (SPActionEventVector)) && avector->set_shortcut) {
					avector->set_shortcut (action, shortcut, listener->data);
				}
			}
		}
	}
}

