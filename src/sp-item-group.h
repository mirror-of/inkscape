#ifndef __SP_ITEM_GROUP_H__
#define __SP_ITEM_GROUP_H__

/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-item.h"

#define SP_TYPE_GROUP            (sp_group_get_type ())
#define SP_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GROUP, SPGroup))
#define SP_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GROUP, SPGroupClass))
#define SP_IS_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GROUP))
#define SP_IS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GROUP))

typedef enum _SPGroupMode {
	SP_GROUP_MODE_GROUP,
	SP_GROUP_MODE_LAYER
} SPGroupMode;

struct SPGroup {
	SPItem item;
	SPGroupMode mode;
};

struct SPGroupClass {
	SPItemClass parent_class;
};

GType sp_group_get_type (void);

void sp_item_group_ungroup (SPGroup *group, GSList **children);

SPGroupMode sp_item_group_get_mode (SPGroup *group);
void sp_item_group_set_mode (SPGroup *group, SPGroupMode mode);

GSList *sp_item_group_item_list (SPGroup *group);
SPObject *sp_item_group_get_child_by_name (SPGroup *group, SPObject *ref, const gchar *name);

#endif
