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
	form_preferences.fd \
	form_print.fd \
	form_tabular.fd


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
	for hfil in *.h ; do \
		if [ -f "$$hfil.patch" ] ; then \
			(echo "Patching $$hfil with $$hfil.patch" ; \
			patch -s $$hfil < $$hfil.patch) \
		fi; \
		sed -f fdfixh.sed < $$hfil > $$hfil.tmp; \
		mv $$hfil.tmp $$hfil; \
	done

updatesrc: d e
	@echo >&2
	@echo "Everythings been generated now.">&2
	@echo "It's up to you to actually merge it into the respective">&2
	@echo "implementations in the parent directory.  Most of the work">&2
	@echo "simply involves cutting and pasting the FormXxxx::build()">&2
	@echo "code into that forms FormXxxx.C file.  You'll also want to">&2
	@echo "copy the FD_form... declaration from the header into the">&2
	@echo "classes header file.">&2
	@echo >&2

.fd.c:
	$(FDESIGN) -convert $<

.c.C:
	./fdfix.sh $< $@
	rm $<

clean:
	rm -f *.[hcC] *.bak

distclean: clean
	rm -f *.orig *.rej *~

install: clean updatesrc
	cp *.C *.h ..
