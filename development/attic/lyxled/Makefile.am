include $(top_srcdir)/config/common.am

BUILT_SOURCES = $(MOCEDFILES)
#BUILT_SOURCES = $(UIFILES:%.ui=ui_%.h)
#BUILT_SOURCES += Resources.cpp Resources.qrc

CLEANFILES += $(BUILT_SOURCES)

#########################  Qt stuff  #############################
# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr lyx::qt_

ui_%.h: ui/%.ui
	$(QT_UIC) $(UICFLAGS) $< -o $@

MOCEDFILES = $(MOCHEADER:%.h=%_moc.cpp)

%_moc.cpp: %.h
	$(QT_MOC) -o $@ $<

#Resources.qrc: Makefile
#	echo "<!DOCTYPE RCC><RCC version='1.0'><qresource>" > $@
#	find $(top_srcdir)/lib/images -name '*.png' \
#		| sed -e 's:$(top_srcdir)/lib/\(.*\):<file alias="\1">&</file>:' \
#		>> $@
#	echo "</qresource></RCC>" >> $@
#
#Resources.cpp: Resources.qrc
#	$(QT_RCC) $< -name Resources -o $@


#########################  LIBRARIES  #############################

bin_PROGRAMS = lyxled

lyxled_LDADD = $(QT_LIB)
lyxled_DEPENDENCIES = $(MOCEDFILES)

AM_CPPFLAGS += \
	$(QT_CPPFLAGS) \
	-UQT_NO_KEYWORDS \
	-I$(top_srcdir)/src \
	$(QT_INCLUDES)

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
include $(top_srcdir)/config/common.am

BUILT_SOURCES = $(MOCEDFILES)
#BUILT_SOURCES = $(UIFILES:%.ui=ui_%.h)
#BUILT_SOURCES += Resources.cpp Resources.qrc

CLEANFILES += $(BUILT_SOURCES)

#########################  Qt stuff  #############################
# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr lyx::qt_

ui_%.h: ui/%.ui
	$(QT_UIC) $(UICFLAGS) $< -o $@

MOCEDFILES = $(MOCHEADER:%.h=%_moc.cpp)

%_moc.cpp: %.h
	$(QT_MOC) -o $@ $<

#Resources.qrc: Makefile
#	echo "<!DOCTYPE RCC><RCC version='1.0'><qresource>" > $@
#	find $(top_srcdir)/lib/images -name '*.png' \
#		| sed -e 's:$(top_srcdir)/lib/\(.*\):<file alias="\1">&</file>:' \
#		>> $@
#	echo "</qresource></RCC>" >> $@
#
#Resources.cpp: Resources.qrc
#	$(QT_RCC) $< -name Resources -o $@


#########################  LIBRARIES  #############################

bin_PROGRAMS = lyxled

lyxled_LDADD = $(QT_LIB)
lyxled_DEPENDENCIES = $(MOCEDFILES)

AM_CPPFLAGS += \
	$(QT_CPPFLAGS) \
	-UQT_NO_KEYWORDS \
	-I$(top_srcdir)/src \
	$(QT_INCLUDES)

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
