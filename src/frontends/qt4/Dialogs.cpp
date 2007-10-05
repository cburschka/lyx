/**
 * \file qt4/Dialogs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Dialogs.h"
#include "GuiDialog.h"

#include "ButtonController.h"
#include "DialogView.h"
#include "DockView.h"
#include "GuiAbout.h"
#include "GuiBibitem.h"
#include "GuiBibtex.h"
#include "GuiBox.h"
#include "GuiBranch.h"
#include "GuiChanges.h"
#include "GuiCharacter.h"
#include "GuiCitation.h"
#include "GuiDelimiter.h"
#include "GuiDocument.h"
#include "GuiEmbeddedFiles.h"
#include "GuiErrorList.h"
#include "GuiERT.h"
#include "GuiExternal.h"
#include "GuiFloat.h"
#include "GuiGraphics.h"
#include "GuiInclude.h"
#include "GuiIndex.h"
#include "GuiMathMatrix.h"
#include "GuiNomencl.h"
#include "GuiListings.h"
#include "GuiLog.h"
#include "GuiNote.h"
#include "GuiParagraph.h"
#include "GuiPrefs.h"
#include "GuiPrint.h"
#include "GuiSearch.h"
#include "GuiSendto.h"
#include "GuiShowFile.h"
#include "GuiSpellchecker.h"
#include "GuiTabular.h"
#include "GuiTabularCreate.h"
#include "GuiTexinfo.h"
#include "GuiToc.h"
#include "GuiView.h"
#include "GuiViewSource.h"
#include "TocWidget.h"
#include "GuiURL.h"
#include "GuiVSpace.h"
#include "GuiWrap.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "GuiThesaurus.h"
#endif

// Uncomment this if you prefer dock widget
//#define USE_DOCK_WIDGET

#include "qt_helpers.h"

#include <boost/assert.hpp>

using std::string;

namespace lyx {
namespace frontend {

namespace {

// This list should be kept in sync with the list of insets in
// src/insets/Inset.cpp.  I.e., if a dialog goes with an inset, the
// dialog should have the same name as the inset.

char const * const dialognames[] = {
"aboutlyx", "bibitem", "bibtex", "box", "branch", "changes", "character",
"citation", "document", "embedding", "errorlist", "ert", "external", "file",
"findreplace", "float", "graphics", "include", "index", "nomenclature", "label", "log",
"mathdelimiter", "mathmatrix", "note", "paragraph",
"prefs", "print", "ref", "sendto", "spellchecker","tabular", "tabularcreate",

#ifdef HAVE_LIBAIKSAURUS
"thesaurus",
#endif

"texinfo", "toc", "url", "view-source", "vspace", "wrap", "listings" };

char const * const * const end_dialognames =
	dialognames + (sizeof(dialognames) / sizeof(char *));

class cmpCStr {
public:
	cmpCStr(char const * name) : name_(name) {}
	bool operator()(char const * other) {
		return strcmp(other, name_) == 0;
	}
private:
	char const * name_;
};


} // namespace anon


Dialog * createGuiRef(LyXView & lv);


bool Dialogs::isValidName(string const & name) const
{
	return std::find_if(dialognames, end_dialognames,
			    cmpCStr(name.c_str())) != end_dialognames;
}


Dialog * Dialogs::build(string const & name)
{
	BOOST_ASSERT(isValidName(name));

	Dialog * dialog = 0;
	GuiViewBase & guiview = static_cast<GuiViewBase &>(lyxview_);

	if (name == "aboutlyx") {
		dialog = new GuiAboutDialog(lyxview_);
	} else if (name == "bibitem") {
		dialog = new GuiBibitemDialog(lyxview_);
	} else if (name == "bibtex") {
		dialog = new GuiBibtexDialog(lyxview_);
	} else if (name == "box") {
		dialog = new GuiBoxDialog(lyxview_);
	} else if (name == "branch") {
		dialog = new GuiBranchDialog(lyxview_);
	} else if (name == "changes") {
		dialog = new GuiChangesDialog(lyxview_);
	} else if (name == "character") {
		dialog = new GuiCharacterDialog(lyxview_);
	} else if (name == "citation") {
		dialog = new GuiCitationDialog(lyxview_);
	} else if (name == "document") {
		dialog = new GuiDocumentDialog(lyxview_);
	} else if (name == "embedding") {
		dialog = new DockView<ControlEmbeddedFiles, GuiEmbeddedFilesDialog>(
			guiview, name, Qt::RightDockWidgetArea);
	} else if (name == "errorlist") {
		dialog = new GuiErrorListDialog(lyxview_);
	} else if (name == "ert") {
		dialog = new GuiERTDialog(lyxview_);
	} else if (name == "external") {
		dialog = new GuiExternalDialog(lyxview_);
	} else if (name == "file") {
		dialog = new GuiShowFileDialog(lyxview_);
	} else if (name == "findreplace") {
		dialog = new GuiSearchDialog(lyxview_);
	} else if (name == "float") {
		dialog = new GuiFloatDialog(lyxview_);
	} else if (name == "graphics") {
		dialog = new GuiGraphicsDialog(lyxview_);
	} else if (name == "include") {
		dialog = new GuiIncludeDialog(lyxview_);
	} else if (name == "index") {
		dialog = new GuiIndexDialog(lyxview_);
	} else if (name == "nomenclature") {
		dialog = new GuiNomenclDialog(lyxview_);
	} else if (name == "label") {
		dialog = new GuiLabelDialog(lyxview_);
	} else if (name == "log") {
		dialog = new GuiLogDialog(lyxview_);
	} else if (name == "view-source") {
		dialog = new DockView<ControlViewSource, GuiViewSourceDialog>(
			guiview, name, Qt::BottomDockWidgetArea);
	} else if (name == "mathdelimiter") {
		dialog = new GuiDelimiterDialog(lyxview_);
	} else if (name == "mathmatrix") {
		dialog = new GuiMathMatrixDialog(lyxview_);
	} else if (name == "note") {
		dialog = new GuiNoteDialog(lyxview_);
	} else if (name == "paragraph") {
#ifdef USE_DOCK_WIDGET
		DockView<ControlParagraph, GuiParagraph> * dv =
			new DockView<ControlParagraph, GuiParagraph>(guiview, name,
				Qt::TopDockWidgetArea);
#else
		DialogView<ControlParagraph, GuiParagraph> * dv =
			new DialogView<ControlParagraph, GuiParagraph>(guiview, name);
#endif
		dialog = dv;
	} else if (name == "prefs") {
		dialog = new GuiPrefsDialog(lyxview_);
	} else if (name == "print") {
		dialog = new GuiPrintDialog(lyxview_);
	} else if (name == "ref") {
		dialog = createGuiRef(lyxview_);
	} else if (name == "sendto") {
		dialog = new GuiSendtoDialog(lyxview_);
	} else if (name == "spellchecker") {
		dialog = new GuiSpellcheckerDialog(lyxview_);
	} else if (name == "tabular") {
		dialog = new GuiTabularDialog(lyxview_);
	} else if (name == "tabularcreate") {
		dialog = new GuiTabularCreateDialog(lyxview_);
	} else if (name == "texinfo") {
		dialog = new GuiTexinfoDialog(lyxview_);
#ifdef HAVE_LIBAIKSAURUS
	} else if (name == "thesaurus") {
		dialog = new GuiThesaurusDialog(lyxview_);
#endif
	} else if (name == "toc") {
#ifdef Q_WS_MACX
		// On Mac show as a drawer at the right
		dialog = new DockView<GuiToc, TocWidget>(guiview, name,
			Qt::RightDockWidgetArea, Qt::Drawer);
#else
		dialog = new DockView<GuiToc, TocWidget>(guiview, name);
#endif
	} else if (name == "url") {
		dialog = new GuiURLDialog(lyxview_);
	} else if (name == "vspace") {
		dialog = new GuiVSpaceDialog(lyxview_);
	} else if (name == "wrap") {
		dialog = new GuiWrapDialog(lyxview_);
	} else if (name == "listings") {
		dialog = new GuiListingsDialog(lyxview_);
	}

	return dialog;
}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return false;
}

} // namespace frontend
} // namespace lyx
