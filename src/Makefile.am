## Process this file with automake to produce Makefile.in

module_libs = modules/libspmodules.a
module_src = module.c module.h

if USE_KDE
kdeldadd = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDEPRINT) $(LIB_QT)
kdeldflags = $(KDE_RPATH)
endif

SUBDIRS = libarikkei libnr libnrtype xml svg helper widgets dialogs display modules

AM_CPPFLAGS = \
	-DSODIPODI_VERSION=\""$(VERSION)"\"

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DSODIPODI_PIXMAPDIR=\""$(datadir)/sodipodi"\" \
	-DDATADIR=\""$(datadir)"\" \
	-I$(srcdir) \
	-I$(srcdir/xml) \
	-I$(srcdir/svg) \
	$(SODIPODI_CFLAGS) \
	$(GNOME_PRINT_CFLAGS)

bin_PROGRAMS = sodipodi

sodipodilib_src = \
	macros.h \
	forward.h \
	enums.h \
	attributes.c attributes.h \
	sp-object.c sp-object.h \
	sp-object-repr.c sp-object-repr.h \
	sp-object-group.c sp-object-group.h \
	sp-defs.c sp-defs.h \
	sp-item.c sp-item.h \
	sp-item-group.c sp-item-group.h \
	sp-symbol.c sp-symbol.h \
	sp-marker.c sp-marker.h \
	sp-use.c sp-use.h \
	sp-anchor.c sp-anchor.h \
	sp-root.c sp-root.h \
	sp-image.c sp-image.h \
	sp-path.c sp-path.h \
	sp-shape.c sp-shape.h \
	sp-rect.c sp-rect.h \
	sp-ellipse.c sp-ellipse.h \
	sp-star.c sp-star.h \
	sp-spiral.c sp-spiral.h \
	sp-line.c sp-line.h \
	sp-polyline.c sp-polyline.h \
	sp-polygon.c sp-polygon.h \
	sp-chars.c sp-chars.h \
	sp-text.c sp-text.h \
	sp-paint-server.c sp-paint-server.h \
	sp-gradient.c sp-gradient.h \
	sp-pattern.c sp-pattern.h \
	sp-clippath.c sp-clippath.h \
	sp-mask.c sp-mask.h \
	sp-animation.c sp-animation.h \
	\
	color.c color.h \
	style.c style.h \
	\
	document.c document.h document-private.h \
	document-undo.c \
	uri-references.c uri-references.h \
	extension.h

noinst_LTLIBRARIES = libsodipodi.la
libsodipodi_la_SOURCES = $(sodipodilib_src)
spproglibs = libsodipodi.la

sodipodi_SOURCES = \
	$(spprogsrc) \
	\
	sodipodi.c sodipodi.h sodipodi-private.h \
	\
	sp-namedview.c sp-namedview.h \
	sp-guide.c sp-guide.h \
	\
	view.c view.h \
	svg-view.c svg-view.h \
	selection.c selection.h \
	desktop.c desktop.h \
	desktop-units.h \
	desktop-events.c desktop-events.h \
	desktop-handles.c desktop-handles.h \
	desktop-affine.c desktop-affine.h \
	desktop-snap.c desktop-snap.h \
	sp-metrics.c sp-metrics.h \
	toolbox.c toolbox.h \
	\
	verbs.c verbs.h \
	shortcuts.c shortcuts.h \
	event-context.c event-context.h \
	seltrans-handles.c seltrans-handles.h \
	seltrans.c seltrans.h \
	select-context.c select-context.h \
	node-context.c node-context.h \
	nodepath.c nodepath.h \
	knotholder.c knotholder.h \
	object-edit.c object-edit.h \
	rect-context.c rect-context.h \
	arc-context.c arc-context.h \
	star-context.c star-context.h \
	spiral-context.c spiral-context.h \
	draw-context.c draw-context.h \
	dyna-draw-context.c dyna-draw-context.h \
	text-context.c text-context.h \
	zoom-context.c zoom-context.h \
	dropper-context.c dropper-context.h \
	\
	knot.c knot.h \
	\
	main.c \
	help.c help.h \
	sp-cursor.c sp-cursor.h \
	dir-util.c dir-util.h \
	rubberband.c rubberband.h \
	event-broker.c event-broker.h \
	path-chemistry.c path-chemistry.h \
	selection-chemistry.c selection-chemistry.h \
	gradient-chemistry.c gradient-chemistry.h \
	sp-item-transform.c sp-item-transform.h \
	\
	file.c file.h \
	print.c print.h \
	interface.c interface.h \
	object-ui.c object-ui.h \
	slideshow.c slideshow.h \
	$(module_src) \
	$(extensions_src) \
	\
	preferences-skeleton.h \
	extensions-skeleton.h

sodipodi_private_libs = \
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
	$(module_libs)


sodipodi_LDADD = \
	$(sodipodi_private_libs) \
	$(SODIPODI_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(XFT_LIBS) \
	$(FREETYPE_LIBS) \
	$(kdeldadd)

sodipodi_LDFLAGS = -Wl,--export-dynamic $(kdeldflags)

noinst_PROGRAMS = spsvgview

spsvgview_SOURCES = \
	spsvgview.c \
	view.c view.h \
	svg-view.c svg-view.h \
	dir-util.c dir-util.h \
	modules/ps.c modules/ps.h \
	module.c module.h \
	print.c print.h

spsvgview_LDADD = \
	$(spproglibs) \
	$(module_libs) \
	xml/libspxml.a \
	svg/libspsvg.a \
	display/libspdisplay.a \
	helper/libspchelp.a \
	libnrtype/libnrtype.a \
	libnr/libnr.a \
	libarikkei/libarikkei.a \
	$(SODIPODI_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(XFT_LIBS) \
	$(FREETYPE_LIBS) \
	$(kdeldadd)

# widgets/libspwidgets.a
# dialogs/libspdialogs.a

EXTRA_DIST = \
	brokenimage.xpm

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps
