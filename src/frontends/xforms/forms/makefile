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
### Rewritten by Angus Leeming.

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

OBJS := $(SRCS:.fd=.C)

all:
	@echo "This is a makefile intended for the maintainer only."
	@echo "Only run it if you are absolutely sure about what you"
	@echo "are doing."

test: clean $(OBJS)

updatesrc: test
	mv *.C *.h ..

.fd.C:
	${SHELL} ./fdfix.sh $<

clean:
	rm -f *.[hcC] *.bak

distclean: clean
	rm -f *.orig *.rej *~
