include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

CLEANFILES += *.h

EXTRA_DIST = $(UIFILES)

BUILT_SOURCES = $(UIFILES:.ui=.h)

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC4) $(UIC4FLAGS) $< -o $@
