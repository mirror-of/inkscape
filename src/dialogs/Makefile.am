#
# Several object property dialogs
# Author: Lauris Kaplinski <lauris@ximian.com>
#
# Here should be things, that use only xml interface, not objects themselves
# Currently we still have selection_changed functions, but these will be
# replaced by selection 'changed' signal handlers
#

gladedir = $(datadir)/sodipodi

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-DSODIPODI_GLADEDIR=\""$(gladedir)"\" \
	-DSODIPODI_PIXMAPDIR=\""$(gladedir)"\" \
	$(SODIPODI_CFLAGS)

noinst_LIBRARIES = libspdialogs.a

libspdialogs_a_SOURCES = \
	sp-attribute-widget.c sp-attribute-widget.h \
	text-edit.c text-edit.h \
	export.c export.h \
	xml-tree.c xml-tree.h \
	align.c align.h	\
	display-settings.c display-settings.h \
	desktop-properties.c desktop-properties.h \
	document-properties.c document-properties.h \
	object-properties.c object-properties.h \
	item-properties.c item-properties.h \
	object-attributes.c object-attributes.h \
	tool-options.c tool-options.h \
	tool-attributes.c tool-attributes.h \
	fill-style.c fill-style.h \
	stroke-style.c stroke-style.h \
	transformation.c transformation.h \
	node-edit.c node-edit.h

#	sp-widget.c sp-widget.h \
#	gradient-vector.c gradient-vector.h \
#	gradient-selector.c gradient-selector.h

