/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

/* generic implementation of bit manipulations. These are only used if
   fast machine-specific implementations are unavailable. These
   functions work on 32-bit integers. */

/* lobit: return the position of the rightmost "1" bit of an int, or
   32 if none. hibit: return 1 + the position of the leftmost "1" bit
   of an int, or 0 if none. */

unsigned int generic_lobit(unsigned int x) {
  unsigned int res = 32;
  while (x & 0xffffff) {
    x <<= 8;
    res -= 8;
  }
  while (x) {
    x <<= 1;
    res -= 1;
  }
  return res;
}

unsigned int generic_hibit(unsigned int x) {
  unsigned int res = 0;
  while (x > 0xff) {
    x >>= 8;
    res += 8;
  }
  while (x) {
    x >>= 1;
    res += 1;
  }
  return res;
}

