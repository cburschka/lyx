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

linked_files = \
	FloatList.C \
	Floating.C \
	boost.C \
	counters.C \
	lyxlayout.h \
	lyxlayout.C \
	lyxtextclass.C \
	lyxtextclass.h \
	lyxlex.C \
	lyxlex_pimpl.C

tex2lyx_SOURCES = \
	$(linked_files) \
	Spacing.h \
	gettext.C \
	gettext.h \
	lyxfont.C \
	lyxfont.h \
	texparser.C \
	texparser.h \
	tex2lyx.C \
	tex2lyx.h \
	preamble.C \
	math.C \
	table.C \
	text.C

tex2lyx_LDADD = \
	../support/libsupport.la \
	../../boost/libs/regex/src/libboostregex.la

FloatList.C: link_files 

link_files:
	for i in $(linked_files); do \
		rm -f $$i; \
		ln -s "$(top_srcdir)/src/$$i" . ; \
	done

rm_link_files:
	rm -f $(linked_files)
