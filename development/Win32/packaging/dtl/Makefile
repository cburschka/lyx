# Makefile for dv2dt, dt2dv
# Version 0.6.1
# Thu 9 March 1995
# Geoffrey Tobin
# Nelson H. F. Beebe
#
# Changes 27 July 2005 by Angus Leeming to enable the Makefile to
# work out of the box on both *nix and Windows machines under
# the MinSYS environment.
#
# The Makefile can also be used unaltered to build a Windows executable
# from a Linux box if make is invoked as:
# $ make EXEEXT='.exe' CC='i386-mingw32-gcc'
#=======================================================================

BINDIR          = /usr/local/bin
CATDIR		= $(MANDIR)/../cat$(MANEXT)
CC              = gcc
CFLAGS          = -O2 -Wall
# Some compilers don't optimise correctly; for those, don't use `-O2' :
# CFLAGS          = -Wall
CHMOD           = /bin/chmod
COL		= col -b
CP              = /bin/cp
DITROFF		= ditroff
DITROFF		= groff
EXEEXT		= .exe
DT2DV		= dt2dv$(EXEEXT)
DV2DT		= dv2dt$(EXEEXT)
EXES 		= $(DT2DV) $(DV2DT)
LDFLAGS         = -s
LDFLAGS         =
MAN2PS		= sh ./man2ps
MANDIR		= /usr/local/man/man$(MANEXT)
MANEXT		= 1
OBJS            = dt2dv.o dv2dt.o
RM              = /bin/rm -f
SHELL           = /bin/sh

DOCS		= README dtl.doc dvi.doc dt2dv.man dv2dt.man
SRC		= Makefile dtl.h dt2dv.c dv2dt.c man2ps
TESTS		= hello.tex example.tex tripvdu.tex edited.txt

DTL_DBN		= $(DOCS) $(SRC) $(TESTS)

#=======================================================================

.SUFFIXES:	.hlp .ps .man

.man.hlp:
	$(DITROFF) -man -Tascii $< | $(COL) >$@

.man.ps:
	$(MAN2PS) < $< > $@

#=======================================================================

#all:  dtl check doc
all:  dtl check

doc:  dt2dv.hlp dv2dt.hlp dt2dv.ps dv2dt.ps

dtl:  dv2dt dt2dv

check tests:  hello example tripvdu edited

dv2dt: dv2dt.o dtl.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(DV2DT) dv2dt.o

dt2dv: dt2dv.o dtl.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(DT2DV) dt2dv.o

hello:  hello.dtl dv2dt dt2dv
	./$(DT2DV) hello.dtl hello2.dvi
	./$(DV2DT) hello2.dvi hello2.dtl
	-@diff hello.dtl hello2.dtl > hello.dif
	@if [ -s hello.dif ] ; \
	then echo ERROR: differences in hello.dif ; \
	else $(RM) hello.dif ; \
	fi

hello.dtl:  hello.tex
	tex hello
	./$(DV2DT) hello.dvi hello.dtl

example:  example.dtl dv2dt dt2dv
	./$(DT2DV) example.dtl example2.dvi
	./$(DV2DT) example2.dvi example2.dtl
	-@diff example.dtl example2.dtl > example.dif
	@if [ -s example.dif ] ; \
	then echo ERROR: differences in example.dif ; \
	else $(RM) example.dif ; \
	fi

example.dtl:  example.tex
	tex example
	./$(DV2DT) example.dvi example.dtl

tripvdu:  tripvdu.dtl dv2dt dt2dv
	./$(DT2DV) tripvdu.dtl tripvdu2.dvi
	./$(DV2DT) tripvdu2.dvi tripvdu2.dtl
	-@diff tripvdu.dtl tripvdu2.dtl > tripvdu.dif
	@if [ -s tripvdu.dif ] ; \
	then echo ERROR: differences in tripvdu.dif ; \
	else $(RM) tripvdu.dif ; \
	fi

tripvdu.dtl:  tripvdu.tex
	tex tripvdu
	./$(DV2DT) tripvdu.dvi tripvdu.dtl

# edited.txt is already a dtl file.

edited:  edited.txt dv2dt dt2dv
	./$(DT2DV) edited.txt edited.dvi
	./$(DV2DT) edited.dvi edited2.dtl
	./$(DT2DV) edited2.dtl edited2.dvi
	./$(DV2DT) edited2.dvi edited3.dtl
	@if [ -s edited.dif ] ; \
	then echo ERROR : differences in edited.dif ; \
	else $(RM) edited.dif ; \
	fi

clean mostlyclean:
	-$(RM) $(OBJS)

clobber: clean
	-$(RM) $(EXES) *~ core *.log *.dvi *.dtl *.dif

distclean realclean: clobber
	-$(RM) dt2dv.hlp dv2dt.hlp dt2dv.ps dv2dt.ps

install:	dtl
	-$(MAKE) uninstall
	$(CP) dt2dv $(BINDIR)/dt2dv
	$(CHMOD) 775 $(BINDIR)/dt2dv
	$(CP) dv2dt $(BINDIR)/dv2dt
	$(CHMOD) 775 $(BINDIR)/dv2dt
	$(CP) dt2dv.man $(MANDIR)/dt2dv.$(MANEXT)
	$(CHMOD) 664 $(MANDIR)/dt2dv.$(MANEXT)
	$(CP) dv2dt.man $(MANDIR)/dv2dt.$(MANEXT)
	$(CHMOD) 664 $(MANDIR)/dv2dt.$(MANEXT)

uninstall:
	-$(RM) $(BINDIR)/dt2dv
	-$(RM) $(BINDIR)/dv2dt
	-$(RM) $(CATDIR)/dt2dv.$(MANEXT)
	-$(RM) $(CATDIR)/dv2dt.$(MANEXT)

dist:  dtl.tar.gz

dtl.tar.gz:  $(DTL_DBN)
	tar -czf dtl.tar.gz $(DTL_DBN)

zip:  dtl.zip

dtl.zip:  $(DTL_DBN)
	zip dtl.zip $(DTL_DBN)

# EOF Makefile
