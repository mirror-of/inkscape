
if USE_GNOME_PRINT
gnome_print_sources = gnome.c gnome.h
endif

extdir = $(libdir)/inkscape/extensions
extdatadir = $(datadir)/inkscape/extensions
moduledir = $(datadir)/inkscape/modules

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-DINKSCAPE_EXTENSIONDIR=\""$(extdir)"\" \
	-DINKSCAPE_EXTENSIONDATADIR=\""$(extdatadir)"\" \
	-DINKSCAPE_MODULESDIR=\""$(moduledir)"\" \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS)

noinst_LIBRARIES = libspmodules.a

libspmodules_a_SOURCES = \
		init.c \
		init.h \
		menu.c \
		menu.h \
		db.c \
		db.h \
		system.c \
		system.h \
\
		extension.c \
		extension.h \
\
		svg.c \
		svg.h \
\
		ps.c ps.h \
		$(kdeprivsources) \
		$(gnome_print_sources)

EXTRA_DIST = \
	$(gnome_print_sources) \
	win32.c win32.h

