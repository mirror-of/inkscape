#ifndef __OBJECT_PROPERTIES_H__
#define __OBJECT_PROPERTIES_H__

/*
 * Basic object style dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Frank Felfe <iname@innerspace.com>
 *
 * Copyright (C) 2000-2001 Lauris Kaplinski and Frank Felfe
 * Copyright (C) 2001-2002 Ximian, Inc. and Lauris Kaplinski
 *
 * Released under GNU GPL
 */

void sp_object_properties_dialog (void);

void sp_object_properties_stroke (void);
void sp_object_properties_fill (void);
void sp_object_properties_layout (void);

void sp_object_properties_reread_page (void);
void sp_object_properties_selection_changed (void);
void sp_object_properties_close (void);
void sp_object_properties_apply (void);


#endif
