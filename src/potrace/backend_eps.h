/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

#ifndef BACKEND_EPS_H
#define BACKEND_EPS_H

#include "curve.h"
#include "main.h"

int init_ps(FILE *fout);
int page_ps(FILE *fout, path_t *plist, imginfo_t *imginfo);
int term_ps(FILE *fout);

int page_eps(FILE *fout, path_t *plist, imginfo_t *imginfo);

#endif /* BACKEND_EPS_H */

