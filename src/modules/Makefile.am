
if USE_GNOME_PRINT
gnome_print_sources = gnome.cpp gnome.h
endif

if PLATFORM_WIN32
win32_sources = win32.cpp win32.h
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
		init.cpp \
		init.h \
		menu.cpp \
		menu.h \
		db.cpp \
		db.h \
		system.cpp \
		system.h \
\
		extension.cpp \
		extension.h \
\
		svg.cpp \
		svg.h \
\
		ps.cpp ps.h \
		$(kdeprivsources) \
		$(gnome_print_sources) \
		$(win32_sources)

EXTRA_DIST = \
	$(gnome_print_sources) \
	win32.cpp win32.h

