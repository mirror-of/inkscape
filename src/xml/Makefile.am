#
# Sodipodi xml wrapper
# Author: Lauris Kaplinski <lauris@ariman.ee>
#
# Implements xml backbone of inkscape document. In future will be
# repaced with (wrapper of?) DOM
#

INCLUDES = \
	$(INKSCAPE_CFLAGS) \
	-I$(includedir)

TESTS = repr-action-test

check_PROGRAMS = repr-action-test

noinst_LIBRARIES = libspxml.a

libspxml_a_SOURCES = \
	repr.c repr.h \
	repr-private.h \
	repr-util.c \
	repr-io.c \
	repr-css.c \
	repr-action.c repr-action.h

repr_action_test_SOURCES = repr-action-test.c repr-action.c repr.c repr-util.c

repr_action_test_LDADD = ../utest/libutest.a $(INKSCAPE_LIBS)
