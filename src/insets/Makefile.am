AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libinsets.la
LIBS =
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${srcdir}/../ $(SIGC_CFLAGS) $(BOOST_INCLUDES)

EXTRA_DIST = \
	insetlist.C \
	insetlist.h \
	insetsection.h \
	insetsection.C \
	insettheorem.C \
	insettheorem.h

libinsets_la_SOURCES = \
	ExternalTemplate.C \
	ExternalTemplate.h \
	figinset.C \
	figinset.h \
	inset.C \
	inset.h \
	insetbib.C \
	insetbib.h \
	insetbutton.C \
	insetbutton.h \
	insetcaption.C \
	insetcaption.h \
	insetcite.C \
	insetcite.h \
	insetcollapsable.C \
	insetcollapsable.h \
	insetcommand.C \
	insetcommand.h \
	inseterror.C \
	inseterror.h \
	insetert.C \
	insetert.h \
	insetexternal.C \
	insetexternal.h \
	insetfloat.h \
	insetfloat.C \
	insetfloatlist.C \
	insetfloatlist.h \
	insetfoot.C \
	insetfoot.h \
	insetfootlike.C \
	insetfootlike.h \
	insetgraphicsParams.h \
	insetgraphicsParams.C \
	insetgraphics.C \
	insetgraphics.h \
	insetinclude.C \
	insetinclude.h \
	insetindex.C \
	insetindex.h \
	insetlabel.C \
	insetlabel.h \
	insetlatexaccent.C \
	insetlatexaccent.h \
	insetmarginal.h \
	insetmarginal.C \
	insetminipage.C \
	insetminipage.h \
	insetnote.C \
	insetnote.h \
	insetparent.C \
	insetparent.h \
	insetquotes.C \
	insetquotes.h \
	insetref.C \
	insetref.h \
	insetspecialchar.C \
	insetspecialchar.h \
	insettabular.C \
	insettabular.h \
	insettext.C \
	insettext.h \
	insettoc.C \
	insettoc.h \
	inseturl.C \
	inseturl.h
#	insetlist.C \
#	insetlist.h \
#	insetsection.h \
#	insetsection.C \
#	insettheorem.C \
#	insettheorem.h \

libinsets.la: libinsets.o

libinsets.o: $(libinsets_la_OBJECTS) $(libinsets_la_DEPENDENCIES)
	$(CXXLINK) $(libinsets_la_OBJECTS)
