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
#	nr-uta.cpp nr-uta.h \
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

noinst_LIBRARIES = libspdisplay.a

libspdisplay_a_SOURCES = \
	nr-arena-forward.h \
	nr-arena.cpp nr-arena.h \
	nr-arena-item.cpp nr-arena-item.h \
	nr-arena-group.cpp nr-arena-group.h \
	nr-arena-image.cpp nr-arena-image.h \
	nr-arena-shape.cpp nr-arena-shape.h \
	nr-arena-glyphs.cpp nr-arena-glyphs.h \
	canvas-arena.cpp canvas-arena.h
