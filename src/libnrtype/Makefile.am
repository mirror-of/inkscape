noinst_LIBRARIES = libnrtype.a

if USE_GNOME_PRINT
gnome_print_sources = nr-type-gnome.cpp nr-type-gnome.h
endif

if USE_XFT
xft_sources = nr-type-xft.cpp nr-type-xft.h
endif

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(FREETYPE_CFLAGS) \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS) \
	$(XFT_CFLAGS)

libnrtype_a_SOURCES = \
	nr-type-primitives.cpp nr-type-primitives.h \
	nr-typeface.cpp nr-typeface.h \
	nr-font.cpp nr-font.h \
	nr-rasterfont.cpp nr-rasterfont.h \
	nr-type-directory.cpp nr-type-directory.h \
	nr-type-ft2.cpp nr-type-ft2.h \
	$(gnome_print_sources) \
	$(xft_sources)

EXTRA_DIST = \
	nr-type-xft.cpp nr-type-xft.h \
	nr-type-gnome.cpp nr-type-gnome.h \
	nr-type-w32.cpp nr-type-w32.h \
	codepages.h
