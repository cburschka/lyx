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
#include "GuiDelimiter.h"
#include "GuiIndex.h"
#include "GuiMathMatrix.h"
#include "GuiNomencl.h"
#include "GuiView.h"

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
Dialog * createGuiMath(LyXView & lv);
Dialog * createGuiNomencl(LyXView & lv);
Dialog * createGuiNote(LyXView & lv);
Dialog * createGuiParagraph(LyXView & lv);
Dialog * createGuiPreferences(LyXView & lv);
Dialog * createGuiPrint(LyXView & lv);
Dialog * createGuiRef(LyXView & lv);
Dialog * createGuiSearch(LyXView & lv);
Dialog * createGuiSendTo(LyXView & lv);
Dialog * createGuiShowFile(LyXView & lv);
Dialog * createGuiSpellchecker(LyXView & lv);
Dialog * createGuiTabularCreate(LyXView & lv);
Dialog * createGuiTabular(LyXView & lv);
Dialog * createGuiTexInfo(LyXView & lv);
Dialog * createGuiToc(LyXView & lv);
Dialog * createGuiThesaurus(LyXView & lv);
Dialog * createGuiURL(LyXView & lv);
Dialog * createGuiVSpace(LyXView & lv);
Dialog * createGuiViewSource(LyXView & lv);
Dialog * createGuiWrap(LyXView & lv);


bool Dialogs::isValidName(string const & name) const
{
	return std::find_if(dialognames, end_dialognames,
			    cmpCStr(name.c_str())) != end_dialognames;
}


Dialog * Dialogs::build(string const & name)
{
	BOOST_ASSERT(isValidName(name));

	if (name == "aboutlyx")
		return createGuiAbout(lyxview_);
	if (name == "bibitem")
		return createGuiBibitem(lyxview_);
	if (name == "bibtex")
		return createGuiBibtex(lyxview_);
	if (name == "box")
		return createGuiBox(lyxview_);
	if (name == "branch")
		return createGuiBranch(lyxview_);
	if (name == "changes")
		return createGuiChanges(lyxview_);
	if (name == "character")
		return createGuiCharacter(lyxview_);
	if (name == "citation")
		return createGuiCitation(lyxview_);
	if (name == "document")
		return createGuiDocument(lyxview_);
	if (name == "embedding")
		return createGuiEmbeddedFiles(lyxview_);
	if (name == "errorlist")
		return createGuiErrorList(lyxview_);
	if (name == "ert")
		return createGuiERT(lyxview_);
	if (name == "external")
		return createGuiExternal(lyxview_);
	if (name == "file")
		return createGuiShowFile(lyxview_);
	if (name == "findreplace")
		return createGuiSearch(lyxview_);
	if (name == "float")
		return createGuiFloat(lyxview_);
	if (name == "graphics")
		return createGuiGraphics(lyxview_);
	if (name == "include")
		return createGuiInclude(lyxview_);
	if (name == "index")
		return new GuiIndexDialog(lyxview_);
	if (name == "nomenclature")
		return new GuiNomenclDialog(lyxview_);
	if (name == "label")
		return new GuiLabelDialog(lyxview_);
	if (name == "log")
		return createGuiLog(lyxview_);
	if (name == "view-source")
		return createGuiViewSource(lyxview_);
	if (name == "mathdelimiter")
		return new GuiDelimiterDialog(lyxview_);
	if (name == "mathmatrix")
		return new GuiMathMatrixDialog(lyxview_);
	if (name == "note")
		return createGuiNote(lyxview_);
	if (name == "paragraph")
		return createGuiParagraph(lyxview_);
	if (name == "prefs")
		return createGuiPreferences(lyxview_);
	if (name == "print")
		return createGuiPrint(lyxview_);
	if (name == "ref")
		return createGuiRef(lyxview_);
	if (name == "sendto")
		return createGuiSendTo(lyxview_);
	if (name == "spellchecker")
		return createGuiSpellchecker(lyxview_);
	if (name == "tabular")
		return createGuiTabular(lyxview_);
	if (name == "tabularcreate")
		return createGuiTabularCreate(lyxview_);
	if (name == "texinfo")
		return createGuiTexInfo(lyxview_);
#ifdef HAVE_LIBAIKSAURUS
	if (name == "thesaurus")
		return createGuiThesaurus(lyxview_);
#endif
	if (name == "toc")
		return createGuiToc(lyxview_);
	if (name == "url")
		return createGuiURL(lyxview_);
	if (name == "vspace")
		return createGuiVSpace(lyxview_);
	if (name == "wrap")
		return createGuiWrap(lyxview_);
	if (name == "listings")
		return createGuiListings(lyxview_);

	return 0;
}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return false;
}

} // namespace frontend
} // namespace lyx
