###/* This file is part of
###* ======================================================
###*
###*           LyX, the High Level Word Processor
###*
###*	       Copyright 1995 Matthias Ettrich
###*           Copyright 1995-2000 The LyX Team
###*
###*======================================================*/
###
###
### Makefile by Lars Gullik Bjønnes (larsbj@ifi.uio.no)
### Modified by Allan Rae <rae@lyx.org>
###

SHELL = /bin/sh
.SUFFIXES: .fd .c

# Various commands
FDESIGN = fdesign

SRCS := form_citation.fd \
	form_copyright.fd \
	form_document.fd \
	form_graphics.fd \
	form_index.fd \
	form_preferences.fd \
	form_print.fd \
	form_ref.fd \
	form_tabular.fd \
	form_toc.fd \
	form_url.fd


OBJS := $(SRCS:.fd=.c)

COBJS := $(SRCS:.fd=.C)

all:
	@echo "This is a makefile intended for the maintainer only."
	@echo "Only run it if you are absolutely sure about what you"
	@echo "are doing."

c: $(OBJS)

d: c $(COBJS)


# Modify the header files using the instructions in fdfixh.sed
e: c
	@echo "Modifying header files using the instructions in fdfixh.sed";\
	for hfil in *.h ; do \
		if [ -f "$$hfil.patch" ] ; then \
			(echo "Patching $$hfil with $$hfil.patch" ; \
			patch -s $$hfil < $$hfil.patch) \
		fi; \
		sed -f fdfixh.sed < $$hfil > $$hfil.tmp; \
		mv $$hfil.tmp $$hfil; \
	done

updatesrc: d e

.fd.c:
	$(FDESIGN) -convert $<

.c.C:
	${SHELL} ./fdfix.sh $< $@
	rm $<
	@if [ -f "$@.patch" ] ; then \
		(echo "Patching $@ with $@.patch" ; \
		patch -s $@ < $@.patch) \
	fi;

clean:
	rm -f *.[hcC] *.bak

distclean: clean
	rm -f *.orig *.rej *~

install: clean updatesrc
	cp *.C *.h ..
