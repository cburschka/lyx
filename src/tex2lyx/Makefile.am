include $(top_srcdir)/config/common.am

INCLUDES = -I$(srcdir)/../ $(BOOST_INCLUDES)

#noinst_LTLIBRARIES = libtexparser.la
#
#libtexparser_la_SOURCES = \
#	texparser.C \
#	texparser.h 
#
#tex2lyx_LDADD = libtexparser.la

bin_PROGRAMS = tex2lyx

tex2lyx_SOURCES = \
	texparser.C \
	texparser.h \
	tex2lyx.C \
	tex2lyx.h \
	preamble.C \
	math.C \
	table.C \
	text.C 
