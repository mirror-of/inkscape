extdir = $(libdir)/inkscape/extensions
extdatadir = $(datadir)/inkscape/extensions
moduledir = $(datadir)/inkscape/modules

SUBDIRS = implementation internal

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-DINKSCAPE_EXTENSIONDIR=\""$(extdir)"\" \
	-DINKSCAPE_EXTENSIONDATADIR=\""$(extdatadir)"\" \
	-DINKSCAPE_MODULESDIR=\""$(moduledir)"\" \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS)

noinst_LIBRARIES = libextension.a

libextension_a_SOURCES = \
		extension.cpp \
		\
		db.cpp \
		db.h \
		init.cpp \
		init.h \
		menu.cpp \
		menu.h \
		system.cpp \
		system.h

