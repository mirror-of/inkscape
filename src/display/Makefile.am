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
#	nr-uta.c nr-uta.h \
#	nr-svp.c nr-svp.h \
#	nr-svp-uncross.c nr-svp-uncross.h \
#	nr-svp-render.c nr-svp-render.h
#
#noinst_PROGRAMS = testnr
#testnr_LDADD = \
#	#$(GNOME_LIBDIR) \
#	#$(GNOMEUI_LIBS)
#testnr_SOURCES = \
#	testnr.c \
#	$(nr_sources)
#else
#nr_sources =
#endif

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(SODIPODI_CFLAGS)

noinst_LIBRARIES = libspdisplay.a

libspdisplay_a_SOURCES = \
	nr-arena-forward.h \
	nr-arena.c nr-arena.h \
	nr-arena-item.c nr-arena-item.h \
	nr-arena-group.c nr-arena-group.h \
	nr-arena-image.c nr-arena-image.h \
	nr-arena-shape.c nr-arena-shape.h \
	nr-arena-glyphs.c nr-arena-glyphs.h \
	canvas-arena.c canvas-arena.h
