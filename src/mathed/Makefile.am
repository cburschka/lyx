AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libmathed.la
LIBS=
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/images -I${srcdir}/../ $(SIGC_CFLAGS) $(BOOST_INCLUDES)

libmathed_la_SOURCES = \
	array.C \
	array.h \
	formula.C \
	formula.h \
	formulamacro.C \
	formulamacro.h \
	math_cursor.C \
	math_cursor.h \
	math_defs.h \
	math_delim.C \
	math_draw.C \
	math_forms.C \
	math_forms.h \
	math_hash.C \
	math_inset.C \
	math_inset.h \
	math_iter.C \
	math_iter.h \
	math_macro.C \
	math_macro.h \
	math_panel.C \
	math_panel.h \
	math_parser.C \
	math_parser.h \
	math_root.C \
	math_root.h \
	math_symbols.C \
	math_utils.C \
	math_write.C \
	symbol_def.h
