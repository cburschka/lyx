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

SRCS := form_bibitem.fd \
	form_bibtex.fd \
	form_character.fd \
	form_citation.fd \
	form_copyright.fd \
	form_document.fd \
	form_error.fd \
	form_filedialog.fd \
	form_graphics.fd \
	form_include.fd \
	form_index.fd \
	form_browser.fd \
	form_paragraph.fd \
	form_preamble.fd \
	form_preferences.fd \
	form_print.fd \
	form_ref.fd \
	form_search.fd \
	form_splash.fd \
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
	rm -f *.[hHcC] *.bak

.fd.C:
	${SHELL} ./fdfix.sh $<

clean:
	rm -f *.[hHcC] *.bak fdfixc_modified.sed

distclean: clean
	rm -f *.orig *.rej *~
