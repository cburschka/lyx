include $(top_srcdir)/config/common.am

SHELL = /bin/sh

SUFFIXES = .fd

SRCS =  form_aboutlyx.fd \
	form_bibitem.fd \
	form_bibtex.fd \
	form_bibtex.fd \
	form_browser.fd \
	form_character.fd \
	form_citation.fd \
	form_document.fd \
	form_error.fd \
	form_ert.fd \
	form_external.fd \
	form_filedialog.fd \
	form_float.fd \
	form_forks.fd \
	form_graphics.fd \
	form_include.fd \
	form_index.fd \
	form_maths_deco.fd \
	form_maths_delim.fd \
	form_maths_matrix.fd \
	form_maths_panel.fd \
	form_maths_space.fd \
	form_maths_style.fd \
	form_minipage.fd \
	form_paragraph.fd \
	form_preamble.fd \
	form_preferences.fd \
	form_print.fd \
	form_ref.fd \
	form_search.fd \
	form_sendto.fd \
	form_spellchecker.fd \
	form_tabular.fd \
	form_tabular_create.fd \
	form_texinfo.fd \
	form_thesaurus.fd \
	form_toc.fd \
	form_url.fd

OBJS = $(SRCS:.fd=.C)

#all:
#	@echo "This is a makefile intended for the maintainer only."
#	@echo "Only run it if you are absolutely sure about what you"
#	@echo "are doing."

test: clean $(OBJS)

updatesrc: test
	for fil in *.C *.h; \
	do cmp -s $$fil ../$$fil || mv $$fil ..; \
	done
	rm -f *.[hHcC] *.bak

.fd.C:
	${SHELL} ./fdfix.sh $<

clean-local:
	rm -f *.[hHcC] *.bak fdfixc_modified.sed

distclean-local:
	rm -f *.orig *.rej *~
