noinst_LIBRARIES = libnrtype.a

if USE_GNOME_PRINT
gnome_print_sources = nr-type-gnome.cpp nr-type-gnome.h
endif

if USE_XFT
xft_sources = nr-type-xft.cpp nr-type-xft.h
endif

if PLATFORM_WIN32
win32_sources = nr-type-w32.cpp nr-type-w32.h \
		codepages.h \
                cp1251.cpp cp1253.cpp cp1255.cpp \
                cp1257.cpp cp874.cpp cp936.cpp \
                cp950.cpp cp1250.cpp cp1252.cpp \
                cp1254.cpp cp1256.cpp cp1258.cpp \
                cp932.cpp cp949.cpp
endif 

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(FREETYPE_CFLAGS) \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS) \
	$(XFT_CFLAGS)

libnrtype_a_SOURCES = \
	font-style-to-pos.cpp font-style-to-pos.h	\
	nr-type-primitives.cpp nr-type-primitives.h \
	nr-typeface.cpp nr-typeface.h \
	nr-font.cpp nr-font.h \
	nr-rasterfont.cpp nr-rasterfont.h \
	nr-type-directory.cpp nr-type-directory.h \
	nr-type-pos-def.cpp nr-type-pos-def.h	\
	nr-type-ft2.cpp nr-type-ft2.h \
	$(win32_sources) \
	$(gnome_print_sources) \
	$(xft_sources)

EXTRA_DIST = \
	nr-type-xft.cpp nr-type-xft.h \
	nr-type-gnome.cpp nr-type-gnome.h \
	nr-type-w32.cpp nr-type-w32.h \
	codepages.h
