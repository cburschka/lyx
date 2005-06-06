# Makefile for dv2dt, dt2dv
# Version 0.6.1
# Thu 9 March 1995
# Geoffrey Tobin
# Nelson H. F. Beebe
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
DT2DV		= dt2dv.exe
DV2DT		= dv2dt.exe
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

dtl:  $(EXES)

check tests:  hello example tripvdu edited

$(DV2DT): dv2dt.o dtl.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ dv2dt.o

$(DT2DV): dt2dv.o dtl.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ dt2dv.o

hello:  hello.dtl $(EXES)
	./dt2dv hello.dtl hello2.dvi
	./dv2dt hello2.dvi hello2.dtl
	-@diff hello.dtl hello2.dtl > hello.dif
	@if [ -s hello.dif ] ; \
	then echo ERROR: differences in hello.dif ; \
	else $(RM) hello.dif ; \
	fi

hello.dtl:  hello.tex
	tex hello
	./dv2dt hello.dvi hello.dtl

example:  example.dtl $(EXES)
	./dt2dv example.dtl example2.dvi
	./dv2dt example2.dvi example2.dtl
	-@diff example.dtl example2.dtl > example.dif
	@if [ -s example.dif ] ; \
	then echo ERROR: differences in example.dif ; \
	else $(RM) example.dif ; \
	fi

example.dtl:  example.tex
	tex example
	./dv2dt example.dvi example.dtl

tripvdu:  tripvdu.dtl $(EXES)
	./dt2dv tripvdu.dtl tripvdu2.dvi
	./dv2dt tripvdu2.dvi tripvdu2.dtl
	-@diff tripvdu.dtl tripvdu2.dtl > tripvdu.dif
	@if [ -s tripvdu.dif ] ; \
	then echo ERROR: differences in tripvdu.dif ; \
	else $(RM) tripvdu.dif ; \
	fi

tripvdu.dtl:  tripvdu.tex
	tex tripvdu
	./dv2dt tripvdu.dvi tripvdu.dtl

# edited.txt is already a dtl file.

edited:  edited.txt $(EXES)
	./dt2dv edited.txt edited.dvi
	./dv2dt edited.dvi edited2.dtl
	./dt2dv edited2.dtl edited2.dvi
	./dv2dt edited2.dvi edited3.dtl
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
