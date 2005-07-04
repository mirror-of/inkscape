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
	fix-roff-punct	\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	mkinstalldirs		\
	$(Graphics_in_files)	\
	po/check-markup \
	inkscape.1		\
	inkscape.fr.1		\
	inkscape.pod		\
	inkscape.fr.pod		\
	inkscape.spec.in	\
	inkscape.spec		\
	Info.plist.in	\
	Info.plist		\
	inkscape.nsi		\
	Makefile.mingw	\
	Makefile.mingw.common	\
	README HACKING		\
	inkscape.png \
	inkscape.ico inkscape16.ico inkscape32.ico inkscape64.ico \
	cxxtest/cxxtestgen.pl \
	cxxtest/cxxtestgen.py \
	cxxtest/cxxtest/Descriptions.cpp \
	cxxtest/cxxtest/Descriptions.h \
	cxxtest/cxxtest/DummyDescriptions.cpp \
	cxxtest/cxxtest/DummyDescriptions.h \
	cxxtest/cxxtest/ErrorFormatter.h \
	cxxtest/cxxtest/ErrorPrinter.h \
	cxxtest/cxxtest/Flags.h \
	cxxtest/cxxtest/GlobalFixture.cpp \
	cxxtest/cxxtest/GlobalFixture.h \
	cxxtest/cxxtest/Gui.h \
	cxxtest/cxxtest/LinkedList.cpp \
	cxxtest/cxxtest/LinkedList.h \
	cxxtest/cxxtest/Mock.h \
	cxxtest/cxxtest/ParenPrinter.h \
	cxxtest/cxxtest/QtGui.h \
	cxxtest/cxxtest/RealDescriptions.cpp \
	cxxtest/cxxtest/RealDescriptions.h \
	cxxtest/cxxtest/Root.cpp \
	cxxtest/cxxtest/SelfTest.h \
	cxxtest/cxxtest/StdHeaders.h \
	cxxtest/cxxtest/StdValueTraits.h \
	cxxtest/cxxtest/StdioFilePrinter.h \
	cxxtest/cxxtest/StdioPrinter.h \
	cxxtest/cxxtest/TeeListener.h \
	cxxtest/cxxtest/TestListener.h \
	cxxtest/cxxtest/TestRunner.h \
	cxxtest/cxxtest/TestSuite.cpp \
	cxxtest/cxxtest/TestSuite.h \
	cxxtest/cxxtest/TestTracker.cpp \
	cxxtest/cxxtest/TestTracker.h \
	cxxtest/cxxtest/ValueTraits.cpp \
	cxxtest/cxxtest/ValueTraits.h \
	cxxtest/cxxtest/Win32Gui.h \
	cxxtest/cxxtest/X11Gui.h \
	cxxtest/cxxtest/YesNoRunner.h \
	debian/changelog \
	debian/compat	\
	debian/control \
	debian/copyright	\
	debian/dirs	\
	debian/docs	\
	debian/inkscape.applications	\
	debian/inkscape.menu	\
	debian/inkscape.xpm	\
	debian/mime	\
	debian/rules	\
	packaging/autopackage/default.apspec.in

# Note: it isn't clear that debian/* belongs in the tar file:
# tarfile users should be able to use the official Debian packages.
# They were added to EXTRA_DIST in v1.5, in response to a message on
# inkscape-devel from "Carlos Ingelmo <carlsalam@te...>" on 2003-11-12 01:35.

# We want the built inkscape.1 to go in the distributed .tar.gz so
# that .tar.gz users don't need a full perl distribution to get a man
# page.

DISTCLEANFILES = inkscape.desktop \
                 intltool-extract \
                 intltool-merge \
                 intltool-update

#ACLOCAL_AMFLAGS = -I m4

SUFFIXES = .pod .1
inkscape.1 inkscape.fr.1: fix-roff-punct Makefile.am AUTHORS utf8-to-roff
.pod.1:
	set -e; \
	d=`sed -n 's,/,-,g;s,.*\$$[D]ate: \(..........\).*,\1,p' $<`; \
	pod2man $< \
	  | sed 's/^\.TH .*/.TH INKSCAPE 1 "'"$$d"'" "Inkscape-$(VERSION)" "Inkscape"/' \
	  | perl -pe 'if (/^\[\% .*INCLUDE.*AUTHORS.* \%\]/) { open(FOO, "$(srcdir)/AUTHORS");$$_ = join(",\n", map { chomp; $$_ } <FOO>) . "\n"; }' \
	  | perl $(srcdir)/fix-roff-punct \
	  | perl -CI $(srcdir)/utf8-to-roff	\
	  > tmp.$$$$ \
	 && mv tmp.$$$$ $@
# man 7 groff_char for further remarks on charset encoding for man pages.

install-data-local: inkscape.fr.1
	$(mkinstalldirs) $(DESTDIR)$(mandir)/fr/man1
	$(INSTALL_DATA) `if [ -f inkscape.fr.1 ]; then :; else echo $(srcdir)/;fi`inkscape.fr.1 $(DESTDIR)$(mandir)/fr/man1/inkscape.1

.PHONY: warn_markup
noinst_DATA = warn_markup
warn_markup:
	@if perl $(srcdir)/po/check-markup $(srcdir)/po/*.po; [ $$? = 1 ]; then \
	  echo "WARNING: Bad markup found in translations.  Please consider fixing the above problems." >&2; \
	fi

.PHONY: check_markup
check_DATA = check_markup
check_markup:
	perl $(srcdir)/po/check-markup $(srcdir)/po/*.po || [ $$? = 127 ]
#	# `make check' probably shouldn't require perl to be installed, hence ignoring 127.

distcheck-hook:
	perl $(srcdir)/po/check-markup $(srcdir)/po/*.po

test-check-markup:
	if perl -e 42; then \
	  perl $(srcdir)/po/check-markup $(srcdir)/po/bad.po.test 2>&1	\
	    | diff $(srcdir)/po/bad.po.test.exp -;	\
	fi
