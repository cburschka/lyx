include $(top_srcdir)/config/common.am

CLEANFILES += $(man_MANS)

EXTRA_DIST = pch.h lyxclient.man

BUILT_SOURCES = $(PCH_FILE)

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

lyxclient_LDADD = \
	$(top_builddir)/src/liblyxcore.la \
	$(top_builddir)/src/liblyxinsets.la \
	$(top_builddir)/src/liblyxmathed.la \
	$(top_builddir)/src/liblyxgraphics.la \
	$(top_builddir)/src/frontends/liblyxfrontends.la \
	$(top_builddir)/src/frontends/controllers/liblyxcontrollers.la \
	$(top_builddir)/src/frontends/qt4/liblyxqt4.la \
	$(top_builddir)/src/support/liblyxsupport.la \
	$(BOOST_LIBS) $(INTLLIBS) @LIBS@ $(SOCKET_LIBS)  $(QT4_LIB) $(AIKSAURUS_LIBS)

# everything below the line containing a single backslash and the 
# spellchecker stuff is an ugly hack and needed because of the
# linking problems described in ../Makefile.am
# These four objects are linked as object files as they are not
# referenced within the core and therefore are not picked up
# by the linker without looping over libs. We do not want that,
# and in fact libtools seems not able to do that.

if USE_ASPELL
ASPELL = ../ASpell.cpp ../ASpell_local.h
endif
if USE_PSPELL
PSPELL = ../PSpell.cpp ../PSpell.h
endif
if USE_ISPELL
ISPELL = ../ISpell.cpp ../ISpell.h
endif

lyxclient_SOURCES = \
  boost.cpp \
  client.cpp \
  debug.cpp \
  debug.h \
  gettext.cpp \
  Messages.cpp \
  Messages.h \
	\
	$(ASPELL) $(PSPELL) $(ISPELL) ../SpellBase.cpp \
	../Box.cpp \
	../Box.h \
	../Dimension.cpp \
	../Dimension.h \
	../PrinterParams.cpp \
	../PrinterParams.h \
	../Thesaurus.cpp \
	../Thesaurus.h

lyxclient.1:
	cp -p $(srcdir)/lyxclient.man lyxclient.1
