#
# Sodipodi xml wrapper
# Author: Lauris Kaplinski <lauris@ariman.ee>
#
# Implements xml backbone of sodipodi document. In future will be
# repaced with (wrapper of?) DOM
#

INCLUDES = \
	$(SODIPODI_CFLAGS) \
	-I$(includedir)

noinst_LIBRARIES = libspxml.a

libspxml_a_SOURCES = \
	repr.c repr.h \
	repr-private.h \
	repr-util.c \
	repr-io.c \
	repr-css.c \
	repr-action.c repr-action.h

