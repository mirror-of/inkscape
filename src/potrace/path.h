/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

#ifndef PATH_H
#define PATH_H

#include "curve.h"
#include "bitmap.h"

int bm_to_pathlist(bitmap_t *bm, path_t **plistp);

#endif /* PATH_H */

