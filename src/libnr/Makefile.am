noinst_LIBRARIES = libnr.a

noinst_PROGRAMS = gen_nr_config testnr

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(cflags) \
	$(SODIPODI_CFLAGS)

if USE_MMX
mmx_sources = \
	have_mmx.S \
	nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP.S \
	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP.S \
	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM.S \
	nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P.S
endif

BUILT_SOURCES = nr_config.h

nr_config.h: gen_nr_config
	./gen_nr_config > nr_config.h

libnr_a_SOURCES = \
	nr-types.h \
	nr-forward.h \
	nr-macros.h \
	nr-values.c nr-values.h \
	nr-matrix.c nr-matrix.h \
	nr-rect.c nr-rect.h \
	nr-pixops.h \
	nr-compose.c nr-compose.h \
	nr-compose-transform.c nr-compose-transform.h \
	nr-pixblock.c nr-pixblock.h \
	nr-pixblock-pixel.c nr-pixblock-pixel.h \
	nr-pixblock-line.c nr-pixblock-line.h \
	nr-pixblock-pattern.c nr-pixblock-pattern.h \
	nr-blit.c nr-blit.h \
	nr-render.h \
	nr-gradient.c nr-gradient.h \
	nr-path.c nr-path.h \
	nr-svp.c nr-svp.h nr-svp-private.h \
	nr-svp-uncross.c nr-svp-uncross.h \
	nr-svp-render.c nr-svp-render.h \
	nr-stroke.c nr-stroke.h \
	nr-uta.c nr-uta.h \
	nr-object.c nr-object.h \
	$(mmx_sources)

testnr_SOURCES = \
	testnr.c

testnr_LDADD = \
	libnr.a
