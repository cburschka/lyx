include $(top_srcdir)/config/common.am

AM_CPPFLAGS += -I$(top_srcdir)/src $(BOOST_INCLUDES) -I$(top_srcdir)/src/frontends

EXTRA_DIST = tests/regfiles/biblio

noinst_LTLIBRARIES = liblyxcontrollers.la

SOURCEFILES = \
	Dialog.cpp \
	ButtonPolicy.cpp \
	ControlCommand.cpp \
	ControlMath.cpp \
	ControlParagraph.cpp \
	frontend_helpers.cpp 

HEADERFILES = \
	Dialog.h \
	ButtonPolicy.h \
	ControlCommand.h \
	ControlMath.h \
	ControlParagraph.h \
	frontend_helpers.h

if MONOLITHIC_CONTROLLERS

lyxcontrollers.cpp:
	@echo -e '$(SOURCEFILES:%=\n#include "%")\n' > $@

BUILT_SOURCES = lyxcontrollers.cpp

liblyxcontrollers_la_SOURCES = lyxcontrollers.cpp $(HEADERFILES) 

else

EXTRA_DIST += pch.h
BUILT_SOURCES = $(PCH_FILE)
AM_CPPFLAGS += $(PCH_FLAGS)

liblyxcontrollers_la_SOURCES = $(SOURCEFILES) $(HEADERFILES)

endif
