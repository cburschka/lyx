include $(top_srcdir)/config/common.am

EXTRA_DIST = fdfixc.sed fdfixh.sed fdfix.sh README

noinst_LTLIBRARIES = libfdesign.la

# For (forms_fwd.h, forms_gettext.h) and LString.h, respectively.
INCLUDES = -I$(srcdir)/..  -I$(top_srcdir)/src

# These are used to generate the .[Ch] files from the .fd ones.
# It would be nice if we could make the .Ch files dependent on the fdfix files.
SHELL = /bin/sh
SCRIPT = $(srcdir)/fdfix.sh
SUFFIXES = .fd

SRCS =  form_aboutlyx.fd \
	form_bibitem.fd \
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

libfdesign_la_SOURCES = $(SRCS:.fd=.C) $(SRCS:.fd=.h)

libfdesign.la: $(libfdesign_la_OBJECTS) $(libfdesign_la_DEPENDENCIES)

.fd.C:
	$(SHELL) $(SCRIPT) $<

clean-local:
	rm -f *.[ChH] *.orig \
	rm -f $(srcdir)/*.[hc] $(srcdir)/*.bak

distclean-local:
	rm -f $(srcdir)/*.orig $(srcdir)/*.rej $(srcdir)/*~
