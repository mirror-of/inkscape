/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

#ifndef BACKEND_XFIG_H
#define BACKEND_XFIG_H

#include <stdio.h>
#include "curve.h"
#include "main.h"

int page_xfig(FILE *fout, path_t *plist, imginfo_t *imginfo);

#endif /* BACKEND_XFIG_H */

