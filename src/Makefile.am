## Process this file with automake to produce Makefile.in

module_libs = modules/libspmodules.a
module_src = module.cpp module.h

SUBDIRS = libarikkei libnr livarot libnrtype xml svg helper widgets dialogs display modules utest

AM_CPPFLAGS = \
	-DINKSCAPE_VERSION=\""$(VERSION)"\"

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DINKSCAPE_PIXMAPDIR=\""$(datadir)/inkscape"\" \
	-DINKSCAPE_DATADIR=\""$(datadir)"\" \
	-I$(srcdir) \
	-I$(srcdir)/xml \
	-I$(srcdir)/svg \
	$(INKSCAPE_CFLAGS) \
	$(GNOME_PRINT_CFLAGS)

check-recursive: all-recursive
# So that test programs can link against object files in other directories.
# We can get rid of this if we switch to a single makefile in src or toplevel
# that contains all the rules & dependencies for its (recursive) subdirectories
# (*see (automake-1)Alternative, in the Texinfo documentation for automake).
# Disadvantages of adding this all-recursive dependency: `make check' takes
# longer; and the dependency won't have any effect if someone does `make check'
# from a subdirectory.

bin_PROGRAMS = inkscape inkview

inkscapelib_src = \
	macros.h \
	forward.h \
	enums.h \
	attributes.cpp attributes.h \
	monostd.h	\
	sp-object.cpp sp-object.h \
	sp-object-repr.cpp sp-object-repr.h \
	sp-object-group.cpp sp-object-group.h \
	sp-defs.cpp sp-defs.h \
	sp-item.cpp sp-item.h \
	sp-item-group.cpp sp-item-group.h \
	sp-symbol.cpp sp-symbol.h \
	sp-marker.cpp sp-marker.h \
	sp-use.cpp sp-use.h \
	sp-anchor.cpp sp-anchor.h \
	sp-root.cpp sp-root.h \
	sp-image.cpp sp-image.h \
	brokenimage.xpm \
	sp-path.cpp sp-path.h \
	sp-shape.cpp sp-shape.h \
	sp-rect.cpp sp-rect.h \
	sp-ellipse.cpp sp-ellipse.h \
	sp-star.cpp sp-star.h \
	sp-spiral.cpp sp-spiral.h \
	sp-offset.cpp sp-offset.h \
	sp-line.cpp sp-line.h \
	sp-polyline.cpp sp-polyline.h \
	sp-polygon.cpp sp-polygon.h \
	sp-chars.cpp sp-chars.h \
	sp-text.cpp sp-text.h \
	sp-paint-server.cpp sp-paint-server.h \
	sp-gradient.cpp sp-gradient.h \
	sp-pattern.cpp sp-pattern.h \
	sp-clippath.cpp sp-clippath.h \
	sp-mask.cpp sp-mask.h \
	sp-animation.cpp sp-animation.h \
	\
	color.cpp color.h \
	style.cpp style.h \
	\
	document.cpp document.h document-private.h \
	document-undo.cpp \
	uri-references.cpp uri-references.h \
	version.cpp version.h

noinst_LTLIBRARIES = libinkscape.la
libinkscape_la_SOURCES = $(inkscapelib_src)
spproglibs = libinkscape.la

inkscape_SOURCES = \
	$(spprogsrc) \
	\
	inkscape.cpp inkscape.h inkscape-private.h \
	\
	inkscape-stock.cpp inkscape-stock.h\
	\
	sp-namedview.cpp sp-namedview.h \
	sp-guide.cpp sp-guide.h \
	\
	geom.cpp geom.h \
	view.cpp view.h \
	svg-view.cpp svg-view.h \
	selection.cpp selection.h \
	desktop.cpp desktop.h \
	desktop-units.h \
	desktop-events.cpp desktop-events.h \
	desktop-handles.cpp desktop-handles.h \
	desktop-affine.cpp desktop-affine.h \
	desktop-snap.cpp desktop-snap.h \
	sp-metrics.cpp sp-metrics.h \
	toolbox.cpp toolbox.h \
	\
	verbs.cpp verbs.h \
	shortcuts.cpp shortcuts.h \
	event-context.cpp event-context.h \
	seltrans-handles.cpp seltrans-handles.h \
	seltrans.cpp seltrans.h \
	select-context.cpp select-context.h \
	node-context.cpp node-context.h \
	nodepath.cpp nodepath.h \
	knotholder.cpp knotholder.h \
	object-edit.cpp object-edit.h \
	rect-context.cpp rect-context.h \
	arc-context.cpp arc-context.h \
	star-context.cpp star-context.h \
	spiral-context.cpp spiral-context.h \
	draw-context.cpp draw-context.h \
	dyna-draw-context.cpp dyna-draw-context.h \
	text-context.cpp text-context.h \
	zoom-context.cpp zoom-context.h \
	dropper-context.cpp dropper-context.h \
	tools-switch.cpp tools-switch.h\
	prefs-utils.cpp prefs-utils.h\
	\
	knot.cpp knot.h \
	\
	main.cpp \
	help.cpp help.h \
	sp-cursor.cpp sp-cursor.h \
	dir-util.cpp dir-util.h \
	rubberband.cpp rubberband.h \
	path-chemistry.cpp path-chemistry.h \
	selection-chemistry.cpp selection-chemistry.h \
	gradient-chemistry.cpp gradient-chemistry.h \
	sp-item-transform.cpp sp-item-transform.h \
	\
	splivarot.cpp splivarot.h \
	\
	file.cpp file.h \
	print.cpp print.h \
	interface.cpp interface.h \
	object-ui.cpp object-ui.h \
	slideshow.cpp slideshow.h \
	$(module_src) \
	\
	preferences-skeleton.h

inkscape_private_libs = \
	$(spproglibs) \
	dialogs/libspdialogs.a \
	xml/libspxml.a \
	svg/libspsvg.a \
	widgets/libspwidgets.a \
	display/libspdisplay.a \
	helper/libspchelp.a \
	libnrtype/libnrtype.a \
	libnr/libnr.a \
	libarikkei/libarikkei.a \
	livarot/libvarot.a \
	$(module_libs)


inkscape_LDADD = \
	$(inkscape_private_libs) \
	$(INKSCAPE_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(XFT_LIBS) \
	$(FREETYPE_LIBS) \
	$(kdeldadd)

inkscape_LDFLAGS = --export-dynamic $(kdeldflags)

EXTRA_PROGRAMS = inkview

inkview_SOURCES = \
	inkview.cpp \
	view.cpp view.h \
	svg-view.cpp svg-view.h \
	dir-util.cpp dir-util.h \
	modules/ps.cpp modules/ps.h \
	module.cpp module.h \
	print.cpp print.h \
	prefs-utils.cpp prefs-utils.h

inkview_LDADD = \
	$(spproglibs) \
	$(module_libs) \
	xml/libspxml.a \
	svg/libspsvg.a \
	display/libspdisplay.a \
	helper/libspchelp.a \
	libnrtype/libnrtype.a \
	libnr/libnr.a \
	libarikkei/libarikkei.a \
	livarot/libvarot.a \
	$(INKSCAPE_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(XFT_LIBS) \
	$(FREETYPE_LIBS) \
	$(kdeldadd)

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps
