AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libmathed.la
LIBS=
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${srcdir}/../ $(SIGC_CFLAGS) $(BOOST_INCLUDES)

libmathed_la_SOURCES = \
	array.C \
	array.h \
	formula.C \
	formula.h \
	formulamacro.C \
	formulamacro.h \
	macro_support.C \
	macro_support.h \
	math_sqrtinset.C \
	math_sqrtinset.h \
	math_accentinset.C \
	math_accentinset.h \
	math_bigopinset.C \
	math_bigopinset.h \
	math_cursor.C \
	math_cursor.h \
	math_decorationinset.C \
	math_decorationinset.h \
	math_defs.h \
	math_deliminset.C \
	math_deliminset.h \
	math_dotsinset.C \
	math_dotsinset.h \
	math_fracinset.C \
	math_fracinset.h \
	math_funcinset.C \
	math_funcinset.h \
	math_hash.C \
	math_inset.C \
	math_inset.h \
	math_iter.C \
	math_iter.h \
	math_macro.C \
	math_macro.h \
	math_macroarg.C \
	math_macroarg.h \
	math_macrotemplate.C \
	math_macrotemplate.h \
	math_macrotable.C \
	math_macrotable.h \
	math_matrixinset.C \
	math_matrixinset.h \
	math_parinset.C \
	math_parinset.h \
	math_parser.C \
	math_parser.h \
	math_root.C \
	math_root.h \
	math_rowst.C \
	math_rowst.h \
	math_spaceinset.C \
	math_spaceinset.h \
	math_utils.C \
	math_utils.h \
	math_xiter.C \
	math_xiter.h \
	matriz.C \
	matriz.h \
	support.C \
	support.h \
	symbol_def.h
