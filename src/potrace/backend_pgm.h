/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

#ifndef BACKEND_PGM_H
#define BACKEND_PGM_H

#include "curve.h"
#include "main.h"

int page_pgm(FILE *fout, path_t *plist, imginfo_t *imginfo);

#endif /* BACKEND_PGM_H */
