#ifndef __SP_MAINTOOLBOX_H__
#define __SP_MAINTOOLBOX_H__

/**
 * \brief Main toolbox
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

GtkWidget *sp_tool_toolbox_new (void);
void sp_tool_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_aux_toolbox_new (void);
void sp_aux_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_toolbox_button_normal_new_from_verb ( GtkWidget *t,
                                                    unsigned int size,
                                                    sp_verb_t verb,
                                                    SPView *view,
                                                    GtkTooltips *tt );

void aux_toolbox_space (GtkWidget *tb, gint space);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
