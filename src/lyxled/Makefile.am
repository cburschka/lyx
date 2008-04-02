include $(top_srcdir)/config/common.am

BUILT_SOURCES = $(MOCEDFILES)
#BUILT_SOURCES = $(UIFILES:%.ui=ui_%.h)
#BUILT_SOURCES += Resources.cpp Resources.qrc

CLEANFILES += $(BUILT_SOURCES)

#########################  Qt stuff  #############################
# Use _() for localization instead of tr() or trUtf8()
UIC4FLAGS=-tr lyx::qt_

ui_%.h: ui/%.ui
	$(UIC4) $(UIC4FLAGS) $< -o $@

MOCEDFILES = $(MOCHEADER:%.h=%_moc.cpp)

%_moc.cpp: %.h
	$(MOC4) -o $@ $<

#Resources.qrc: Makefile
#	echo "<!DOCTYPE RCC><RCC version='1.0'><qresource>" > $@
#	find $(top_srcdir)/lib/images -name '*.png' \
#		| sed -e 's:$(top_srcdir)/lib/\(.*\):<file alias="\1">&</file>:' \
#		>> $@
#	echo "</qresource></RCC>" >> $@
#
#Resources.cpp: Resources.qrc
#	$(RCC4) $< -name Resources -o $@


#########################  LIBRARIES  #############################

bin_PROGRAMS = lyxled

lyxled_LDADD = $(QT4_LIB)
lyxled_DEPENDENCIES = $(MOCEDFILES)

AM_CPPFLAGS += \
	$(QT4_CPPFLAGS) \
	-UQT_NO_KEYWORDS \
	-I$(top_srcdir)/src \
	$(QT4_INCLUDES)

SOURCEFILES = \
	main.cpp \
	LayoutEditor.cpp \
	LayoutEditor_moc.cpp

NOMOCHEADER = 

MOCHEADER = \
	LayoutEditor.h

UIFILES = 

lyxled_SOURCES = \
	$(SOURCEFILES) \
	$(MOCEDFILES) \
	$(MOCHEADER) \
	$(NOMOCHEADER)

EXTRA_DIST = ui/compile_uic.sh $(UIFILES:%=ui/%)
#EXTRA_DIST = pch.h ui/compile_uic.sh $(UIFILES:%=ui/%)
#BUILT_SOURCES += $(PCH_FILE)
#AM_CPPFLAGS += $(PCH_FLAGS)
