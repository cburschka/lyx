include $(top_srcdir)/config/common.am

EXTRA_DIST = $(SRCS)

SRCS = \
	BiblioModuleBase.ui \
	BulletsModuleBase.ui \
	ClassModuleBase.ui \
	LanguageModuleBase.ui \
	MarginsModuleBase.ui \
	NumberingModuleBase.ui \
	PackagesModuleBase.ui \
	PaperModuleBase.ui \
	PreambleModuleBase.ui \
	QAboutDialog.ui \
	QAskForTextDialog.ui \
	QBibitemDialog.ui \
	QBibtexDialog.ui \
	QCharacterDialog.ui \
	QCitationDialog.ui \
	QDelimiterDialog.ui \
	QDocumentDialog.ui \
	QErrorDialog.ui \
	QERTDialog.ui \
	QExternalDialog.ui \
	QFloatDialog.ui \
	QGraphicsDialog.ui \
	QIncludeDialog.ui \
	QIndexDialog.ui \
	QLogDialog.ui \
	QMathDialog.ui \
	QMathMatrixDialog.ui \
	QMinipageDialog.ui \
	QParagraphDialog.ui \
	QPreambleDialog.ui \
	QPrefAsciiModule.ui \
	QPrefColorsModule.ui \
	QPrefConvertersModule.ui \
	QPrefDateModule.ui \
	QPrefDisplayModule.ui \
	QPrefFileformatsModule.ui \
	QPrefKeyboardModule.ui \
	QPrefLanguageModule.ui \
	QPrefLatexModule.ui \
	QPrefPathsModule.ui \
	QPrefPrinterModule.ui \
	QPrefScreenFontsModule.ui \
	QPrefsDialog.ui \
	QPrefSpellcheckerModule.ui \
	QPrefUIModule.ui \
	QPrintDialog.ui \
	QRefDialog.ui \
	QSearchDialog.ui \
	QSendtoDialog.ui \
	QShowFileDialog.ui \
	QSpellcheckerDialog.ui \
	QTabularCreateDialog.ui \
	QTabularDialog.ui \
	QTexinfoDialog.ui \
	QThesaurusDialog.ui \
	QTocDialog.ui \
	QURLDialog.ui \
	QVCLogDialog.ui \
	QWrapDialog.ui


noinst_LTLIBRARIES = libqt2ui.la

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt2 \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_TRANSLATION


SUBDIRS = . moc

include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += $(UIDIALOGS)

nodist_libqt2ui_la_SOURCES = $(UIDIALOGS)

libqt2ui.la: $(libqt2ui_la_OBJECTS) $(libqt2ui_la_DEPENDENCIES)

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC) $(UICFLAGS) $< -o $@
%.C: %.h %.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

QAskForTextDialog.h: QAskForTextDialog.ui
	$(UIC) $(UICFLAGS) $< -o $@
QAskForTextDialog.C: QAskForTextDialog.h QAskForTextDialog.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

ClassModuleBase.h: ClassModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
ClassModuleBase.C: ClassModuleBase.h ClassModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

BulletsModuleBase.h: BulletsModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
BulletsModuleBase.C: BulletsModuleBase.h BulletsModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

PackagesModuleBase.h: PackagesModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
PackagesModuleBase.C: PackagesModuleBase.h PackagesModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

PaperModuleBase.h: PaperModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
PaperModuleBase.C: PaperModuleBase.h PaperModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

MarginsModuleBase.h: MarginsModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
MarginsModuleBase.C: MarginsModuleBase.h MarginsModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

LanguageModuleBase.h: LanguageModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
LanguageModuleBase.C: LanguageModuleBase.h LanguageModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

NumberingModuleBase.h: NumberingModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
NumberingModuleBase.C: NumberingModuleBase.h NumberingModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

BiblioModuleBase.h: BiblioModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
BiblioModuleBase.C: BiblioModuleBase.h BiblioModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

PreambleModuleBase.h: PreambleModuleBase.ui
	$(UIC) $(UICFLAGS) $< -o $@
PreambleModuleBase.C: PreambleModuleBase.h PreambleModuleBase.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@


%Module.h: %Module.ui
	$(UIC) $(UICFLAGS) $< -o $@
%Module.C: %Module.h %Module.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@

%Base.h: %.ui
	$(UIC) $(UICFLAGS) $< -o $@
%Base.C: %Base.h %.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@
