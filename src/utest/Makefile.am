noinst_LIBRARIES = libutest.a
# Making this a library was the first way I could think of of getting
# automake to build utest.o before any test programs are built, which
# in turn is done to avoid having the .TPo file go in non-existent
# src/xml/.deps/../utest directory.  Feel free to de-librarify, I
# haven't tried very hard to get it working without using a library.
# (pjrm)

libutest_a_SOURCES = utest.c
