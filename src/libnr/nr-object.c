#define __NR_OBJECT_C__

/*
 * RGBA display list system for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <string.h>
#include <stdio.h>

#include <libnr/nr-macros.h>

#include "nr-object.h"

unsigned int
nr_emit_fail_warning (const unsigned char *file, unsigned int line, const unsigned char *method, const unsigned char *expr)
{
	fprintf (stderr, "File %s line %d (%s): Assertion %s failed\n", file, line, method, expr);
	return 1;
}

/* NRObject */

static NRObjectClass **classes = NULL;
static unsigned int classes_len = 0;
static unsigned int classes_size = 0;

unsigned int
nr_type_is_a (NRType type, NRType test)
{
	NRObjectClass *klass;

	nr_return_val_if_fail (type < classes_len, FALSE);
	nr_return_val_if_fail (test < classes_len, FALSE);

	klass = classes[type];

	while (klass) {
		if (klass->type == test) return TRUE;
		klass = klass->parent;
	}

	return FALSE;
}

void *
nr_object_check_instance_cast (void *ip, unsigned int tc)
{
	nr_return_val_if_fail (ip != NULL, NULL);
	nr_return_val_if_fail (nr_type_is_a (((NRObject *) ip)->klass->type, tc), ip);
	return ip;
}

unsigned int
nr_object_check_instance_type (void *ip, NRType tc)
{
	if (ip == NULL) return FALSE;
	return nr_type_is_a (((NRObject *) ip)->klass->type, tc);
}

NRType
nr_object_register_type (NRType parent,
			 unsigned char *name,
			 unsigned int csize,
			 unsigned int isize,
			 void (* cinit) (NRObjectClass *),
			 void (* iinit) (NRObject *))
{
	NRType type;
	NRObjectClass *klass;

	if (classes_len >= classes_size) {
		classes_size += 32;
		classes = nr_renew (classes, NRObjectClass *, classes_size);
		if (classes_len == 0) {
			classes[0] = NULL;
			classes_len = 1;
		}
	}

	type = classes_len;
	classes_len += 1;

	classes[type] = malloc (csize);
	klass = classes[type];
	memset (klass, 0, csize);

	if (classes[parent]) {
		memcpy (klass, classes[parent], classes[parent]->csize);
	}

	klass->type = type;
	klass->parent = classes[parent];
	klass->name = strdup (name);
	klass->csize = csize;
	klass->isize = isize;
	klass->cinit = cinit;
	klass->iinit = iinit;

	klass->cinit (klass);

	return type;
}

static void nr_object_class_init (NRObjectClass *klass);
static void nr_object_init (NRObject *object);
static void nr_object_finalize (NRObject *object);

NRType
nr_object_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (0,
						"NRObject",
						sizeof (NRObjectClass),
						sizeof (NRObject),
						(void (*) (NRObjectClass *)) nr_object_class_init,
						(void (*) (NRObject *)) nr_object_init);
	}
	return type;
}

static void
nr_object_class_init (NRObjectClass *klass)
{
	klass->finalize = nr_object_finalize;
}

static void nr_object_init (NRObject *object)
{
}

static void nr_object_finalize (NRObject *object)
{
}

/* Dynamic lifecycle */

NRObject *
nr_object_new (NRType type)
{
	NRObjectClass *klass;
	NRObject *object;

	nr_return_val_if_fail (type < classes_len, NULL);

	klass = classes[type];
	object = malloc (klass->isize);
	nr_object_setup (object, type);

	return object;
}

NRObject *
nr_object_delete (NRObject *object)
{
	nr_object_release (object);
	free (object);
	return NULL;
}

NRObject *
nr_object_ref (NRObject *object)
{
	object->refcount += 1;
	return object;
}

NRObject *
nr_object_unref (NRObject *object)
{
	object->refcount -= 1;
	if (object->refcount < 1) {
		nr_object_delete (object);
	}
	return NULL;
}

/* Automatic lifecycle */

static void
nr_class_tree_object_invoke_init (NRObjectClass *klass, NRObject *object)
{
	if (klass->parent) {
		nr_class_tree_object_invoke_init (klass->parent, object);
	}
	klass->iinit (object);
}

NRObject *
nr_object_setup (NRObject *object, NRType type)
{
	NRObjectClass *klass;

	nr_return_val_if_fail (type < classes_len, NULL);

	klass = classes[type];

	memset (object, 0, klass->isize);
	object->klass = klass;
	object->refcount = 1;

	nr_class_tree_object_invoke_init (klass, object);

	return object;
}

NRObject *
nr_object_release (NRObject *object)
{
	object->klass->finalize (object);
	return NULL;
}

/* NRActiveObject */

static void nr_active_object_class_init (NRActiveObjectClass *klass);
static void nr_active_object_init (NRActiveObject *object);
static void nr_active_object_finalize (NRObject *object);

static NRObjectClass *parent_class;

NRType
nr_active_object_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_OBJECT,
						"NRActiveObject",
						sizeof (NRActiveObjectClass),
						sizeof (NRActiveObject),
						(void (*) (NRObjectClass *)) nr_active_object_class_init,
						(void (*) (NRObject *)) nr_active_object_init);
	}
	return type;
}

static void
nr_active_object_class_init (NRActiveObjectClass *klass)
{
	NRObjectClass *object_class;

	object_class = (NRObjectClass *) klass;

	parent_class = ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_active_object_finalize;
}

static void
nr_active_object_init (NRActiveObject *object)
{
}

static void
nr_active_object_finalize (NRObject *object)
{
	NRActiveObject *aobject;

	aobject = (NRActiveObject *) object;

	if (aobject->callbacks) {
		int i;
		for (i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener;
			listener = aobject->callbacks->listeners + i;
			if (listener->vector->dispose) listener->vector->dispose (object, listener->data);
		}
		free (aobject->callbacks);
	}

	((NRObjectClass *) (parent_class))->finalize (object);
}

void
nr_active_object_add_listener (NRActiveObject *object, const NRObjectEventVector *vector, unsigned int size, void *data)
{
	NRObjectListener *listener;

	if (!object->callbacks) {
		object->callbacks = malloc (sizeof (NRObjectCallbackBlock));
		object->callbacks->size = 1;
		object->callbacks->length = 0;
	}
	if (object->callbacks->length >= object->callbacks->size) {
		int newsize;
		newsize = object->callbacks->size << 1;
		object->callbacks = realloc (object->callbacks, sizeof (NRObjectCallbackBlock) + (newsize - 1) * sizeof (NRObjectListener));
		object->callbacks->size = newsize;
	}
	listener = object->callbacks->listeners + object->callbacks->length;
	listener->vector = vector;
	listener->size = size;
	listener->data = data;
	object->callbacks->length += 1;
}

void
nr_active_object_remove_listener_by_data (NRActiveObject *object, void *data)
{
	if (object->callbacks) {
		int i;
		for (i = 0; i < object->callbacks->length; i++) {
			NRObjectListener *listener;
			listener = object->callbacks->listeners + i;
			if (listener->data == data) {
				object->callbacks->length -= 1;
				if (object->callbacks->length < 1) {
					free (object->callbacks);
					object->callbacks = NULL;
				} else if (object->callbacks->length != i) {
					*listener = object->callbacks->listeners[object->callbacks->length];
				}
				return;
			}
		}
	}
}


