## Process this file with automake to produce Makefile.in


# ################################################
#
#  G L O B A L
#
# ################################################

SUBDIRS = libarikkei libnr livarot libnrtype xml svg helper widgets dialogs display extension utest $(inkjar_dir)

AM_CPPFLAGS = \
	-DINKSCAPE_VERSION=\""$(VERSION)"\"

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DINKSCAPE_PIXMAPDIR=\""$(datadir)/inkscape/icons"\" \
	-DINKSCAPE_SCREENSDIR=\""$(datadir)/inkscape/screens"\" \
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

# ################################################
#
#  E X T R A 
#
# ################################################

if PLATFORM_WIN32
win32_sources = winmain.cpp
win32ldflags = -lcomdlg32
endif 

if INKJAR
inkjar_dir = inkjar
inkjar_libs = inkjar/libinkjar.a
endif


# ################################################
#
#  I N K S C A P E
#
# ################################################

bin_PROGRAMS = inkscape 

noinst_LIBRARIES = libinkscape.a

libinkscape_a_SOURCES = \
	approx-equal.h remove-last.h \
	arc-context.cpp arc-context.h \
	attributes.cpp attributes.h \
	brokenimage.xpm \
	color.cpp color.h \
	desktop-affine.cpp desktop-affine.h \
	desktop.cpp desktop.h \
	desktop-events.cpp desktop-events.h \
	desktop-handles.cpp desktop-handles.h \
	desktop-snap.cpp desktop-snap.h \
	desktop-units.h \
	dir-util.cpp dir-util.h \
	document.cpp document.h document-private.h \
	document-undo.cpp \
	draw-context.cpp draw-context.h \
	dropper-context.cpp dropper-context.h \
	dyna-draw-context.cpp dyna-draw-context.h \
	enums.h \
	event-context.cpp event-context.h \
	extract-uri.cpp extract-uri.h \
	file.cpp file.h \
	forward.h \
	geom.cpp geom.h \
	gradient-chemistry.cpp gradient-chemistry.h \
	help.cpp help.h \
	inkscape.cpp inkscape.h inkscape-private.h \
	inkscape-stock.cpp inkscape-stock.h\
	interface.cpp interface.h \
	knot.cpp knot.h \
	knotholder.cpp knotholder.h \
	macros.h \
	marker-status.cpp marker-status.h	\
	node-context.cpp node-context.h \
	nodepath.cpp nodepath.h \
	object-edit.cpp object-edit.h \
	object-ui.cpp object-ui.h \
	path-chemistry.cpp path-chemistry.h \
	preferences-skeleton.h \
	prefs-utils.cpp prefs-utils.h\
	print.cpp print.h \
	rect-context.cpp rect-context.h \
	rubberband.cpp rubberband.h \
	satisfied-guide-cns.cpp satisfied-guide-cns.h	\
	select-context.cpp select-context.h \
	selection-chemistry.cpp selection-chemistry.h \
	selection.cpp selection.h \
	select-toolbar.cpp select-toolbar.h\
	seltrans.cpp seltrans.h \
	seltrans-handles.cpp seltrans-handles.h \
	shortcuts.cpp shortcuts.h \
	slideshow.cpp slideshow.h \
	sp-anchor.cpp sp-anchor.h \
	sp-animation.cpp sp-animation.h \
	sp-chars.cpp sp-chars.h \
	sp-clippath.cpp sp-clippath.h \
	sp-cursor.cpp sp-cursor.h \
	sp-defs.cpp sp-defs.h \
	sp-ellipse.cpp sp-ellipse.h \
	sp-gradient.cpp sp-gradient.h \
	sp-guide-attachment.h	\
	sp-guide-constraint.h	\
	sp-guide.cpp sp-guide.h \
	sp-image.cpp sp-image.h \
	spiral-context.cpp spiral-context.h \
	sp-item.cpp sp-item.h \
	sp-item-group.cpp sp-item-group.h \
	sp-item-notify-moveto.cpp sp-item-notify-moveto.h \
	sp-item-rm-unsatisfied-cns.cpp sp-item-rm-unsatisfied-cns.h	\
	sp-item-transform.cpp sp-item-transform.h \
	sp-item-update-cns.cpp sp-item-update-cns.h	\
	sp-line.cpp sp-line.h \
	splivarot.cpp splivarot.h \
	sp-marker.cpp sp-marker.h \
	sp-mask.cpp sp-mask.h \
	sp-metrics.cpp sp-metrics.h \
	sp-namedview.cpp sp-namedview.h \
	sp-object.cpp sp-object.h \
	sp-object-group.cpp sp-object-group.h \
	sp-object-repr.cpp sp-object-repr.h \
	sp-offset.cpp sp-offset.h \
	sp-paint-server.cpp sp-paint-server.h \
	sp-path.cpp sp-path.h \
	sp-pattern.cpp sp-pattern.h \
	sp-polygon.cpp sp-polygon.h \
	sp-polyline.cpp sp-polyline.h \
	sp-rect.cpp sp-rect.h \
	sp-root.cpp sp-root.h \
	sp-shape.cpp sp-shape.h \
	sp-spiral.cpp sp-spiral.h \
	sp-star.cpp sp-star.h \
	sp-symbol.cpp sp-symbol.h \
	sp-text.cpp sp-text.h \
	sp-use.cpp sp-use.h \
	star-context.cpp star-context.h \
	streams-gzip.h streams-gzip.cpp \
	streams-handles.h streams-handles.cpp \
	streams-zlib.h streams-zlib.cpp \
	style.cpp style.h \
	svg-view.cpp svg-view.h \
	text-context.cpp text-context.h \
	toolbox.cpp toolbox.h \
	tools-switch.cpp tools-switch.h\
	uri.cpp uri.h \
	uri-references.cpp uri-references.h \
	verbs.cpp verbs.h \
	version.cpp version.h \
	view.cpp view.h \
	$(win32_sources) \
	zoom-context.cpp zoom-context.h

inkscape_private_libs = \
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
	extension/libextension.a \
	extension/implementation/libimplementation.a \
	extension/internal/libinternal.a \
	$(inkjar_libs)

all_libs = \
	$(inkscape_private_libs) \
	$(INKSCAPE_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(XFT_LIBS) \
	$(FREETYPE_LIBS) \
	$(kdeldadd) \
	$(win32ldflags)


inkscape_SOURCES = \
	$(libinkscape_a_SOURCES) = \
	main.cpp

inkscape_LDADD = \
	$(all_libs)

inkscape_LDFLAGS = --export-dynamic $(kdeldflags)



# ################################################
#
#  I N K V I E W
#
# ################################################


EXTRA_PROGRAMS = inkview


inkview_SOURCES = \
	$(libinkscape_a_SOURCES) = \
	inkview.cpp


inkview_LDADD = \
	$(all_libs)
	$(inkjar_libs)



# ################################################
#
#  D I S T
#
# ################################################

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps



# ################################################
#
#  T E S T S
#
# ################################################

TESTS = extract-uri-test$(EXEEXT)
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS = extract-uri-test

extract_uri_test_SOURCES = extract-uri-test.cpp
extract_uri_test_LDADD = libinkscape.a -lglib-2.0
