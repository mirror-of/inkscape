/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

#ifndef FLATE_H
#define FLATE_H

int dummy_xship(FILE *f, int filter, char *s, int len);
int flate_xship(FILE *f, int filter, char *s, int len);
int lzw_xship(FILE *f, int filter, char *s, int len);
int a85_xship(FILE *f, int filter, char *s, int len);

int a85init(FILE *f);
int a85finish(FILE *f);
int a85write(FILE *f, char *buf, int n);

#endif /* FLATE_H */
