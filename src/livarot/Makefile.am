noinst_LIBRARIES = libvarot.a

INCLUDES = \
	-I.. \
	-I$(top_srcdir)/src \
	$(INKSCAPE_CFLAGS)

TESTS = Path-test$(EXEEXT)
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS = Path-test

libvarot_a_SOURCES = \
	AVL.cpp AVL.h \
	DblLinked.cpp DblLinked.h \
	LivarotDefs.h	\
	MyMath.h	\
	Path.cpp Path.h \
	PathConversion.cpp Path.h \
	PathOutline.cpp Path.h \
	PathStroke.cpp Path.h \
	PathSimplify.cpp Path.h \
	ShapeUtils.h	\
	Shape.cpp Shape.h \
	ShapeMisc.cpp Shape.h \
	ShapeDraw.cpp Shape.h \
	ShapeSweep.cpp Shape.h \
	ShapeSweepUtils.cpp Shape.h \
	MySeg.cpp MySeg.h

Path_test_SOURCES = Path-test.cpp AVL.cpp DblLinked.cpp PathConversion.cpp PathOutline.cpp Shape.cpp ShapeSweepUtils.cpp
Path_test_LDADD = ../libnr/libnr.a -lglib-2.0
