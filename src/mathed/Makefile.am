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
	xarray.C \
	xarray.h \
	formulabase.C \
	formulabase.h \
	formula.C \
	formula.h \
	formulamacro.C \
	formulamacro.h \
	math_arrayinset.C \
	math_arrayinset.h \
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
	math_gridinset.C \
	math_gridinset.h \
	math_hash.C \
	math_inset.C \
	math_inset.h \
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
	math_parser.C \
	math_parser.h \
	math_rootinset.C \
	math_rootinset.h \
	math_scriptinset.C \
	math_scriptinset.h \
	math_sizeinset.C \
	math_sizeinset.h \
	math_spaceinset.C \
	math_spaceinset.h \
	math_sqrtinset.C \
	math_sqrtinset.h \
	math_utils.C \
	math_utils.h \
	support.C \
	support.h \
	symbol_def.h
