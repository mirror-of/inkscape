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

GtkWidget *sp_tool_toolbox_new ();
void sp_tool_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_aux_toolbox_new ();
void sp_aux_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop);

#endif
