include $(top_srcdir)/config/common.am

BUILT_SOURCES = $(UIFILES:%.ui=ui_%.h)
BUILT_SOURCES += $(MOCEDFILES)
BUILT_SOURCES += Resources.cpp Resources.qrc

CLEANFILES = $(BUILT_SOURCES)

#########################  Qt stuff  #############################
# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr lyx::qt_

# The ui_%.h pattern must match the filter in ../../../po/Rules-lyx
ui_%.h: ui/%.ui
	$(AM_V_GEN)$(QT_UIC) $(UICFLAGS) $< -o $@

MOCEDFILES = $(MOCHEADER:%.h=moc_%.cpp)

QT_VERSION = $(shell IFS=.; set -- `echo $(QTLIB_VERSION)`; \
	     echo 0x0`echo "obase=16; $$1*65536+$$2*256+$$3" | bc`)

# The moc_%.cpp pattern must match the filter in ../../../po/Rules-lyx
moc_%.cpp: %.h
	$(AM_V_GEN)$(QT_MOC) -DQT_VERSION=$(QT_VERSION) -o $@ $<

Resources.qrc: Makefile
	$(AM_V_GEN)echo "<!DOCTYPE RCC><RCC version='1.0'><qresource>" > $@ ; \
	find $(top_srcdir)/lib/images -name '*.svgz' -o -name '*.png' -o -name '*.gif' \
		| sed -e 's:$(top_srcdir)/lib/\(.*\):<file alias="\1">&</file>:' \
		>> $@ ;\
	echo "</qresource></RCC>" >> $@

Resources.cpp: Resources.qrc
	$(AM_V_GEN)$(QT_RCC) $< -name Resources -o $@


#########################  LIBRARIES  #############################

noinst_LIBRARIES = liblyxqt4.a

liblyxqt4_a_DEPENDENCIES = $(MOCEDFILES)

AM_CPPFLAGS += \
	$(QT_CPPFLAGS) \
	-DQT_NO_CAST_TO_ASCII \
	-DQT_NO_STL \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/images \
	$(QT_INCLUDES) $(BOOST_INCLUDES)

SOURCEFILES = \
	ButtonPolicy.cpp \
	ButtonPolicy.h \
	Dialog.cpp \
	Dialog.h \
	Action.cpp \
	BulletsModule.cpp \
	ButtonController.cpp \
	CategorizedCombo.cpp \
	ColorCache.cpp \
	CustomizedWidgets.cpp \
	EmptyTable.cpp \
	FancyLineEdit.cpp \
	FileDialog.cpp \
	FindAndReplace.cpp \
	FloatPlacement.cpp \
	GuiAbout.cpp \
	GuiAlert.cpp \
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
	GuiCompare.cpp \
	GuiCompareHistory.cpp \
	GuiCompleter.cpp \
	GuiDelimiter.cpp \
	GuiDialog.cpp \
	GuiDocument.cpp \
	GuiErrorList.cpp \
	GuiERT.cpp \
	GuiExternal.cpp \
	GuiFontExample.cpp \
	GuiFontLoader.cpp \
	GuiFontMetrics.cpp \
	GuiGraphics.cpp \
	GuiHSpace.cpp \
	GuiHyperlink.cpp \
	GuiIdListModel.cpp \
	GuiImage.cpp \
	GuiInclude.cpp \
	GuiIndex.cpp \
	GuiIndices.cpp \
	GuiInfo.cpp \
	GuiKeySymbol.cpp \
	GuiLabel.cpp \
	GuiLine.cpp \
	GuiListings.cpp \
	GuiLog.cpp \
	GuiMathMatrix.cpp \
	GuiNomenclature.cpp \
	GuiNote.cpp \
	GuiPainter.cpp \
	GuiParagraph.cpp \
	GuiPhantom.cpp \
	GuiPrefs.cpp \
	GuiPrintindex.cpp \
	GuiPrintNomencl.cpp \
	GuiProgress.cpp \
	GuiProgressView.cpp \
	GuiRef.cpp \
	GuiSearch.cpp \
	GuiSelection.cpp \
	GuiSelectionManager.cpp \
	GuiSendto.cpp \
	GuiSetBorder.cpp \
	GuiShowFile.cpp \
	GuiSpellchecker.cpp \
	GuiSymbols.cpp \
	GuiTabular.cpp \
	GuiTabularCreate.cpp \
	GuiTexinfo.cpp \
	GuiThesaurus.cpp \
	GuiToc.cpp \
	GuiToolbar.cpp \
	GuiView.cpp \
	GuiViewSource.cpp \
	GuiVSpace.cpp \
	GuiWorkArea.cpp \
	GuiWrap.cpp \
	IconPalette.cpp \
	InGuiThread.cpp \
	InsertTableWidget.cpp \
	InsetParamsDialog.cpp \
	InsetParamsWidget.cpp \
	LengthCombo.cpp \
	LyXFileDialog.cpp \
	LaTeXHighlighter.cpp \
	LayoutBox.cpp \
	Menus.cpp \
	PanelStack.cpp \
	qt_helpers.cpp \
	TocModel.cpp \
	TocWidget.cpp \
	Toolbars.cpp \
	Validator.cpp 

NOMOCHEADER = \
	ButtonController.h \
	ColorCache.h \
	DialogView.h \
	DockView.h \
	FileDialog.h \
	GuiFontExample.h \
	GuiFontLoader.h \
	GuiFontMetrics.h \
	GuiIdListModel.h \
	GuiImage.h \
	GuiKeySymbol.h \
	GuiPainter.h \
	GuiWorkArea_Private.h \
	LaTeXHighlighter.h \
	qt_i18n.h \
	qt_helpers.h \
	Toolbars.h

MOCHEADER = \
	Action.h \
	BulletsModule.h \
	CategorizedCombo.h \
	CustomizedWidgets.h \
	EmptyTable.h \
	FancyLineEdit.h \
	FindAndReplace.h \
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
	GuiClipboard.h \
	GuiCommandBuffer.h \
	GuiCommandEdit.h \
	GuiCompare.h \
	GuiCompareHistory.h \
	GuiCompleter.h \
	GuiDelimiter.h \
	GuiDialog.h \
	GuiDocument.h \
	GuiErrorList.h \
	GuiERT.h \
	GuiExternal.h \
	GuiGraphics.h \
	GuiHSpace.h \
	GuiHyperlink.h \
	GuiInclude.h \
	GuiIndex.h \
	GuiIndices.h \
	GuiInfo.h \
	GuiLabel.h \
	GuiLine.h \
	GuiListings.h \
	GuiLog.h \
	GuiMathMatrix.h \
	GuiNomenclature.h \
	GuiNote.h \
	GuiParagraph.h \
	GuiPhantom.h \
	GuiPrefs.h \
	GuiPrintindex.h \
	GuiPrintNomencl.h \
	GuiProgress.h \
	GuiProgressView.h \
	GuiRef.h \
	GuiSearch.h \
	GuiSelection.h \
	GuiSelectionManager.h \
	GuiSendto.h \
	GuiSetBorder.h \
	GuiShowFile.h \
	GuiSpellchecker.h \
	GuiSymbols.h \
	GuiTabularCreate.h \
	GuiTabular.h \
	GuiTexinfo.h \
	GuiThesaurus.h \
	GuiToc.h \
	GuiToolbar.h \
	GuiView.h \
	GuiViewSource.h \
	GuiVSpace.h \
	GuiWorkArea.h \
	GuiWrap.h \
	IconPalette.h \
	InGuiThread.h \
	InsertTableWidget.h \
	InsetParamsDialog.h \
	InsetParamsWidget.h \
	LayoutBox.h \
	LengthCombo.h \
	LyXFileDialog.h \
	Menus.h \
	PanelStack.h \
	TocModel.h \
	TocWidget.h \
	Validator.h

UIFILES = \
	AboutUi.ui \
	BibitemUi.ui \
	BiblioUi.ui \
	BibtexAddUi.ui \
	BibtexUi.ui \
	BoxUi.ui \
	BranchesUi.ui \
	BranchesUnknownUi.ui \
	BranchUi.ui \
	BulletsUi.ui \
	ChangesUi.ui \
	CharacterUi.ui \
	CitationUi.ui \
	ColorUi.ui \
	CompareUi.ui \
	CompareHistoryUi.ui \
	DelimiterUi.ui \
	DocumentUi.ui \
	ErrorListUi.ui \
	ERTUi.ui \
	ExternalUi.ui \
	FindAndReplaceUi.ui \
	FloatPlacementUi.ui \
	FontUi.ui \
	GraphicsUi.ui \
	HSpaceUi.ui \
	HyperlinkUi.ui \
	IncludeUi.ui \
	IndexUi.ui \
	IndicesUi.ui \
	InfoUi.ui \
	InsetParamsUi.ui \
	LabelUi.ui \
	LanguageUi.ui \
	LaTeXUi.ui \
	LineUi.ui \
	ListingsUi.ui \
	ListingsSettingsUi.ui \
	LocalLayoutUi.ui \
	LogUi.ui \
	MarginsUi.ui \
	MasterChildUi.ui \
	MathMatrixUi.ui \
	MathsUi.ui \
	ModulesUi.ui \
	NomenclUi.ui \
	NoteUi.ui \
	NumberingUi.ui \
	OutputUi.ui \
	PageLayoutUi.ui \
	ParagraphUi.ui \
	PDFSupportUi.ui \
	PhantomUi.ui \
	PreambleUi.ui \
	PrefColorsUi.ui \
	PrefCompletionUi.ui \
	PrefConvertersUi.ui \
	PrefDocHandlingUi.ui \
	PrefOutputUi.ui \
	PrefDisplayUi.ui \
	PrefEditUi.ui \
	PrefFileformatsUi.ui \
	PrefIdentityUi.ui \
	PrefInputUi.ui \
	PrefLanguageUi.ui \
	PrefLatexUi.ui \
	PrefPathsUi.ui \
	PrefScreenFontsUi.ui \
	PrefShortcutsUi.ui \
	PrefSpellcheckerUi.ui \
	PrefsUi.ui \
	PrefUi.ui \
	PrintindexUi.ui \
	PrintNomenclUi.ui \
	ProgressViewUi.ui \
	RefUi.ui \
	SearchUi.ui \
	SendtoUi.ui \
	ShortcutUi.ui \
	ShowFileUi.ui \
	SpellcheckerUi.ui \
	SymbolsUi.ui \
	TabularCreateUi.ui \
	TabularUi.ui \
	TexinfoUi.ui \
	TextLayoutUi.ui \
	ThesaurusUi.ui \
	TocUi.ui \
	ToggleWarningUi.ui \
	ViewSourceUi.ui \
	VSpaceUi.ui \
	WrapUi.ui

nodist_liblyxqt4_a_SOURCES = Resources.cpp

liblyxqt4.cpp:
	$(AM_V_GEN)for file in $(SOURCEFILES) ; do echo '#include "'$${file}'"' ; done >$@

if MONOLITHIC_FRONTEND_QT

liblyxqt4_a_SOURCES = \
	liblyxqt4.cpp \
	$(MOCHEADER) \
	$(NOMOCHEADER)

BUILT_SOURCES += liblyxqt4.cpp
CLEANFILES += liblyxqt4.cpp

else

liblyxqt4_a_SOURCES = \
	$(SOURCEFILES) \
	$(MOCHEADER) \
	$(NOMOCHEADER)

endif

EXTRA_DIST = CMakeLists.txt $(UIFILES:%=ui/%)
