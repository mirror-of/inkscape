#
# Miscellaneous unsystematized and temporary helper utilities
#
# libspchelp - canvas utilities, specific canvas items, libart add-ons
#

noinst_LIBRARIES = libspchelp.a

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(cflags) \
	$(INKSCAPE_CFLAGS)

libspchelp_a_SOURCES = \
	sp-intl.h \
	helper-forward.h \
	window.cpp window.h \
	sp-marshal.cpp sp-marshal.h \
	gnome-utils.cpp gnome-utils.h \
	png-write.cpp png-write.h \
	action.cpp action.h \
	units.cpp units.h \
	unit-menu.cpp unit-menu.h

sp-marshal.h: sp-marshal.list
	glib-genmarshal --prefix=sp_marshal --header $(srcdir)/sp-marshal.list > tmp.$$$$ && \
	 mv tmp.$$$$ sp-marshal.h

sp-marshal.cpp: sp-marshal.list sp-marshal.h
	( echo '#include "helper/sp-marshal.h"' &&	\
	  glib-genmarshal --prefix=sp_marshal --body $(srcdir)/sp-marshal.list )	\
	 > tmp.$$$$ &&	\
	 mv tmp.$$$$ sp-marshal.cpp

sp-marshal.c sp-marshal.h: Makefile

EXTRA_DIST = sp-marshal.list

DISTCLEANFILES = sp-marshal.cpp sp-marshal.h
