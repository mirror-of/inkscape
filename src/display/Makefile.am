#
# Sodipodi GnomeCanvas objects
# Author: Lauris Kaplinski <lauris@ariman.ee>
#
# Here are major objects, used for displaying things
#

#if WITH_ARENA
#SUBDIRS = nr
#endif

#if WITH_NEW_RENDER
#nr_sources = \
#	nr-svp.c nr-svp.h \
#	nr-svp-uncross.c nr-svp-uncross.h \
#	nr-svp-render.c nr-svp-render.h
#
#noinst_PROGRAMS = testnr
#testnr_LDADD = \
#	#$(GNOME_LIBDIR) \
#	#$(GNOMEUI_LIBS)
#testnr_SOURCES = \
#	testnr.cpp \
#	$(nr_sources)
#else
#nr_sources =
#endif

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(INKSCAPE_CFLAGS)


TESTS = bezier-utils-test$(EXEEXT)
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS = bezier-utils-test

noinst_LIBRARIES = libspdisplay.a

libspdisplay_a_SOURCES = \
	nr-arena-forward.h \
	nr-arena.cpp nr-arena.h \
	nr-arena-item.cpp nr-arena-item.h \
	nr-arena-group.cpp nr-arena-group.h \
	nr-arena-image.cpp nr-arena-image.h \
	nr-arena-shape.cpp nr-arena-shape.h \
	nr-arena-glyphs.cpp nr-arena-glyphs.h \
	canvas-arena.cpp canvas-arena.h \
	bezier-utils.cpp bezier-utils.h \
	canvas-bpath.cpp canvas-bpath.h canvas-grid.cpp canvas-grid.h \
	curve.cpp curve.h gnome-canvas-acetate.cpp gnome-canvas-acetate.h \
	guideline.cpp guideline.h \
	nr-gradient-gpl.cpp nr-gradient-gpl.h \
	nr-plain-stuff-gdk.cpp nr-plain-stuff-gdk.h nr-plain-stuff.cpp nr-plain-stuff.h \
	sodipodi-ctrl.cpp sodipodi-ctrl.h sodipodi-ctrlrect.cpp sodipodi-ctrlrect.h \
	sp-canvas-util.cpp sp-canvas-util.h sp-canvas.cpp sp-canvas.h \
	sp-ctrlline.cpp sp-ctrlline.h


bezier_utils_test_SOURCES = bezier-utils-test.cpp
bezier_utils_test_LDADD = ../libnr/nr-point-fns.o ../libnr/nr-types.o -lglib-2.0
