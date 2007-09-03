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
	Action.cpp \
	alert_pimpl.cpp \
	BulletsModule.cpp \
	ButtonController.cpp \
	ColorCache.cpp \
	Dialogs.cpp \
	EmptyTable.cpp \
	FileDialog.cpp \
	FloatPlacement.cpp \
	GuiAbout.cpp \
	GuiApplication.cpp \
	GuiBibitem.cpp \
	GuiBibtex.cpp \
	GuiBox.cpp \
	GuiBranch.cpp \
	GuiBranches.cpp \
	GuiChanges.cpp \
	GuiCharacter.cpp \
	GuiCitation.cpp \
	GuiClipboard.cpp \
	GuiCommandBuffer.cpp \
	GuiCommandEdit.cpp \
	GuiDelimiter.cpp \
	GuiDialog.cpp \
	GuiDialogView.cpp \
	GuiDocument.cpp \
	GuiEmbeddedFiles.cpp \
	GuiErrorList.cpp \
	GuiERT.cpp \
	GuiExternal.cpp \
	GuiFloat.cpp \
	GuiFontExample.cpp \
	GuiFontLoader.cpp \
	GuiFontMetrics.cpp \
	GuiGraphics.cpp \
	GuiImage.cpp \
	GuiImplementation.cpp \
	GuiInclude.cpp \
	GuiIndex.cpp \
	GuiKeySymbol.cpp \
	GuiKeySymbol.cpp \
	GuiListings.cpp \
	GuiLog.cpp \
	GuiMathMatrix.cpp \
	GuiMenubar.cpp \
	GuiNomencl.cpp \
	GuiNote.cpp \
	GuiPainter.cpp \
	GuiParagraph.cpp \
	GuiPopupMenu.cpp \
	GuiPrefs.cpp \
	GuiPrint.cpp \
	GuiRef.cpp \
	GuiSearch.cpp \
	GuiSelection.cpp \
	GuiSelectionManager.cpp \
	GuiSendto.cpp \
	GuiSetBorder.cpp \
	GuiShowFile.cpp \
	GuiSpellchecker.cpp \
	GuiTabular.cpp \
	GuiTabularCreate.cpp \
	GuiTexinfo.cpp \
	GuiThesaurus.cpp \
	GuiToc.cpp \
	GuiToolbar.cpp \
	GuiURL.cpp \
	GuiView.cpp \
	GuiViewSource.cpp \
	GuiVSpace.cpp \
	GuiWorkArea.cpp \
	GuiWrap.cpp \
	IconPalette.cpp \
	InsertTableWidget.cpp \
	LengthCombo.cpp \
	LyXFileDialog.cpp \
	PanelStack.cpp \
	qt_helpers.cpp \
	socket_callback.cpp \
	TocModel.cpp \
	TocWidget.cpp \
	Validator.cpp 

NOMOCHEADER = \
	ButtonController.h \
	GuiClipboard.h \
	GuiFontLoader.h \
	GuiFontMetrics.h \
	GuiSelection.h \
	GuiImage.h \
	GuiPainter.h \
	qt_helpers.h

MOCHEADER = \
	Action.h \
	BulletsModule.h \
	ColorCache.h \
	DockView.h \
	EmptyTable.h \
	FloatPlacement.h \
	GuiAbout.h \
	GuiApplication.h \
	GuiBibitem.h \
	GuiBibtex.h \
	GuiBox.h \
	GuiBranches.h \
	GuiBranch.h \
	GuiChanges.h \
	GuiCharacter.h \
	GuiCitation.h \
	GuiCommandBuffer.h \
	GuiCommandEdit.h \
	GuiDelimiter.h \
	GuiDialog.h \
	GuiDialogView.h \
	GuiDocument.h \
	GuiEmbeddedFiles.h \
	GuiErrorList.h \
	GuiERT.h \
	GuiExternal.h \
	GuiFloat.h \
	GuiFontExample.h \
	GuiGraphics.h \
	GuiImplementation.h \
	GuiInclude.h \
	GuiIndex.h \
	GuiKeySymbol.h \
	GuiListings.h \
	GuiLog.h \
	GuiMathMatrix.h \
	GuiMenubar.h \
	GuiNomencl.h \
	GuiNote.h \
	GuiParagraph.h \
	GuiPopupMenu.h \
	GuiPrefs.h \
	GuiPrint.h \
	GuiRef.h \
	GuiSearch.h \
	GuiSelectionManager.h \
	GuiSendto.h \
	GuiSetBorder.h \
	GuiShowFile.h \
	GuiSpellchecker.h \
	GuiTabularCreate.h \
	GuiTabular.h \
	GuiTexinfo.h \
	GuiThesaurus.h \
	GuiToc.h \
	GuiToolbar.h \
	GuiURL.h \
	GuiView.h \
	GuiViewSource.h \
	GuiVSpace.h \
	GuiWorkArea.h \
	GuiWrap.h \
	IconPalette.h \
	InsertTableWidget.h \
	LengthCombo.h \
	LyXFileDialog.h \
	PanelStack.h \
	qlkey.h \
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
	EmbeddedFilesUi.ui \
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

liblyxqt4.cpp:
	touch $@

liblyxqt4_la_SOURCES = \
	$(SOURCEFILES) \
	$(MOCHEADER) \
	$(NOMOCHEADER)

EXTRA_DIST = pch.h
BUILT_SOURCES += $(PCH_FILE)
AM_CPPFLAGS += $(PCH_FLAGS)


endif
