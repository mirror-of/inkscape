## Process this file with automake to produce Makefile.in


# ################################################
#
#  G L O B A L
#
# ################################################

# Should work in either automake1.7 or 1.8, but 1.6 doesn't
# handle foo/libfoo_a_CPPFLAGS properly (if at all).
AUTOMAKE_OPTIONS = 1.7 subdir-objects

INCLUDES =	\
	$(INKSCAPE_CFLAGS)

include Makefile_insert
include dialogs/Makefile_insert
include display/Makefile_insert
include extension/Makefile_insert
include extension/implementation/Makefile_insert
include extension/internal/Makefile_insert
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

bin_PROGRAMS = inkscape inkview

noinst_LIBRARIES =	\
	libinkpre.a	\
	dialogs/libspdialogs.a	\
	display/libspdisplay.a	\
	extension/implementation/libimplementation.a	\
	extension/internal/libinternal.a	\
	extension/libextension.a	\
	helper/libspchelp.a	\
	$(inkjar_libs)	\
	libnr/libnr.a	\
	libnrtype/libnrtype.a	\
	livarot/libvarot.a	\
	svg/libspsvg.a	\
	widgets/libspwidgets.a	\
	xml/libspxml.a	\
	libinkpost.a

DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape_version.h

EXTRA_DIST =	\
	dialogs/makefile.in	\
	display/makefile.in	\
	extension/implementation/makefile.in	\
	extension/internal/makefile.in	\
	extension/makefile.in	\
	helper/makefile.in	\
	inkjar/makefile.in	\
	libnr/makefile.in	\
	libnrtype/makefile.in	\
	livarot/makefile.in	\
	svg/makefile.in	\
	utest/makefile.in	\
	widgets/makefile.in	\
	xml/makefile.in	\
	extension/internal/gnome.cpp	\
	extension/internal/gnome.h	\
	extension/internal/win32.cpp	\
	extension/internal/win32.h	\
	helper/sp-marshal.list	\
	utest/utest.h \
	algorithms/longest-common-suffix.h \
	traits/list.h \
	traits/reference.h \
	traits/tree-iterator.h \
	util/parent-axis.h \
	util/sibling-axis.h \
	util/tuple.h \
	util/list-iterator.h

EXTRA_PROGRAMS =	\
	inkview 	\
	libnr/testnr

TESTS =	\
	extract-uri-test$(EXEEXT)	\
	mod360-test$(EXEEXT)	\
	display/bezier-utils-test$(EXEEXT)	\
	helper/units-test$(EXEEXT)	\
	libnr/nr-matrix-test$(EXEEXT)	\
	libnr/nr-point-fns-test$(EXEEXT)	\
	libnr/nr-rotate-test$(EXEEXT)	\
	libnr/nr-rotate-fns-test$(EXEEXT)	\
	libnr/nr-scale-test$(EXEEXT)	\
	libnr/nr-translate-test$(EXEEXT)	\
	libnr/nr-types-test$(EXEEXT)	\
	livarot/Path-test$(EXEEXT)	\
	xml/repr-action-test$(EXEEXT)

# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS =	\
	extract-uri-test	\
	mod360-test	\
	display/bezier-utils-test	\
	helper/units-test	\
	libnr/nr-matrix-test	\
	libnr/nr-point-fns-test	\
	libnr/nr-rotate-test	\
	libnr/nr-rotate-fns-test	\
	libnr/nr-scale-test	\
	libnr/nr-translate-test	\
	libnr/nr-types-test	\
	livarot/Path-test	\
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

