#define __SP_ANIMATION_C__

/*
 * SVG <animate> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "attributes.h"
#include "sp-animation.h"

/* Animation base class */

static void sp_animation_class_init (SPAnimationClass *klass);
static void sp_animation_init (SPAnimation *animation);

static void sp_animation_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_animation_release (SPObject *object);
static void sp_animation_set (SPObject *object, unsigned int key, const unsigned char *value);

static SPObjectClass *animation_parent_class;

GType
sp_animation_get_type (void)
{
	static GType animation_type = 0;

	if (!animation_type) {
		GTypeInfo animation_info = {
			sizeof (SPAnimationClass),
			NULL, NULL,
			(GClassInitFunc) sp_animation_class_init,
			NULL, NULL,
			sizeof (SPAnimation),
			16,
			(GInstanceInitFunc) sp_animation_init,
		};
		animation_type = g_type_register_static (SP_TYPE_OBJECT, "SPAnimation", &animation_info, 0);
	}
	return animation_type;
}

static void
sp_animation_class_init (SPAnimationClass *klass)
{
	GObjectClass *gobject_class;
	SPObjectClass *sp_object_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	animation_parent_class = g_type_class_peek_parent (klass);

	sp_object_class->build = sp_animation_build;
	sp_object_class->release = sp_animation_release;
	sp_object_class->set = sp_animation_set;
}

static void
sp_animation_init (SPAnimation *animation)
{
}


static void
sp_animation_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) animation_parent_class)->build)
		((SPObjectClass *) animation_parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "xlink:href");
	sp_object_read_attr (object, "attributeName");
	sp_object_read_attr (object, "attributeType");
	sp_object_read_attr (object, "begin");
	sp_object_read_attr (object, "dur");
	sp_object_read_attr (object, "end");
	sp_object_read_attr (object, "min");
	sp_object_read_attr (object, "max");
	sp_object_read_attr (object, "restart");
	sp_object_read_attr (object, "repeatCount");
	sp_object_read_attr (object, "repeatDur");
	sp_object_read_attr (object, "fill");
}

static void
sp_animation_release (SPObject *object)
{
}

static void
sp_animation_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPAnimation * animation;

	animation = SP_ANIMATION (object);

	g_print ("SPAnimation: Set %s %s\n", sp_attribute_name (key), value);

	if (((SPObjectClass *) animation_parent_class)->set)
		((SPObjectClass *) animation_parent_class)->set (object, key, value);
}

/* Interpolated animation base class */

static void sp_ianimation_class_init (SPIAnimationClass *klass);
static void sp_ianimation_init (SPIAnimation *animation);

static void sp_ianimation_build (SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_ianimation_release (SPObject *object);
static void sp_ianimation_set (SPObject *object, unsigned int key, const unsigned char *value);

static SPObjectClass *ianimation_parent_class;

GType
sp_ianimation_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (SPIAnimationClass),
			NULL, NULL,
			(GClassInitFunc) sp_ianimation_class_init,
			NULL, NULL,
			sizeof (SPIAnimation),
			16,
			(GInstanceInitFunc) sp_ianimation_init,
		};
		type = g_type_register_static (SP_TYPE_OBJECT, "SPIAnimation", &info, 0);
	}
	return type;
}

static void
sp_ianimation_class_init (SPIAnimationClass *klass)
{
	GObjectClass *gobject_class;
	SPObjectClass *sp_object_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	ianimation_parent_class = g_type_class_peek_parent (klass);

	sp_object_class->build = sp_ianimation_build;
	sp_object_class->release = sp_ianimation_release;
	sp_object_class->set = sp_ianimation_set;
}

static void
sp_ianimation_init (SPIAnimation *animation)
{
}


static void
sp_ianimation_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) ianimation_parent_class)->build)
		((SPObjectClass *) ianimation_parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "calcMode");
	sp_object_read_attr (object, "values");
	sp_object_read_attr (object, "keyTimes");
	sp_object_read_attr (object, "keySplines");
	sp_object_read_attr (object, "from");
	sp_object_read_attr (object, "to");
	sp_object_read_attr (object, "by");
	sp_object_read_attr (object, "additive");
	sp_object_read_attr (object, "accumulate");
}

static void
sp_ianimation_release (SPObject *object)
{
}

static void
sp_ianimation_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPIAnimation *ianimation;

	ianimation = SP_IANIMATION (object);

	g_print ("SPIAnimation: Set %s %s\n", sp_attribute_name (key), value);

	if (((SPObjectClass *) ianimation_parent_class)->set)
		((SPObjectClass *) ianimation_parent_class)->set (object, key, value);
}

/* SVG <animate> */

static void sp_animate_class_init (SPAnimateClass *klass);
static void sp_animate_init (SPAnimate *animate);

static void sp_animate_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_animate_release (SPObject *object);
static void sp_animate_set (SPObject *object, unsigned int key, const unsigned char *value);

static SPIAnimationClass *animate_parent_class;

GType
sp_animate_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (SPAnimateClass),
			NULL, NULL,
			(GClassInitFunc) sp_animate_class_init,
			NULL, NULL,
			sizeof (SPAnimate),
			16,
			(GInstanceInitFunc) sp_animate_init,
		};
		type = g_type_register_static (SP_TYPE_IANIMATION, "SPAnimate", &info, 0);
	}
	return type;
}

static void
sp_animate_class_init (SPAnimateClass *klass)
{
	GObjectClass *gobject_class;
	SPObjectClass *sp_object_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	animate_parent_class = g_type_class_peek_parent (klass);

	sp_object_class->build = sp_animate_build;
	sp_object_class->release = sp_animate_release;
	sp_object_class->set = sp_animate_set;
}

static void
sp_animate_init (SPAnimate *animate)
{
}


static void
sp_animate_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) animate_parent_class)->build)
		((SPObjectClass *) animate_parent_class)->build (object, document, repr);
}

static void
sp_animate_release (SPObject *object)
{
}

static void
sp_animate_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPAnimate * animate;

	animate = SP_ANIMATE (object);

	g_print ("SPAnimate: Set %s %s\n", sp_attribute_name (key), value);

	if (((SPObjectClass *) animate_parent_class)->set)
		((SPObjectClass *) animate_parent_class)->set (object, key, value);
}

