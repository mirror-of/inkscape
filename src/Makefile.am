## Process this file with automake to produce Makefile.in


# ################################################
#
#  G L O B A L
#
# ################################################

include Makefile_insert
include dialogs/Makefile_insert
include display/Makefile_insert
include extension/Makefile_insert
include extension/implementation/Makefile_insert
include extension/internal/Makefile_insert
include helper/Makefile_insert
include inkjar/Makefile_insert
include libarikkei/Makefile_insert
include libnr/Makefile_insert
include libnrtype/Makefile_insert
include livarot/Makefile_insert
include svg/Makefile_insert
include utest/Makefile_insert
include widgets/Makefile_insert
include xml/Makefile_insert

noinst_LIBRARIES =	\
	dialogs/libspdialogs.a	\
	display/libspdisplay.a	\
	extension/implementation/libimplementation.a	\
	extension/internal/libinternal.a	\
	extension/libextension.a	\
	helper/libspchelp.a	\
	$(inkjar_libs)	\
	libarikkei/libarikkei.a	\
	libnr/libnr.a	\
	libnrtype/libnrtype.a	\
	livarot/libvarot.a	\
	svg/libspsvg.a	\
	widgets/libspwidgets.a	\
	xml/libspxml.a	\
	libinkscape.a

DISTCLEANFILES =	\
	helper/sp-marshal.cpp helper/sp-marshal.h

EXTRA_DIST =	\
	extension/internal/gnome.cpp	\
	extension/internal/gnome.h	\
	extension/internal/win32.cpp	\
	extension/internal/win32.h	\
	helper/sp-marshal.list	\
	libnrtype/codepages.h	\
	libnrtype/nr-type-gnome.cpp	\
	libnrtype/nr-type-gnome.h	\
	libnrtype/nr-type-w32.cpp	\
	libnrtype/nr-type-w32.h	\
	libnrtype/nr-type-xft.cpp	\
	libnrtype/nr-type-xft.h	\
	utest/utest.h

EXTRA_PROGRAMS =	\
	inkview 	\
	libnr/testnr

TESTS =	\
	extract-uri-test$(EXEEXT)	\
	mod360-test$(EXEEXT)	\
	display/bezier-utils-test$(EXEEXT)	\
	libnr/nr-matrix-test$(EXEEXT)	\
	libnr/nr-point-fns-test$(EXEEXT)	\
	libnr/nr-rotate-test$(EXEEXT)	\
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
	libnr/nr-matrix-test	\
	libnr/nr-point-fns-test	\
	libnr/nr-rotate-test	\
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

