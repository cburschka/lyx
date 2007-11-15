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
#include "Dialog.h"

#include <boost/assert.hpp>

using std::string;

namespace lyx {

extern bool quitting;

namespace frontend {

Dialogs::Dialogs(LyXView & lyxview)
	: lyxview_(lyxview), in_show_(false)
{}


Dialog * Dialogs::find_or_build(string const & name)
{
	if (!isValidName(name))
		return 0;

	std::map<string, DialogPtr>::iterator it =
		dialogs_.find(name);

	if (it != dialogs_.end())
		return it->second.get();

	dialogs_[name].reset(build(name));
	return dialogs_[name].get();
}


void Dialogs::show(string const & name, string const & data, Inset * inset)
{
	if (in_show_)
		return;

	in_show_ = true;
	Dialog * dialog = find_or_build(name);
	if (dialog) {
		dialog->showData(data);
		if (inset)
			open_insets_[name] = inset;
	}
	in_show_ = false;
}


bool Dialogs::visible(string const & name) const
{
	std::map<string, DialogPtr>::const_iterator it = dialogs_.find(name);
	if (it == dialogs_.end())
		return false;
	return it->second.get()->isVisibleView();
}


void Dialogs::update(string const & name, string const & data)
{
	std::map<string, DialogPtr>::const_iterator it = dialogs_.find(name);
	if (it == dialogs_.end())
		return;

	Dialog * const dialog = it->second.get();
	if (dialog->isVisibleView())
		dialog->updateData(data);
}


void Dialogs::hide(string const & name, Inset* inset)
{
	// Don't send the signal if we are quitting, because on MSVC it is
	// destructed before the cut stack in CutAndPaste.cpp, and this method
	// is called from some inset destructor if the cut stack is not empty
	// on exit.
	if (quitting)
		return;

	std::map<string, DialogPtr>::const_iterator it =
		dialogs_.find(name);
	if (it == dialogs_.end())
		return;

	if (inset && inset != getOpenInset(name))
		return;

	Dialog * const dialog = it->second.get();
	if (dialog->isVisibleView())
		dialog->hide();
	open_insets_[name] = 0;
}


void Dialogs::disconnect(string const & name)
{
	if (!isValidName(name))
		return;

	if (open_insets_.find(name) != open_insets_.end())
		open_insets_[name] = 0;
}


Inset * Dialogs::getOpenInset(string const & name) const
{
	if (!isValidName(name))
		return 0;

	std::map<string, Inset *>::const_iterator it =
		open_insets_.find(name);
	return it == open_insets_.end() ? 0 : it->second;
}


void Dialogs::hideAll() const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it)
		it->second->hide();
}


void Dialogs::hideBufferDependent() const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		Dialog * dialog = it->second.get();
		if (dialog->isBufferDependent())
			dialog->hide();
	}
}


void Dialogs::updateBufferDependent(bool switched) const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		Dialog * dialog = it->second.get();
		if (switched && dialog->isBufferDependent()) {
			if (dialog->isVisibleView() && dialog->initialiseParams(""))
				dialog->updateView();
			else
				dialog->hide();
		} else {
			// A bit clunky, but the dialog will request
			// that the kernel provides it with the necessary
			// data.
			dialog->slotRestore();
		}
	}
}


void Dialogs::redraw() const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it)
		it->second->redraw();
}


void Dialogs::checkStatus()
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		Dialog * const dialog = it->second.get();
		if (dialog && dialog->isVisibleView())
			dialog->checkStatus();
	}
}


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

"texinfo", "toc", "href", "view-source", "vspace", "wrap", "listings" };

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
Dialog * createGuiNomenclature(LyXView & lv);
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
Dialog * createGuiHyperlink(LyXView & lv);
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
		return createGuiIndex(lyxview_);
	if (name == "nomenclature")
		return createGuiNomenclature(lyxview_);
	if (name == "label")
		return createGuiLabel(lyxview_);
	if (name == "log")
		return createGuiLog(lyxview_);
	if (name == "view-source")
		return createGuiViewSource(lyxview_);
	if (name == "mathdelimiter")
		return createGuiDelimiter(lyxview_);
	if (name == "mathmatrix")
		return createGuiMathMatrix(lyxview_);
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
	if (name == "href")
		return createGuiHyperlink(lyxview_);
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
