include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libqt2ui.la

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt2 \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR


SUBDIRS = . moc

include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += $(UIDIALOGS)

nodist_libqt2ui_la_SOURCES = $(UIDIALOGS)

libqt2ui.la: $(libqt2ui_la_OBJECTS) $(libqt2ui_la_DEPENDENCIES)

# a pattern to change tr() to _()

SED = sed
SEDREPLACETR = s,[^A-Za-z]tr[ ]*(,_(,g
SEDREMOVEEMPTY = s,_[ ]*([ ]*""[ ]*),"",g

QAskForTextDialog.h: QAskForTextDialog.ui
	$(UIC) $< -o $@
QAskForTextDialog.C: QAskForTextDialog.h QAskForTextDialog.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

ClassModuleBase.h: ClassModuleBase.ui
	$(UIC) $< -o $@
ClassModuleBase.C: ClassModuleBase.h ClassModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

BulletsModuleBase.h: BulletsModuleBase.ui
	$(UIC) $< -o $@
BulletsModuleBase.C: BulletsModuleBase.h BulletsModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

PackagesModuleBase.h: PackagesModuleBase.ui
	$(UIC) $< -o $@
PackagesModuleBase.C: PackagesModuleBase.h PackagesModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

PaperModuleBase.h: PaperModuleBase.ui
	$(UIC) $< -o $@
PaperModuleBase.C: PaperModuleBase.h PaperModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

MarginsModuleBase.h: MarginsModuleBase.ui
	$(UIC) $< -o $@
MarginsModuleBase.C: MarginsModuleBase.h MarginsModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

LanguageModuleBase.h: LanguageModuleBase.ui
	$(UIC) $< -o $@
LanguageModuleBase.C: LanguageModuleBase.h LanguageModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

NumberingModuleBase.h: NumberingModuleBase.ui
	$(UIC) $< -o $@
NumberingModuleBase.C: NumberingModuleBase.h NumberingModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

BiblioModuleBase.h: BiblioModuleBase.ui
	$(UIC) $< -o $@
BiblioModuleBase.C: BiblioModuleBase.h BiblioModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@

PreambleModuleBase.h: PreambleModuleBase.ui
	$(UIC) $< -o $@
PreambleModuleBase.C: PreambleModuleBase.h PreambleModuleBase.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@



%Base.h: %.ui
	$(UIC) $< -o $@
%Base.C: %Base.h %.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@
