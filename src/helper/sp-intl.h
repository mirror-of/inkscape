#ifndef __SP_INTL_H__
#define __SP_INTL_H__

/*
 * Wrappers around internationalization code
 *
 * Author:
 *  Mitsuru Oka
 *
 * Copyright (C) 2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#ifdef ENABLE_NLS
#include<libintl.h>
#undef _
#undef N_
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#endif

#endif
