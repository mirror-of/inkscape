## Process this file with automake to produce Makefile.in

SUBDIRS = src glade po extensions

appicondir = $(datadir)/pixmaps
appicon_DATA = inkscape.png

Graphicsdir = $(datadir)/applications
Graphics_in_files = inkscape.desktop.in
Graphics_DATA = $(Graphics_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

dist-hook:
	mkdir $(distdir)/samples
	cp $(srcdir)/samples/*svg $(distdir)/samples
	cp $(srcdir)/samples/*png $(distdir)/samples


man_MANS = inkscape.1

EXTRA_DIST = \
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	$(Graphics_in_files)	\
	inkscape.spec.in	\
	inkscape.spec		\
	README HACKING		\
	inkscape.png \
	inkscape.ico inkscape16.ico inkscape32.ico inkscape64.ico \
	debian/changelog \
	debian/copyright \
	debian/control \
	debian/rules \
	debian/inkscape.menu

DISTCLEANFILES = inkscape.desktop \
                 intltool-extract \
                 intltool-merge \
                 intltool-update

#ACLOCAL_AMFLAGS = -I m4
