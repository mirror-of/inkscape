noinst_LIBRARIES = libnrtype.a

if USE_GNOME_PRINT
gnome_print_sources = nr-type-gnome.c nr-type-gnome.h
endif

if USE_XFT
xft_sources = nr-type-xft.c nr-type-xft.h
endif

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(FREETYPE_CFLAGS) \
	$(SODIPODI_CFLAGS) \
	$(GNOME_PRINT_CFLAGS) \
	$(XFT_CFLAGS)

libnrtype_a_SOURCES = \
	nr-type-primitives.c nr-type-primitives.h \
	nr-typeface.c nr-typeface.h \
	nr-font.c nr-font.h \
	nr-rasterfont.c nr-rasterfont.h \
	nr-type-directory.c nr-type-directory.h \
	nr-type-ft2.c nr-type-ft2.h \
	$(gnome_print_sources) \
	$(xft_sources)

EXTRA_DIST = \
	nr-type-xft.c nr-type-xft.h \
	nr-type-gnome.c nr-type-gnome.h \
	nr-type-w32.c nr-type-w32.h \
	codepages.h
