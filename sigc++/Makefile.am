SUBDIRS =  macros
DISTCLEANFILES= *.orig *.rej *~ *.bak core 
MAINTAINERCLEANFILES= $(srcdir)/Makefile.in $(srcdir)/configure $(srcdir)/sigc++config* $(srcdir)/stamp* $(srcdir)/aclocal.m4 $(srcdir)/acinclude.m4 
ETAGS_ARGS = --lang=c++

INCLUDES = -I$(top_srcdir)/.. -I$(top_builddir)/..
M4_DIR = $(top_srcdir)/macros
M4_INCLUDES = -I$(M4_DIR)

BUILT_SOURCES = slot.h object_slot.h func_slot.h class_slot.h \
                basic_signal.h \
                convert.h bind.h rettype.h retbind.h

EXTRA_DIST = $(BUILT_SOURCES)

libsigc_la_SOURCES= $(BUILT_SOURCES) \
   object.cc   scope.cc   marshal.cc   slot.cc\
   basic_signal.cc adaptor.cc thread.cc

libsigc_la_LDFLAGS =  \
                      -no-undefined

noinst_LTLIBRARIES = libsigc.la

noinst_HEADERS = \
    marshal.h handle_system.h signal_system.h \
     $(BUILT_SOURCES) type.h\
    handle.h object.h scope.h adaptor.h thread.h


Makefile: $(BUILT_SOURCES)

%.h: $(M4_DIR)/%.h.m4 $(M4_DIR)/template.macros.m4        
	$(M4) $(M4_INCLUDES) $(DEFINES) $<  > $@

all-local: $(noinst_LTLIBRARIES) 

