project(frontend_qt4)

set(frontends_qt4_dialog_sources
	BulletsModule.C
	emptytable.C
	FileDialog_private.C
	floatplacement.C
	iconpalette.C
	InsertTableWidget.C
	lengthcombo.C
	QAboutDialog.C
	QBibitemDialog.C
	QBibtexDialog.C
	QBoxDialog.C
	QBranchDialog.C
	QChangesDialog.C
	QCharacterDialog.C
	QCitationDialog.C
	QCommandBuffer.C
	QCommandEdit.C
	QDelimiterDialog.C
	QDocumentDialog.C
	QErrorListDialog.C
	QERTDialog.C
	QExternalDialog.C
	QFloatDialog.C
	QGraphicsDialog.C
	QIncludeDialog.C
	QIndexDialog.C
	QLogDialog.C
	QViewSourceDialog.C
	QLPopupMenu.C
	QLPrintDialog.C
	QMathDialog.C
	QMathMatrixDialog.C
	QNoteDialog.C
	QParagraphDialog.C
	QPrefsDialog.C
	QRefDialog.C
	QSearchDialog.C
	QSendtoDialog.C
	qsetborder.C
	QShowFileDialog.C
	QSpellcheckerDialog.C
	QDialogView.C
	QTabularCreateDialog.C
	QTabularDialog.C
	QTexinfoDialog.C
	QThesaurusDialog.C
	TocModel.C
	QTocDialog.C
	QtView.C
	QURLDialog.C
	QVSpaceDialog.C
	QWrapDialog.C
	QLToolbar.C
	socket_callback.C
	validators.C
	#Profile.C
)

set(frontends_qt4_sources
	Alert_pimpl.C
	Dialogs.C
	FileDialog.C
	LyXKeySymFactory.C
	LyXScreenFactory.C
	QLMenubar.C 
	qtTimeout.C 
	QAbout.C 
	QBibitem.C 
	QBibtex.C 
	QBox.C 
	QBranch.C 
	QBranches.C 
	QChanges.C 
	QCharacter.C 
	QCitation.C 
	QDocument.C 
	QErrorList.C 
	QERT.C 
	QExternal.C 
	QFloat.C 
	QGraphics.C 
	QInclude.C 
	QIndex.C 
	QLAction.C 
	QLImage.C 
	QLog.C 
	QViewSource.C 
	QLPainter.C 
	QLyXKeySym.C 
	QMath.C 
	QNote.C 
	QParagraph.C
	QPrefs.C 
	QPrint.C 
	QRef.C 
	QSearch.C 
	QSendto.C 
	QShowFile.C 
	QSpellchecker.C
	QTabular.C 
	QTabularCreate.C 
	QTexinfo.C 
	QThesaurus.C 
	QToc.C 
	QURL.C 
	QVSpace.C 
	QWorkArea.C 
	QWrap.C 
	Qt2BC.C	
	WorkAreaFactory.C
	checkedwidgets.C 
	lyx_gui.C
	lcolorcache.C
	panelstack.C
	qfontexample.C
	qfont_loader.C
	qfont_metrics.C
	qt_helpers.C
	${frontends_qt4_dialog_sources}
)	

set(frontend_qt4_UI
	ui/BiblioUi.ui
	ui/BranchesUi.ui
	ui/BulletsUi.ui
	ui/LaTeXUi.ui
	ui/LanguageUi.ui
	ui/MarginsUi.ui
	ui/MathsUi.ui
	ui/NumberingUi.ui
	ui/PageLayoutUi.ui
	ui/PreambleUi.ui
	ui/QAboutUi.ui
	ui/QAskForTextUi.ui
	ui/QBibitemUi.ui
	ui/QBibtexAddUi.ui
	ui/QBibtexUi.ui
	ui/QBoxUi.ui
	ui/QBranchUi.ui
	ui/QChangesUi.ui
	ui/QCharacterUi.ui
	ui/QCitationFindUi.ui
	ui/QCitationUi.ui
	ui/QDelimiterUi.ui
	ui/QDocumentUi.ui
	ui/QERTUi.ui
	ui/QErrorListUi.ui
	ui/QExternalUi.ui
	ui/QFloatUi.ui
	ui/QGraphicsUi.ui
	ui/QIncludeUi.ui
	ui/QIndexUi.ui
	ui/QLogUi.ui
	ui/QMathMatrixUi.ui
	ui/QMathUi.ui
	ui/QNoteUi.ui
	ui/QParagraphUi.ui
	ui/QPrefAsciiUi.ui
	ui/QPrefColorsUi.ui
	ui/QPrefConvertersUi.ui
	ui/QPrefCopiersUi.ui
	ui/QPrefCygwinPathUi.ui
	ui/QPrefDateUi.ui
	ui/QPrefDisplayUi.ui
	ui/QPrefFileformatsUi.ui
	ui/QPrefIdentityUi.ui
	ui/QPrefKeyboardUi.ui
	ui/QPrefLanguageUi.ui
	ui/QPrefLatexUi.ui
	ui/QPrefPathsUi.ui
	ui/QPrefPrinterUi.ui
	ui/QPrefScreenFontsUi.ui
	ui/QPrefSpellcheckerUi.ui
	ui/QPrefUi.ui
	ui/QPrefsUi.ui
	ui/QPrintUi.ui
	ui/QRefUi.ui
	ui/QSearchUi.ui
	ui/QSendtoUi.ui
	ui/QShowFileUi.ui
	ui/QSpellcheckerUi.ui
	ui/QTabularCreateUi.ui
	ui/QTabularUi.ui
	ui/QTexinfoUi.ui
	ui/QThesaurusUi.ui
	ui/QTocUi.ui
	ui/QURLUi.ui
	ui/QVSpaceUi.ui
	ui/QViewSourceUi.ui
	ui/QWrapUi.ui
	ui/TextLayoutUi.ui
)


lyx_add_path(frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4)
lyx_add_path(frontend_qt4_UI ${TOP_SRC_DIR}/src/frontends/qt4)

lyx_automoc(${frontends_qt4_sources})

lyx_add_qt4_ui_files(frontends_qt4_sources ${frontend_qt4_UI})

add_definitions(-DQT3_SUPPORT -DQT_NO_STL)

include_directories(	${TOP_SRC_DIR}/src/frontends/qt4 
			${TOP_SRC_DIR}/src/frontends/controllers
			${CMAKE_CURRENT_BINARY_DIR})

add_library(frontend_qt4 STATIC ${frontends_qt4_sources})

target_link_libraries(frontend_qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} controllers)

if(WIN32)
	target_link_libraries(frontend_qt4 Gdi32)
endif(WIN32)

