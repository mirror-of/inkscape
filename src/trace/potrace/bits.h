/* Copyright (C) 2001-2004 Peter Selinger.
   This file is part of potrace. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* $Id$ */

/* bits.h: this file defines some macros for bit manipulations. We
   provide a generic implementation, as well as machine- and
   compiler-specific fast implementations */

/* lobit: return the position of the rightmost "1" bit of an int, or
   32 if none. hibit: return 1 + the position of the leftmost "1" bit
   of an int, or 0 if none. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* generic functions */
unsigned int generic_lobit(unsigned int x);
unsigned int generic_hibit(unsigned int x);

/* machine specific macros */

#if defined(HAVE_I386) && defined(__GNUC__)

#define lobit(x) ({				\
  unsigned int __arg = (x);			\
  int __res;					\
  asm ("bsf	%1,%0\n\t"			\
       "jnz	0f\n\t"				\
       "movl	$32,%0\n"			\
       "0:"					\
       : "=r" (__res)				\
       : "r" (__arg));				\
  __res;					\
})

#define hibit(x) ({				\
  unsigned int __arg = (x);			\
  int __res;					\
  asm ("bsr	%1,%0\n\t"			\
       "jnz	0f\n\t"				\
       "movl	$-1,%0\n"			\
       "0:"					\
       : "=r" (__res)				\
       : "r" (__arg));				\
  __res+1;					\
})

#else /* generic macros */

#define lobit(x) generic_lobit(x)
#define hibit(x) generic_hibit(x)

#endif 
