#ifndef __SP_MAINTOOLBOX_H__
#define __SP_MAINTOOLBOX_H__

/*
 * Main toolbox
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"

#define TOOL_BUTTON_SIZE 28
#define AUX_BUTTON_SIZE 20
#define AUX_BETWEEN_BUTTON_GROUPS 5
#define AUX_FONT_SIZE 8000

GtkWidget *sp_tool_toolbox_new (void);
void sp_tool_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_aux_toolbox_new (void);
void sp_aux_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_toolbox_button_normal_new_from_verb (GtkWidget *t, unsigned int size, sp_verb_t verb, SPView *view, GtkTooltips *tt);

#endif
