## Process this file with automake to produce Makefile.in


# ################################################
#
#  G L O B A L
#
# ################################################

# Should work in either automake1.7 or 1.8, but 1.6 doesn't
# handle foo/libfoo_a_CPPFLAGS properly (if at all).
# Update: We now avoid setting foo/libfoo_a_CPPFLAGS,
# so perhaps 1.6 will work.
AUTOMAKE_OPTIONS = 1.7 subdir-objects

INCLUDES =	\
	$(FREETYPE_CFLAGS)	\
	$(GNOME_PRINT_CFLAGS)	\
	$(XFT_CFLAGS)	\
	-DPOTRACE=\"potrace\"	\
	$(INKSCAPE_CFLAGS)

include Makefile_insert
include dialogs/Makefile_insert
include display/Makefile_insert
include extension/Makefile_insert
include extension/implementation/Makefile_insert
include extension/internal/Makefile_insert
include extension/plugin/Makefile_insert
include helper/Makefile_insert
include inkjar/Makefile_insert
include libnr/Makefile_insert
include libnrtype/Makefile_insert
include livarot/Makefile_insert
include svg/Makefile_insert
include utest/Makefile_insert
include widgets/Makefile_insert
include xml/Makefile_insert
include traits/Makefile_insert
include algorithms/Makefile_insert
include util/Makefile_insert
include trace/Makefile_insert

bin_PROGRAMS = inkscape inkview

noinst_LIBRARIES =	\
	libinkpre.a	\
	dialogs/libspdialogs.a	\
	display/libspdisplay.a	\
	extension/implementation/libimplementation.a	\
	extension/internal/libinternal.a	\
	extension/libextension.a	\
	helper/libspchelp.a	\
	util/libinkutil.a \
	$(inkjar_libs)	\
	libnr/libnr.a	\
	libnrtype/libnrtype.a	\
	livarot/libvarot.a	\
	svg/libspsvg.a	\
	widgets/libspwidgets.a	\
	trace/libtrace.a  \
	xml/libspxml.a	\
	libinkpost.a

DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape_version.h

EXTRA_DIST =	\
	Doxyfile	\
	algorithms/makefile.in	\
	dialogs/makefile.in	\
	display/makefile.in	\
	extension/makefile.in	\
	extension/implementation/makefile.in	\
	extension/internal/makefile.in	\
	helper/makefile.in	\
	inkjar/makefile.in	\
	libnr/makefile.in	\
	libnrtype/makefile.in	\
	livarot/makefile.in	\
	svg/makefile.in	\
	trace/makefile.in	\
	traits/makefile.in	\
	utest/makefile.in	\
	util/makefile.in	\
	widgets/makefile.in	\
	xml/makefile.in	\
	extension/internal/gnome.cpp	\
	extension/internal/gnome.h	\
	extension/internal/win32.cpp	\
	extension/internal/win32.h	\
	helper/sp-marshal.list	\
	utest/utest.h	\
	utest/test-1ary-cases.h	\
	algorithms/find-last-if.h \
	algorithms/longest-common-suffix.h \
	traits/copy.h	\
	traits/function.h	\
	traits/list-copy.h	\
	traits/reference.h

EXTRA_PROGRAMS =	\
	inkview 	\
	libnr/testnr

TESTS =	\
	extract-uri-test$(EXEEXT)	\
	mod360-test$(EXEEXT)	\
	round-test$(EXEEXT)	\
	sp-gradient-test$(EXEEXT)	\
	display/bezier-utils-test$(EXEEXT)	\
	helper/units-test$(EXEEXT)	\
	libnr/in-svg-plane-test$(EXEEXT)	\
	libnr/nr-matrix-test$(EXEEXT)	\
	libnr/nr-point-fns-test$(EXEEXT)	\
	libnr/nr-rotate-test$(EXEEXT)	\
	libnr/nr-rotate-fns-test$(EXEEXT)	\
	libnr/nr-scale-test$(EXEEXT)	\
	libnr/nr-translate-test$(EXEEXT)	\
	libnr/nr-types-test$(EXEEXT)	\
	xml/quote-test$(EXEEXT)	\
	xml/repr-action-test$(EXEEXT)

# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS =	\
	extract-uri-test	\
	mod360-test	\
	round-test	\
	sp-gradient-test	\
	display/bezier-utils-test	\
	helper/units-test	\
	libnr/in-svg-plane-test	\
	libnr/nr-matrix-test	\
	libnr/nr-point-fns-test	\
	libnr/nr-rotate-test	\
	libnr/nr-rotate-fns-test	\
	libnr/nr-scale-test	\
	libnr/nr-translate-test	\
	libnr/nr-types-test	\
	xml/quote-test	\
	xml/repr-action-test

# todo: consider consolidating some of the foo_CPPFLAGS into AM_CPPFLAGS / INCLUDES.


# ################################################
#
#  D I S T
#
# ################################################

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps

