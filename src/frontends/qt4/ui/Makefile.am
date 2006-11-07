include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

CLEANFILES += *.h

EXTRA_DIST = $(UIFILES)

BUILT_SOURCES = $(UIFILES:.ui=.h)

# Use _() for localization instead of tr() or trUtf8()
UIC4FLAGS=-tr lyx::qt_

%.h: %.ui
	$(UIC4) $(UIC4FLAGS) $< -o $@
