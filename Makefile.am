## Process this file with automake to produce Makefile.in

SUBDIRS = src doc share po

appicondir = $(datadir)/pixmaps
appicon_DATA = inkscape.png

Graphicsdir = $(datadir)/applications
Graphics_in_files = inkscape.desktop.in
Graphics_DATA = $(Graphics_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

## dist-hook:
##	mkdir $(distdir)/samples
##	cp $(srcdir)/samples/*svg $(distdir)/samples
##	cp $(srcdir)/samples/*png $(distdir)/samples
##	cp $(srcdir)/AUTHORS $(distdir)
##	cp $(srcdir)/NEWS $(distdir)


man_MANS = inkscape.1 \
		inkview.1

EXTRA_DIST = \
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	mkinstalldirs		\
	$(Graphics_in_files)	\
	inkscape.1		\
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

# We want the built inkscape.1 to go in the distributed .tar.gz so
# that .tar.gz users don't need a full perl distribution to get a man
# page.
# I'm not sure whether doing so requires placing inkscape.1 in
# $(srcdir).
inkscape.1: inkscape.pod fix-roff-punct Makefile.am
	set -e; \
	d=`sed -n 's,/,-,g;s,.*\$$Date: \(..........\).*,\1,p' $(srcdir)/inkscape.pod`; \
	pod2man $(srcdir)/inkscape.pod \
	  | sed 's/^\.TH .*/.TH INKSCAPE 1 "'"$$d"'" "Inkscape-$(VERSION)" "User Commands"/' \
	  | perl -pe 'if (/^\[\% .*INCLUDE.*AUTHORS.* \%\]/) { open(FOO, "$(srcdir)/AUTHORS");$$_ = join(",\n", map { chomp; $$_ } <FOO>) . "\n"; }' \
	  | perl $(srcdir)/fix-roff-punct \
	  > tmp.$$$$ \
	 && mv tmp.$$$$ inkscape.1
