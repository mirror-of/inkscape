
if USE_GNOME_PRINT
gnome_print_sources = gnome.c gnome.h
endif

if USE_KDE
kdeincludes = $(all_includes)
kdesources = kde-private.h kde.cpp kde.h
kdeprivsources = kde-private.cpp
kde-private.cpp: kde-private.h
	$(MOC) kde-private.h -o kde-private.cpp
endif

extdir = $(libdir)/sodipodi/extensions
extdatadir = $(datadir)/sodipodi/extensions

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-DSODIPODI_EXTENSIONDIR=\""$(extdir)"\" \
	-DSODIPODI_EXTENSIONDATADIR=\""$(extdatadir)"\" \
	$(SODIPODI_CFLAGS) \
	$(GNOME_PRINT_CFLAGS) \
	$(kdeincludes)

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
	$(gnome_print_sources) \
	$(kdesources)

EXTRA_DIST = \
	$(gnome_print_sources) \
	$(kdesources) \
	win32.c win32.h

