#ifndef __PATH_CHEMISTRY_H__
#define __PATH_CHEMISTRY_H__

/*
 * Here are handlers for modifying selections, specific to paths
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"

void sp_selected_path_combine (void);
void sp_selected_path_break_apart (void);
void sp_selected_path_to_curves (void);

void sp_path_cleanup (SPPath *path);

#endif
