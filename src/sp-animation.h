#ifndef __SP_ANIMATION_H__
#define __SP_ANIMATION_H__

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

#include "sp-object.h"

G_BEGIN_DECLS

/* Animation base class */

#define SP_TYPE_ANIMATION (sp_animation_get_type ())
#define SP_ANIMATION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_ANIMATION, SPAnimation))
#define SP_IS_ANIMATION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ANIMATION))

typedef struct _SPAnimation SPAnimation;
typedef struct _SPAnimationClass SPAnimationClass;

struct _SPAnimation {
	SPObject object;
};

struct _SPAnimationClass {
	SPObjectClass parent_class;
};

GType sp_animation_get_type (void);

/* Interpolated animation base class */

#define SP_TYPE_IANIMATION (sp_ianimation_get_type ())
#define SP_IANIMATION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_IANIMATION, SPIAnimation))
#define SP_IS_IANIMATION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_IANIMATION))

typedef struct _SPIAnimation SPIAnimation;
typedef struct _SPIAnimationClass SPIAnimationClass;

struct _SPIAnimation {
	SPAnimation animation;
};

struct _SPIAnimationClass {
	SPAnimationClass parent_class;
};

GType sp_ianimation_get_type (void);

/* SVG <animate> */

#define SP_TYPE_ANIMATE (sp_animate_get_type ())
#define SP_ANIMATE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_ANIMATE, SPAnimate))
#define SP_IS_ANIMATE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ANIMATE))

typedef struct _SPAnimate SPAnimate;
typedef struct _SPAnimateClass SPAnimateClass;

struct _SPAnimate {
	SPIAnimation animation;
};

struct _SPAnimateClass {
	SPIAnimationClass parent_class;
};

GType sp_animate_get_type (void);

G_END_DECLS

#endif
