# ###########################################
#  Temporary makefile
#  Just for building/testing.
#  Delete me when integrating with the rest
#  of Inkscape's build structure.
# ###########################################

CC=gcc
CXX=g++

CFLAGS = -O -g -Wall -DXP_UNIX 

INC = -I. -I.. -I../extension/script/js \
`pkg-config --cflags sigc++-1.2` \
`pkg-config --cflags gtkmm-2.0` \
`pkg-config --cflags libxml-2.0`

LIBS = \
../libinkpre.a	\
../dialogs/libspdialogs.a	\
../xml/libspxml.a	\
../svg/libspsvg.a	\
../widgets/libspwidgets.a	\
../display/libspdisplay.a	\
../helper/libspchelp.a	\
../libnrtype/libnrtype.a	\
../libnr/libnr.a	\
../livarot/libvarot.a	\
../extension/libextension.a	\
../extension/implementation/libimplementation.a	\
../extension/internal/libinternal.a	\
../extension/script/js/libjs.a \
../inkjar/libinkjar.a	\
../libinkpost.a \
`pkg-config --libs sigc++-1.2` \
`pkg-config --libs gtkmm-2.0` \
`pkg-config --libs libxml-2.0` \
-lpng -lstdc++

OBJ = EcmaBinding.o SVGViewer.o

all: ecmatest svgview

ecmatest: ecmatest.o $(OBJ)
	$(CXX) -o ecmatest $(OBJ) ecmatest.o $(LIBS)

svgview: svgview.o $(OBJ)
	$(CXX) -o svgview $(OBJ) svgview.o $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<


clean:
	$(RM) ecmatest svgview *.o
