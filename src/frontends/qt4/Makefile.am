include $(top_srcdir)/config/common.am

CLEANFILES += $(UIFILES:%.ui=ui_%.h)
CLEANFILES += $(MOCEDFILES)

BUILT_SOURCES = $(UIFILES:%.ui=ui_%.h)
BUILT_SOURCES += $(MOCEDFILES)

#########################  Translation  #############################
# Use _() for localization instead of tr() or trUtf8()
UIC4FLAGS=-tr lyx::qt_

ui_%.h: ui/%.ui
	$(UIC4) $(UIC4FLAGS) $< -o $@

MOCEDFILES = $(MOCHEADER:%.h=%_moc.cpp)

%_moc.cpp: %.h
	$(MOC4) -o $@ $<

#########################  LIBRARIES  #############################

pkglib_LTLIBRARIES = liblyxqt4.la

liblyxqt4_la_DEPENDENCIES = $(MOCEDFILES)
liblyxqt4_la_LDFLAGS = $(QT4_LDFLAGS)
liblyxqt4_la_LIBADD = $(QT4_LIB) 

AM_CPPFLAGS += \
	$(QT4_CPPFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/images \
	$(QT4_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

SOURCEFILES = \
	alert_pimpl.cpp \
	CheckedLineEdit.cpp \
	Dialogs.cpp \
	FileDialog.cpp \
	GuiClipboard.cpp \
	GuiFontLoader.cpp \
	GuiFontMetrics.cpp \
	GuiSelection.cpp \
	KeySymbol.cpp \
	QCitation.cpp \
	QFloat.cpp \
	QFontExample.cpp \
	QLImage.cpp \
	QLPainter.cpp \
	QPrint.cpp \
	Qt2BC.cpp \
	qt_helpers.cpp \
	UrlView.cpp \
	Action.cpp \
	BulletsModule.cpp \
	ColorCache.cpp \
	EmptyTable.cpp \
	FloatPlacement.cpp \
	GuiApplication.cpp \
	GuiImplementation.cpp \
	GuiView.cpp \
	GuiWorkArea.cpp \
	IconPalette.cpp \
	InsertTableWidget.cpp \
	LengthCombo.cpp \
	LyXFileDialog.cpp \
	PanelStack.cpp \
	QAbout.cpp \
	QBibitem.cpp \
	QBibtex.cpp \
	QBox.cpp \
	QBranch.cpp \
	QBranches.cpp \
	QChanges.cpp \
	QCharacter.cpp \
	QCitationDialog.cpp \
	QCommandBuffer.cpp \
	QCommandEdit.cpp \
	QDelimiterDialog.cpp \
	QDialogView.cpp \
	QDocument.cpp \
	QErrorList.cpp \
	QERT.cpp \
	QExternal.cpp \
	QFloatDialog.cpp \
	QGraphics.cpp \
	QGraphicsDialog.cpp \
	QInclude.cpp \
	QIndex.cpp \
	QKeySymbol.cpp \
	QListings.cpp \
	QLMenubar.cpp \
	QLog.cpp \
	QLPopupMenu.cpp \
	QLPrintDialog.cpp \
	QLToolbar.cpp \
	QMathMatrixDialog.cpp \
	QNomencl.cpp \
	QNote.cpp \
	QParagraph.cpp \
	QPrefs.cpp \
	QRef.cpp \
	QSearch.cpp \
	QSendto.cpp \
	QSetBorder.cpp \
	QShowFile.cpp \
	QSpellchecker.cpp \
	QTabular.cpp \
	QTabularCreate.cpp \
	QTexinfo.cpp \
	QThesaurus.cpp \
	QToc.cpp \
	QURLDialog.cpp \
	QViewSource.cpp \
	QVSpace.cpp \
	QWrap.cpp \
	socket_callback.cpp \
	TocModel.cpp \
	TocWidget.cpp \
	Validator.cpp 

NOMOCHEADER = \
	CheckedLineEdit.h \
	GuiClipboard.h \
	GuiFontLoader.h \
	GuiFontMetrics.h \
	GuiSelection.h \
	QGraphics.h \
	QLImage.h \
	QLPainter.h \
	QPrint.h \
	Qt2BC.h \
	qt_helpers.h

MOCHEADER = \
	Action.h \
	BulletsModule.h \
	ColorCache.h \
	DockView.h \
	EmptyTable.h \
	FloatPlacement.h \
	GuiApplication.h \
	GuiImplementation.h \
	GuiView.h \
	GuiWorkArea.h \
	IconPalette.h \
	InsertTableWidget.h \
	LengthCombo.h \
	LyXFileDialog.h \
	PanelStack.h \
	QAbout.h \
	QBibitem.h \
	QBibtex.h \
	QBox.h \
	QBranches.h \
	QBranch.h \
	QChanges.h \
	QCharacter.h \
	QCitationDialog.h \
	QCitation.h \
	QCommandBuffer.h \
	QCommandEdit.h \
	QDelimiterDialog.h \
	QDialogView.h \
	QDocument.h \
	QErrorList.h \
	QERT.h \
	QExternal.h \
	QFloatDialog.h \
	QFloat.h \
	QFontExample.h \
	QGraphicsDialog.h \
	QInclude.h \
	QIndex.h \
	QKeySymbol.h \
	QListings.h \
	qlkey.h \
	QLMenubar.h \
	QLog.h \
	QLPopupMenu.h \
	QLPrintDialog.h \
	QLToolbar.h \
	QMathMatrixDialog.h \
	QNomencl.h \
	QNote.h \
	QParagraph.h \
	QPrefs.h \
	QRef.h \
	QSearch.h \
	QSendto.h \
	QSetBorder.h \
	QShowFile.h \
	QSpellchecker.h \
	QTabularCreate.h \
	QTabular.h \
	QTexinfo.h \
	QThesaurus.h \
	QToc.h \
	QURLDialog.h \
	QViewSource.h \
	QVSpace.h \
	QWrap.h \
	socket_callback.h \
	TocModel.h \
	TocWidget.h \
	Validator.h

UIFILES = \
	AboutUi.ui \
	AskForTextUi.ui \
	BibitemUi.ui \
	BiblioUi.ui \
	BibtexAddUi.ui \
	BibtexUi.ui \
	BoxUi.ui \
	BranchesUi.ui \
	BranchUi.ui \
	BulletsUi.ui \
	ChangesUi.ui \
	CharacterUi.ui \
	CitationUi.ui \
	DelimiterUi.ui \
	DocumentUi.ui \
	ErrorListUi.ui \
	ERTUi.ui \
	ExternalUi.ui \
	FloatPlacementUi.ui \
	FloatUi.ui \
	FontUi.ui \
	GraphicsUi.ui \
	IncludeUi.ui \
	IndexUi.ui \
	LanguageUi.ui \
	LaTeXUi.ui \
	ListingsUi.ui \
	LogUi.ui \
	MarginsUi.ui \
	MathMatrixUi.ui \
	MathsUi.ui \
	NomenclUi.ui \
	NoteUi.ui \
	NumberingUi.ui \
	PageLayoutUi.ui \
	ParagraphUi.ui \
	PreambleUi.ui \
	PrefColorsUi.ui \
	PrefConvertersUi.ui \
	PrefCopiersUi.ui \
	PrefCygwinPathUi.ui \
	PrefDateUi.ui \
	PrefDisplayUi.ui \
	PrefFileformatsUi.ui \
	PrefIdentityUi.ui \
	PrefKeyboardUi.ui \
	PrefLanguageUi.ui \
	PrefLatexUi.ui \
	PrefPathsUi.ui \
	PrefPlaintextUi.ui \
	PrefPrinterUi.ui \
	PrefScreenFontsUi.ui \
	PrefSpellcheckerUi.ui \
	PrefsUi.ui \
	PrefUi.ui \
	PrintUi.ui \
	RefUi.ui \
	SearchUi.ui \
	SendtoUi.ui \
	ShowFileUi.ui \
	SpellcheckerUi.ui \
	TabularCreateUi.ui \
	TabularUi.ui \
	TexinfoUi.ui \
	TextLayoutUi.ui \
	ThesaurusUi.ui \
	TocUi.ui \
	URLUi.ui \
	ViewSourceUi.ui \
	VSpaceUi.ui \
	WrapUi.ui


if MONOLITHIC_FRONTEND_QT4


liblyxqt4.cpp:
	@echo -e '$(SOURCEFILES:%=\n#include "%")\n' > $@

liblyxqt4_la_SOURCES = \
	liblyxqt4.cpp \
	$(MOCHEADER) \
	$(NOMOCHEADER)

BUILT_SOURCES += liblyxqt4.cpp

else


liblyxqt4_la_SOURCES = \
	$(SOURCEFILES) \
	$(MOCHEADER) \
	$(NOMOCHEADER)

EXTRA_DIST = pch.h
BUILT_SOURCES += $(PCH_FILE)
AM_CPPFLAGS += $(PCH_FLAGS)


endif
