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
	PathCutting.cpp Path.h \
	PathOutline.cpp Path.h \
	PathStroke.cpp Path.h \
	PathSimplify.cpp Path.h \
	ShapeUtils.h	\
	Shape.cpp Shape.h \
	ShapeMisc.cpp Shape.h \
	ShapeDraw.cpp Shape.h \
	ShapeSweep.cpp Shape.h \
	ShapeSweepUtils.cpp Shape.h \
	MySeg.cpp MySeg.h \
  Ligne.cpp Ligne.h \
  AlphaLigne.cpp AlphaLigne.h \
  BitLigne.cpp BitLigne.h \
  ShapeRaster.cpp Shape.h

Path_test_SOURCES = Path-test.cpp
Path_test_LDADD = libvarot.a ../libnr/libnr.a -lglib-2.0
