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
### Makefile by Lars Gullik Bjønnes (larsbj@lyx.org)
### Modified by Allan Rae <rae@lyx.org>
### Rewritten by Angus Leeming.
### Only update files that really have changed. (Lgb 20000924))

SHELL = /bin/sh
.SUFFIXES: .fd .c

# Various commands
FDESIGN = fdesign

SRCS := form_citation.fd \
	form_copyright.fd \
	form_document.fd \
	form_error.fd \
	form_graphics.fd \
	form_index.fd \
	form_paragraph.fd \
	form_preferences.fd \
	form_print.fd \
	form_ref.fd \
	form_tabular.fd \
	form_tabular_create.fd \
	form_toc.fd \
	form_url.fd

OBJS := $(SRCS:.fd=.C)

all:
	@echo "This is a makefile intended for the maintainer only."
	@echo "Only run it if you are absolutely sure about what you"
	@echo "are doing."

test: clean $(OBJS)

updatesrc: test
	for fil in *.C *.h; \
		do cmp -s $$fil ../$$fil || mv $$fil ..; \
	done

.fd.C:
	${SHELL} ./fdfix.sh $<

clean:
	rm -f *.[hHcC] *.bak

distclean: clean
	rm -f *.orig *.rej *~
