
if USE_GNOME_PRINT
gnome_print_sources = gnome.cpp gnome.h
endif

extdir = $(libdir)/inkscape/extensions
extdatadir = $(datadir)/inkscape/extensions
moduledir = $(datadir)/inkscape/modules

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/extension \
	-DINKSCAPE_EXTENSIONDIR=\""$(extdir)"\" \
	-DINKSCAPE_EXTENSIONDATADIR=\""$(extdatadir)"\" \
	-DINKSCAPE_MODULESDIR=\""$(moduledir)"\" \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS)

noinst_LIBRARIES = libinternal.a

libinternal_a_SOURCES = \
		svg.h svg.cpp \
		ps.h ps.cpp \
		ps-out.h ps-out.cpp \
		eps-out.h eps-out.cpp \
		$(gnome_print_sources)

EXTRA_DIST = \
	gnome.cpp gnome.h \
	win32.cpp win32.h

