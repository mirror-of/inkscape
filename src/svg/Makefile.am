#
# Sodipodi svg wrapper
# Author: Lauris Kaplinski <lauris@ariman.ee>
# Most code is actually written by Raph Levien
#
# Implements svg interpretation of xml tree
#

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/svg \
	$(INKSCAPE_CFLAGS) \
	-I$(includedir)

noinst_LIBRARIES = libspsvg.a

libspsvg_a_SOURCES = \
	svg.h \
	svg-types.h \
	svg-length.cpp \
	svg-color.cpp \
	svg-affine.cpp \
	svg-path.cpp \
	stringstream.h \
	gnome-canvas-bpath-util.cpp gnome-canvas-bpath-util.h
