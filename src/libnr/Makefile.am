noinst_LIBRARIES = libnr.a

EXTRA_PROGRAMS = testnr

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(cflags) \
	$(INKSCAPE_CFLAGS)

TESTS = nr-types-test$(EXEEXT)	\
	nr-point-fns-test$(EXEEXT)	\
	nr-matrix-test$(EXEEXT)	\
	nr-rotate-test$(EXEEXT)	\
	nr-scale-test$(EXEEXT)	\
	nr-translate-test$(EXEEXT)
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS = nr-types-test	\
	nr-point-fns-test	\
	nr-matrix-test	\
	nr-rotate-test	\
	nr-scale-test	\
	nr-translate-test

if USE_MMX
mmx_sources = \
	have_mmx.S \
	nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP.S \
	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP.S \
	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM.S \
	nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P.S
endif

BUILT_SOURCES =

DISTCLEANFILES =

libnr_a_SOURCES = \
	nr-types.cpp nr-types.h	\
	nr-forward.h \
	nr-macros.h \
	nr-values.cpp nr-values.h	\
	nr-coord.h	\
	nr-dim2.h	\
	nr-i-coord.h	\
	nr-point-l.h	\
	nr-point.h nr-point-ops.h	\
	nr-point-fns.cpp nr-point-fns.h	\
	nr-rect-l.h nr-rect-l.cpp	\
	nr-rotate.h nr-rotate-ops.h	\
	nr-scale.h nr-scale-ops.h	\
	nr-translate-ops.h nr-translate.h	\
	nr-matrix.cpp nr-matrix.h nr-matrix-ops.h	\
	nr-matrix-fns.cpp nr-matrix-fns.h \
	nr-rect.cpp nr-rect.h \
	nr-pixops.h \
	nr-compose.cpp nr-compose.h \
	nr-compose-transform.cpp nr-compose-transform.h \
	nr-pixblock.cpp nr-pixblock.h \
	nr-pixblock-pixel.cpp nr-pixblock-pixel.h \
	nr-pixblock-line.cpp nr-pixblock-line.h \
	nr-pixblock-pattern.cpp nr-pixblock-pattern.h \
	nr-blit.cpp nr-blit.h \
	nr-render.h \
	nr-gradient.cpp nr-gradient.h \
	nr-path.cpp nr-path.h \
	nr-svp.cpp nr-svp.h nr-svp-private.h \
	nr-svp-render.cpp nr-svp-render.h \
	nr-uta.cpp nr-uta.h \
	nr-object.cpp nr-object.h \
	nr-maybe.h \
	$(mmx_sources)

testnr_SOURCES = \
	testnr.cpp

testnr_LDADD =	\
	libnr.a	\
	-lglib-2.0

nr_types_test_SOURCES = nr-types-test.cpp
nr_types_test_LDADD = -lglib-2.0 libnr.a

nr_point_fns_test_SOURCES = nr-point-fns-test.cpp
nr_point_fns_test_LDADD = -lglib-2.0 libnr.a

nr_matrix_test_SOURCES = nr-matrix-test.cpp
nr_matrix_test_LDADD = libnr.a -lglib-2.0

nr_rotate_test_SOURCES = nr-rotate-test.cpp
nr_rotate_test_LDADD = libnr.a -lglib-2.0

nr_scale_test_SOURCES = nr-scale-test.cpp
nr_scale_test_LDADD = libnr.a -lglib-2.0

nr_translate_test_SOURCES = nr-translate-test.cpp
nr_translate_test_LDADD = libnr.a -lglib-2.0
