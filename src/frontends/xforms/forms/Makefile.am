include $(top_srcdir)/config/common.am

MAINTAINERCLEANFILES += *.C *.h *.c

EXTRA_DIST = fdfixc.sed fdfixh.sed fdfix.sh tmp_str.sed README $(SRCS)

# For (forms_fwd.h, forms_gettext.h) and support/std_string.h, respectively.
INCLUDES = -I$(srcdir)/..  -I$(top_srcdir)/src -I..

noinst_LTLIBRARIES = libfdesign.la

SRCS =  form_aboutlyx.fd \
	form_bibitem.fd \
	form_bibtex.fd \
	form_browser.fd \
	form_branch.fd \
	form_changes.fd \
	form_character.fd \
	form_citation.fd \
	form_colorpicker.fd \
	form_document.fd \
	form_errorlist.fd \
	form_ert.fd \
	form_external.fd \
	form_filedialog.fd \
	form_float.fd \
	form_forks.fd \
	form_graphics.fd \
	form_include.fd \
	form_maths_delim.fd \
	form_maths_matrix.fd \
	form_maths_panel.fd \
	form_maths_space.fd \
	form_maths_style.fd \
	form_minipage.fd \
	form_note.fd \
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
	form_text.fd \
	form_thesaurus.fd \
	form_toc.fd \
	form_url.fd \
	form_wrap.fd

nodist_libfdesign_la_SOURCES = $(SRCS:.fd=.C) $(SRCS:.fd=.h)
# This is a trick so that the actual .la file never gets build,
# so we do not need to use diskspace and time for it.
# It is only getting linked into libxforms.la anyway.
libfdesign_la_LINK = touch libfdesign.la

# These are used to generate the .[Ch] files from the .fd ones.
SHELL = /bin/sh
SCRIPT = $(srcdir)/fdfix.sh
SUFFIXES = .fd

%.C: %.fd $(srcdir)/fdfix.sh $(srcdir)/fdfix[ch].sed $(srcdir)/tmp_str.sed
	$(SHELL) $(SCRIPT) $<
