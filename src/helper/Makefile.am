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

TESTS = bezier-utils-test$(EXEEXT)
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS = bezier-utils-test

libspchelp_a_SOURCES = \
	sp-intl.h \
	helper-forward.h \
	window.cpp window.h \
	sp-marshal.c sp-marshal.h \
	sp-canvas.cpp sp-canvas.h \
	sp-canvas-util.cpp sp-canvas-util.h \
	gnome-utils.cpp gnome-utils.h \
	curve.cpp curve.h \
	gnome-canvas-acetate.cpp gnome-canvas-acetate.h \
	sodipodi-ctrl.cpp sodipodi-ctrl.h \
	sodipodi-ctrlrect.cpp sodipodi-ctrlrect.h \
	sp-ctrlline.cpp sp-ctrlline.h \
	guideline.cpp guideline.h \
	canvas-grid.cpp canvas-grid.h \
	canvas-bpath.cpp canvas-bpath.h \
	png-write.cpp png-write.h \
	nr-plain-stuff.cpp nr-plain-stuff.h \
	nr-plain-stuff-gdk.cpp nr-plain-stuff-gdk.h \
	nr-gradient-gpl.cpp nr-gradient-gpl.h \
	action.cpp action.h \
	bezier-utils.cpp bezier-utils.h \
	units.cpp units.h \
	unit-menu.cpp unit-menu.h

#	pathops.c pathops.h

sp-marshal.h: sp-marshal.list
	glib-genmarshal --prefix=sp_marshal --header $(srcdir)/sp-marshal.list > tmp.$$$$ && \
	 mv tmp.$$$$ sp-marshal.h

sp-marshal.c: sp-marshal.list sp-marshal.h
	( echo '#include "helper/sp-marshal.h"' &&	\
	  glib-genmarshal --prefix=sp_marshal --body $(srcdir)/sp-marshal.list )	\
	 > tmp.$$$$ &&	\
	 mv tmp.$$$$ sp-marshal.c

sp-marshal.c sp-marshal.h: Makefile

EXTRA_DIST = sp-marshal.list

DISTCLEANFILES = sp-marshal.c sp-marshal.h

bezier_utils_test_SOURCES = bezier-utils-test.cpp
bezier_utils_test_LDADD = ../libnr/nr-point-fns.o ../libnr/nr-types.o -lglib-2.0
