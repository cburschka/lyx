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
#include "GuiBibitem.h"
#include "GuiDelimiter.h"
#include "GuiDocument.h"
#include "GuiExternal.h"
#include "GuiGraphics.h"
#include "GuiIndex.h"
#include "GuiMathMatrix.h"
#include "GuiNomencl.h"
#include "GuiParagraph.h"
#include "GuiPrefs.h"
#include "GuiPrint.h"
#include "GuiSearch.h"
#include "GuiSendto.h"
#include "GuiShowFile.h"
#include "GuiSpellchecker.h"
#include "GuiToc.h"
#include "GuiView.h"
#include "GuiViewSource.h"
#include "TocWidget.h"
#include "GuiURL.h"

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

// will be replaced by a proper factory...
Dialog * createGuiAbout(LyXView & lv);
Dialog * createGuiBibitem(LyXView & lv);
Dialog * createGuiBibtex(LyXView & lv);
Dialog * createGuiBox(LyXView & lv);
Dialog * createGuiBranch(LyXView & lv);
Dialog * createGuiChanges(LyXView & lv);
Dialog * createGuiCharacter(LyXView & lv);
Dialog * createGuiCitation(LyXView & lv);
Dialog * createGuiDelimiter(LyXView & lv);
Dialog * createGuiDocument(LyXView & lv);
Dialog * createGuiEmbeddedFiles(LyXView & lv);
Dialog * createGuiErrorList(LyXView & lv);
Dialog * createGuiERT(LyXView & lv);
Dialog * createGuiExternal(LyXView & lv);
Dialog * createGuiFloat(LyXView & lv);
Dialog * createGuiGraphics(LyXView & lv);
Dialog * createGuiInclude(LyXView & lv);
Dialog * createGuiIndex(LyXView & lv);
Dialog * createGuiLabel(LyXView & lv);
Dialog * createGuiListings(LyXView & lv);
Dialog * createGuiLog(LyXView & lv);
Dialog * createGuiMathMatrix(LyXView & lv);
Dialog * createGuiNomencl(LyXView & lv);
Dialog * createGuiNote(LyXView & lv);
Dialog * createGuiPrefs(LyXView & lv);
Dialog * createGuiPrint(LyXView & lv);
Dialog * createGuiRef(LyXView & lv);
Dialog * createGuiSearch(LyXView & lv);
Dialog * createGuiSendto(LyXView & lv);
Dialog * createGuiShowFile(LyXView & lv);
Dialog * createGuiSpellchecker(LyXView & lv);
Dialog * createGuiTabularCreate(LyXView & lv);
Dialog * createGuiTabular(LyXView & lv);
Dialog * createGuiTexInfo(LyXView & lv);
Dialog * createGuiThesaurus(LyXView & lv);
Dialog * createGuiURL(LyXView & lv);
Dialog * createGuiVSpace(LyXView & lv);
Dialog * createGuiWrap(LyXView & lv);


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
		dialog = createGuiAbout(lyxview_);
	} else if (name == "bibitem") {
		dialog = new GuiBibitemDialog(lyxview_);
	} else if (name == "bibtex") {
		dialog = createGuiBibtex(lyxview_);
	} else if (name == "box") {
		dialog = createGuiBox(lyxview_);
	} else if (name == "branch") {
		dialog = createGuiBranch(lyxview_);
	} else if (name == "changes") {
		dialog = createGuiChanges(lyxview_);
	} else if (name == "character") {
		dialog = createGuiCharacter(lyxview_);
	} else if (name == "citation") {
		dialog = createGuiCitation(lyxview_);
	} else if (name == "document") {
		dialog = new GuiDocumentDialog(lyxview_);
	} else if (name == "embedding") {
		dialog = createGuiEmbeddedFiles(lyxview_);
	} else if (name == "errorlist") {
		dialog = createGuiErrorList(lyxview_);
	} else if (name == "ert") {
		dialog = createGuiERT(lyxview_);
	} else if (name == "external") {
		dialog = new GuiExternalDialog(lyxview_);
	} else if (name == "file") {
		dialog = createGuiShowFile(lyxview_);
	} else if (name == "findreplace") {
		dialog = new GuiSearchDialog(lyxview_);
	} else if (name == "float") {
		dialog = createGuiFloat(lyxview_);
	} else if (name == "graphics") {
		dialog = new GuiGraphicsDialog(lyxview_);
	} else if (name == "include") {
		dialog = createGuiInclude(lyxview_);
	} else if (name == "index") {
		dialog = new GuiIndexDialog(lyxview_);
	} else if (name == "nomenclature") {
		dialog = new GuiNomenclDialog(lyxview_);
	} else if (name == "label") {
		dialog = new GuiLabelDialog(lyxview_);
	} else if (name == "log") {
		createGuiLog(lyxview_);
	} else if (name == "view-source") {
		dialog = new DockView<ControlViewSource, GuiViewSourceDialog>(
			guiview, name, Qt::BottomDockWidgetArea);
	} else if (name == "mathdelimiter") {
		dialog = new GuiDelimiterDialog(lyxview_);
	} else if (name == "mathmatrix") {
		dialog = new GuiMathMatrixDialog(lyxview_);
	} else if (name == "note") {
		dialog = createGuiNote(lyxview_);
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
		dialog = createGuiTabular(lyxview_);
	} else if (name == "tabularcreate") {
		dialog = createGuiTabularCreate(lyxview_);
	} else if (name == "texinfo") {
		dialog = createGuiTexInfo(lyxview_);
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
		dialog = createGuiVSpace(lyxview_);
	} else if (name == "wrap") {
		dialog = createGuiWrap(lyxview_);
	} else if (name == "listings") {
		dialog = createGuiListings(lyxview_);
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
