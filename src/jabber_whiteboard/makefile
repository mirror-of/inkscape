# Convenience stub makefile to call the real Makefile.



# Explicit so that it's the default rule.
all:
	cd .. && $(MAKE) jabber_whiteboard/all

clean %.a %.o:
	cd .. && $(MAKE) jabber_whiteboard/$@

.PHONY: all clean

OBJEXT = o

.SUFFIXES:
.SUFFIXES: .a .$(OBJEXT)
