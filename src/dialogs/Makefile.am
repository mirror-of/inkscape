#
# Several object property dialogs
# Author: Lauris Kaplinski <lauris@ximian.com>
#
# Here should be things, that use only xml interface, not objects themselves
# Currently we still have selection_changed functions, but these will be
# replaced by selection 'changed' signal handlers
#

iconsdir = $(datadir)/inkscape

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	-DINKSCAPE_ICONS_DIR=\""$(iconsdir)"\" \
	-DINKSCAPE_PIXMAPDIR=\""$(iconsdir)"\" \
	$(INKSCAPE_CFLAGS)

noinst_LIBRARIES = libspdialogs.a

libspdialogs_a_SOURCES = \
	sp-attribute-widget.cpp sp-attribute-widget.h \
	text-edit.cpp text-edit.h \
	export.cpp export.h \
	xml-tree.cpp xml-tree.h \
	align.cpp align.h	\
	display-settings.cpp display-settings.h \
	desktop-properties.cpp desktop-properties.h \
	filedialog.cpp filedialog.h \
	object-properties.cpp object-properties.h \
	item-properties.cpp item-properties.h \
	object-attributes.cpp object-attributes.h \
	tool-options.cpp tool-options.h \
	fill-style.cpp fill-style.h \
	stroke-style.cpp stroke-style.h \
	transformation.cpp transformation.h \
	dialog-events.cpp dialog-events.h

#	sp-widget.c sp-widget.h \
#	gradient-vector.c gradient-vector.h \
#	gradient-selector.c gradient-selector.h

