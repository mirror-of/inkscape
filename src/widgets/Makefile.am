#
# Widgets for inkscape
# Author: Lauris Kaplinski <lauris@ximian.com>
#

iconsdir = $(datadir)/inkscape

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(INKSCAPE_CFLAGS) \
	-DINKSCAPE_PIXMAPDIR=\""$(iconsdir)"\"

noinst_LIBRARIES = libspwidgets.a

libspwidgets_a_SOURCES = \
	icon.cpp icon.h \
	button.cpp button.h \
	menu.cpp menu.h \
	ruler.cpp ruler.h \
	sp-color-preview.cpp sp-color-preview.h \
	sp-color-slider.cpp sp-color-slider.h \
	sp-color-selector.cpp sp-color-selector.h \
	sp-color-notebook.cpp sp-color-notebook.h \
	sp-color-scales.cpp sp-color-scales.h \
	sp-color-gtkselector.cpp sp-color-gtkselector.h \
	dash-selector.cpp dash-selector.h \
	gradient-image.cpp gradient-image.h \
	gradient-position.cpp gradient-position.h \
	paint-selector.cpp paint-selector.h \
	gradient-selector.cpp gradient-selector.h \
	gradient-vector.cpp gradient-vector.h \
	font-selector.cpp font-selector.h \
	sp-xmlview-tree.cpp sp-xmlview-tree.h \
	sp-xmlview-attr-list.cpp sp-xmlview-attr-list.h \
	sp-xmlview-content.cpp sp-xmlview-content.h \
	spw-utilities.cpp spw-utilities.h \
	sp-widget.cpp sp-widget.h\
	spinbutton-events.cpp spinbutton-events.h \
	widget-sizes.h

#	sp-value-winder.c sp-value-winder.h
#	sp-spin-button.cpp sp-spin-button.h

#noinst_PROGRAMS = test-widgets test-wrapbox
#noinst_PROGRAMS = test-wrapbox

#test_widgets_SOURCES = \
#	test-widgets.cpp \
#	sp-value-winder.c sp-value-winder.h \
#	sp-color-slider.cpp sp-color-slider.h \
#	sp-color-selector.cpp sp-color-selector.h

#test_widgets_LDADD = $(INKSCAPE_LIBS)

#test_wrapbox_SOURCES = \
#	test-wrapbox.cpp \
#	sp-wrap-box.cpp sp-wrap-box.h \
#	sp-hwrap-box.cpp sp-hwrap-box.h \
#	sp-vwrap-box.cpp sp-vwrap-box.h

#test_wrapbox_LDADD = $(INKSCAPE_LIBS)
