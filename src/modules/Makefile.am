
if USE_GNOME_PRINT
gnome_print_sources = gnome.c gnome.h
endif

extdir = $(libdir)/inkscape/extensions
extdatadir = $(datadir)/inkscape/extensions

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-DINKSCAPE_EXTENSIONDIR=\""$(extdir)"\" \
	-DINKSCAPE_EXTENSIONDATADIR=\""$(extdatadir)"\" \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS)

noinst_LIBRARIES = libspmodules.a

libspmodules_a_SOURCES = \
	sp-module.c \
	sp-module-doc.c \
	sp-module-doc.h \
	sp-module-exec.c \
	sp-module-exec-ext.c \
	sp-module-exec-ext.h \
	sp-module-exec.h \
	sp-module.h \
	sp-module-sys.c \
	sp-module-sys.h \
	ps.c ps.h \
	$(gnome_print_sources)

EXTRA_DIST = \
	$(gnome_print_sources) \
	win32.c win32.h

