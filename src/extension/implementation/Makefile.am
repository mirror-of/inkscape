
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

noinst_LIBRARIES = libimplementation.a

libimplementation_a_SOURCES = \
		script.cpp

