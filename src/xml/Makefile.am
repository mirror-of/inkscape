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

TESTS = repr-action-test$(EXEEXT)
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS = repr-action-test

noinst_LIBRARIES = libspxml.a

libspxml_a_SOURCES = \
	repr.cpp repr.h \
	repr-private.h \
	repr-util.cpp \
	repr-io.cpp \
	repr-css.cpp \
	repr-action.cpp repr-action.h

repr_action_test_SOURCES = repr-action-test.cpp repr-action.cpp repr.cpp repr-util.cpp

repr_action_test_LDADD = $(INKSCAPE_LIBS)
