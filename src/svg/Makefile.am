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
	$(SODIPODI_CFLAGS) \
	-I$(includedir)

noinst_LIBRARIES = libspsvg.a

libspsvg_a_SOURCES = \
	svg.h \
	svg-types.h \
	svg-length.c \
	svg-color.c \
	svg-affine.c \
	svg-path.c \
	gnome-canvas-bpath-util.c gnome-canvas-bpath-util.h
