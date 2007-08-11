include $(top_srcdir)/config/common.am

CLEANFILES += *.h
CLEANFILES += *_moc.cpp
CLEANFILES += $(UIFILES:%.ui=ui_%.h)

BUILT_SOURCES = $(UIFILES:%.ui=ui_%.h)
BUILT_SOURCES += $(PCH_FILE)
BUILT_SOURCES += $(MOCEDFILES)

EXTRA_DIST = pch.h

#########################  Translation  #############################
# Use _() for localization instead of tr() or trUtf8()
UIC4FLAGS=-tr lyx::qt_

ui_%.h: ui/%.ui
	$(UIC4) $(UIC4FLAGS) $< -o $@

MOCEDFILES = $(MOCFILES:%.cpp=%_moc.cpp)

%_moc.cpp: %.h
	$(MOC4) -o $@ $<


#########################  LIBRARIES  #############################

pkglib_LTLIBRARIES = liblyxqt4.la

liblyxqt4_la_DEPENDENCIES = $(MOCEDFILES)
liblyxqt4_la_LDFLAGS = $(QT4_LDFLAGS)
liblyxqt4_la_LIBADD = $(QT4_LIB) 

AM_CPPFLAGS += \
	$(QT4_CPPFLAGS) \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/images \
	$(QT4_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

liblyxqt4_la_SOURCES = \
	Action.cpp Action.h \
	alert_pimpl.cpp \
	BulletsModule.cpp \
	BulletsModule.h \
	CheckedLineEdit.cpp \
	CheckedLineEdit.h \
	ColorCache.cpp \
	ColorCache.h \
	Dialogs.cpp \
	DockView.h \
	EmptyTable.cpp \
	EmptyTable.h \
	FileDialog.cpp \
	FloatPlacement.cpp \
	FloatPlacement.h \
	GuiApplication.cpp \
	GuiApplication.h \
	GuiClipboard.cpp \
	GuiClipboard.h \
	GuiFontLoader.cpp \
	GuiFontLoader.h \
	GuiFontMetrics.cpp \
	GuiFontMetrics.h \
	GuiImplementation.cpp \
	GuiImplementation.h \
	GuiSelection.cpp \
	GuiSelection.h \
	GuiView.cpp \
	GuiView.h \
	GuiWorkArea.cpp \
	GuiWorkArea.h \
	IconPalette.cpp \
	IconPalette.h \
	InsertTableWidget.cpp \
	InsertTableWidget.h \
	KeySymbol.cpp \
	LengthCombo.cpp \
	LengthCombo.h \
	LyXFileDialog.cpp \
	LyXFileDialog.h \
	PanelStack.cpp \
	PanelStack.h \
	QAbout.cpp \
	QAbout.h \
	QBibitem.cpp \
	QBibitem.h \
	QBibtex.cpp \
	QBibtex.h \
	QBox.cpp \
	QBox.h \
	QBranch.cpp \
	QBranches.cpp \
	QBranches.h \
	QBranch.h \
	QChanges.cpp \
	QChanges.h \
	QCharacter.cpp \
	QCharacter.h \
	QCitation.cpp \
	QCitationDialog.cpp \
	QCitationDialog.h \
	QCitation.h \
	QCommandBuffer.cpp \
	QCommandBuffer.h \
	QCommandEdit.cpp \
	QCommandEdit.h \
	QDelimiterDialog.cpp \
	QDelimiterDialog.h \
	QDialogView.cpp \
	QDialogView.h \
	QDocument.cpp \
	QDocument.h \
	QErrorList.cpp \
	QErrorList.h \
	QERT.cpp \
	QERT.h \
	QExternal.cpp \
	QExternal.h \
	QFloat.cpp \
	QFloatDialog.cpp \
	QFloatDialog.h \
	QFloat.h \
	QFontExample.cpp \
	QFontExample.h \
	QGraphics.cpp \
	QGraphicsDialog.cpp \
	QGraphicsDialog.h \
	QGraphics.h \
	QInclude.cpp \
	QInclude.h \
	QIndex.cpp \
	QIndex.h \
	QKeySymbol.cpp \
	QKeySymbol.h \
	QLImage.cpp \
	QLImage.h \
	QListings.cpp \
	QListings.h \
	qlkey.h \
	QLMenubar.cpp \
	QLMenubar.h \
	QLog.cpp \
	QLog.h \
	QLPainter.cpp \
	QLPainter.h \
	QLPopupMenu.cpp \
	QLPopupMenu.h \
	QLPrintDialog.cpp \
	QLPrintDialog.h \
	QLToolbar.cpp \
	QLToolbar.h \
	QMathMatrixDialog.cpp \
	QMathMatrixDialog.h \
	QNomencl.cpp \
	QNomencl.h \
	QNote.cpp \
	QNote.h \
	QParagraph.cpp \
	QParagraph.h \
	QPrefs.cpp \
	QPrefs.h \
	QPrint.cpp \
	QPrint.h \
	QRef.cpp \
	QRef.h \
	QSearch.cpp \
	QSearch.h \
	QSendto.cpp \
	QSendto.h \
	QSetBorder.cpp \
	QSetBorder.h \
	QShowFile.cpp \
	QShowFile.h \
	QSpellchecker.cpp \
	QSpellchecker.h \
	Qt2BC.cppQt2BC.h\ \
	QTabular.cpp \
	QTabularCreate.cpp \
	QTabularCreate.h \
	QTabular.h \
	QTexinfo.cpp \
	QTexinfo.h \
	qt_helpers.cpp \
	qt_helpers.h \
	QThesaurus.cpp \
	QThesaurus.h \
	QToc.cpp \
	QToc.h \
	qtTimeout.cpp \
	qtTimeout.h \
	QURLDialog.cpp \
	QURLDialog.h \
	QViewSource.cpp \
	QViewSource.h \
	QVSpace.cpp \
	QVSpace.h \
	QWrap.cpp \
	QWrap.h \
	socket_callback.cpp \
	socket_callback.h \
	TocModel.cpp \
	TocModel.h \
	TocWidget.cpp \
	TocWidget.h \
	UrlView.cpp \
	UrlView.h \
	Validator.cpp \
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

