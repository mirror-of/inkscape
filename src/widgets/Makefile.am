#
# Widgets for sodipodi
# Author: Lauris Kaplinski <lauris@ximian.com>
#

gladedir = $(datadir)/sodipodi

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(SODIPODI_CFLAGS) \
	-DSODIPODI_PIXMAPDIR=\""$(gladedir)"\"

noinst_LIBRARIES = libspwidgets.a

libspwidgets_a_SOURCES = \
	icon.c icon.h \
	button.c button.h \
	menu.c menu.h \
	ruler.c ruler.h \
	sp-toolbox.c sp-toolbox.h \
	sp-color-preview.c sp-color-preview.h \
	sp-color-slider.c sp-color-slider.h \
	sp-color-selector.c sp-color-selector.h \
	dash-selector.c dash-selector.h \
	gradient-image.c gradient-image.h \
	gradient-position.c gradient-position.h \
	paint-selector.c paint-selector.h \
	gradient-selector.c gradient-selector.h \
	gradient-vector.c gradient-vector.h \
	font-selector.c font-selector.h \
	sp-xmlview-tree.c sp-xmlview-tree.h \
	sp-xmlview-attr-list.c sp-xmlview-attr-list.h \
	sp-xmlview-content.c sp-xmlview-content.h \
	spw-utilities.c spw-utilities.h \
	sp-widget.c sp-widget.h

#	sp-value-winder.c sp-value-winder.h
#	sp-spin-button.c sp-spin-button.h

#noinst_PROGRAMS = test-widgets test-wrapbox
#noinst_PROGRAMS = test-wrapbox

#test_widgets_SOURCES = \
#	test-widgets.c \
#	sp-value-winder.c sp-value-winder.h \
#	sp-color-slider.c sp-color-slider.h \
#	sp-color-selector.c sp-color-selector.h

#test_widgets_LDADD = $(SODIPODI_LIBS)

#test_wrapbox_SOURCES = \
#	test-wrapbox.c \
#	sp-wrap-box.c sp-wrap-box.h \
#	sp-hwrap-box.c sp-hwrap-box.h \
#	sp-vwrap-box.c sp-vwrap-box.h

#test_wrapbox_LDADD = $(SODIPODI_LIBS)
