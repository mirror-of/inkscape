# ###########################################
#  Temporary makefile
#  Just for building/testing.
#  Delete me when integrating with the rest
#  of Inkscape's build structure.
# ###########################################

CC=gcc
CXX=g++

CFLAGS = -O -g -Wall -DXP_UNIX 

INC = -I. -I.. -I../extension/script/js `pkg-config --cflags gtk+-2.0`

LIBS = -L../extension/script/js -ljs `pkg-config --libs gtk+-2.0`


OBJ = EcmaBinding.o

all: ecmatest

ecmatest: ecmatest.o $(OBJ)
	$(CXX) -o ecmatest $(OBJ) ecmatest.o $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<


clean:
	$(RM) ecmatest *.o
