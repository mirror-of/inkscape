CC = g++
CXX = g++

CFLAGS = -g -Wall -DYYDEBUG

INC =

LIBS =

OBJ = xpath.tab.o lex.yy.o

all: xtest

xtest: xtest.o $(OBJ)
	$(CXX) -o $@ xtest.o $(OBJ) $(LIBS)

lex.yy.c: xpath.lex xpath.ypp
	flex xpath.lex

xpath.tab.o: xpath.tab.cpp xpath.h
	$(CXX) $(CFLAGS) -c xpath.tab.cpp

xpath.tab.cpp: xpath.lex xpath.ypp
	bison -v -d xpath.ypp


clean:
	$(RM) *.o *.a lex.yy.c xpath.tab.cpp xpath.tab.hpp xtest
