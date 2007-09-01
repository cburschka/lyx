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

#include "Qt2BC.h"
#include "ButtonController.h"
#include "DockView.h"
#include "GuiView.h"
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
#include "GuiViewSource.h"
#include "GuiNote.h"
#include "GuiParagraph.h"
#include "GuiPrefs.h"
#include "GuiPrint.h"
#include "GuiRef.h"
#include "GuiSearch.h"
#include "GuiSendto.h"
#include "GuiShowFile.h"
#include "GuiSpellchecker.h"
#include "GuiTabular.h"
#include "GuiTabularCreate.h"
#include "GuiTexinfo.h"
#include "GuiToc.h"
#include "TocWidget.h"
#include "GuiURL.h"
#include "GuiVSpace.h"
#include "GuiWrap.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "GuiThesaurus.h"
#endif

#include "qt_helpers.h"

#include <boost/assert.hpp>

using std::string;

namespace lyx {
namespace frontend {

namespace {

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


bool Dialogs::isValidName(string const & name) const
{
	return std::find_if(dialognames, end_dialognames,
			    cmpCStr(name.c_str())) != end_dialognames;
}


Dialogs::DialogPtr Dialogs::build(string const & name)
{
	BOOST_ASSERT(isValidName(name));

	DialogPtr dialog(new Dialog(lyxview_, name));
	dialog->bc().view(new Qt2BC(dialog->bc()));

	if (name == "aboutlyx") {
		dialog->setController(new ControlAboutlyx(*dialog));
		dialog->setView(new GuiAbout(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "bibitem") {
		dialog->setController(new ControlCommand(*dialog, name, name));
		dialog->setView(new GuiBibitem(*dialog));
		dialog->bc().bp(new OkCancelReadOnlyPolicy);
	} else if (name == "bibtex") {
		dialog->setController(new ControlBibtex(*dialog));
		dialog->setView(new GuiBibtex(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "box") {
		dialog->setController(new ControlBox(*dialog));
		dialog->setView(new GuiBox(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "branch") {
		dialog->setController(new ControlBranch(*dialog));
		dialog->setView(new GuiBranch(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "changes") {
		dialog->setController(new ControlChanges(*dialog));
		dialog->setView(new GuiChanges(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "character") {
		dialog->setController(new ControlCharacter(*dialog));
		dialog->setView(new GuiCharacter(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "citation") {
		GuiCitation * ci = new GuiCitation(*dialog);
		dialog->setController(ci);
		dialog->setView(new GuiCitationDialog(*dialog, ci));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "document") {
		dialog->setController(new ControlDocument(*dialog));
		dialog->setView(new GuiDocument(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "embedding") {
		GuiViewBase & gui_view = static_cast<GuiViewBase &>(lyxview_);
		GuiEmbeddedFiles * qef = new GuiEmbeddedFiles(*dialog);
		dialog->setController(qef);
		dialog->setView(new DockView<GuiEmbeddedFiles, GuiEmbeddedFilesDialog>(
			*dialog, qef, &gui_view, _("Embedded Files"), Qt::RightDockWidgetArea));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "errorlist") {
		dialog->setController(new ControlErrorList(*dialog));
		dialog->setView(new GuiErrorList(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new GuiERT(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new GuiExternal(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "file") {
		dialog->setController(new ControlShowFile(*dialog));
		dialog->setView(new GuiShowFile(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "findreplace") {
		dialog->setController(new ControlSearch(*dialog));
		dialog->setView(new GuiSearch(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "float") {
		dialog->setController(new ControlFloat(*dialog));
		dialog->setView(new GuiFloat(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "graphics") {
		dialog->setController(new ControlGraphics(*dialog));
		dialog->setView(new GuiGraphics(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "include") {
		dialog->setController(new ControlInclude(*dialog));
		dialog->setView(new GuiInclude(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "index") {
		dialog->setController(new ControlCommand(*dialog, name, name));
		dialog->setView(new GuiIndex(*dialog,
					   _("Index Entry"),
					   qt_("&Keyword:")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "nomenclature") {
		dialog->setController(new ControlCommand(*dialog, name, name));
		dialog->setView(new GuiNomencl(*dialog, _("Nomenclature Entry")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "label") {
		dialog->setController(new ControlCommand(*dialog, name, name));
		dialog->setView(new GuiIndex(*dialog,
					   _("Label"),
					   qt_("&Label:")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "log") {
		dialog->setController(new ControlLog(*dialog));
		dialog->setView(new GuiLog(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "view-source") {
		GuiViewSource * qvs = new GuiViewSource(*dialog);
		dialog->setController(qvs);
		GuiViewBase & gui_view = static_cast<GuiViewBase &>(lyxview_);
		dialog->setView(new DockView<GuiViewSource, GuiViewSourceDialog>(
			*dialog, qvs, &gui_view, _("LaTeX Source"), Qt::BottomDockWidgetArea));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "mathdelimiter") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new GuiDelimiter(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "mathmatrix") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new GuiMathMatrix(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "note") {
		dialog->setController(new ControlNote(*dialog));
		dialog->setView(new GuiNote(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "paragraph") {
		dialog->setController(new ControlParagraph(*dialog));
		dialog->setView(new GuiParagraph(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "prefs") {
		dialog->setController(new ControlPrefs(*dialog));
		dialog->setView(new GuiPrefs(*dialog));
		dialog->bc().bp(new PreferencesPolicy);
	} else if (name == "print") {
		dialog->setController(new ControlPrint(*dialog));
		dialog->setView(new GuiPrint(*dialog));
		dialog->bc().bp(new OkApplyCancelPolicy);
	} else if (name == "ref") {
		// full qualification because qt4 has also a ControlRef type
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new GuiRef(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "sendto") {
		dialog->setController(new ControlSendto(*dialog));
		dialog->setView(new GuiSendto(*dialog));
		dialog->bc().bp(new OkApplyCancelPolicy);
	} else if (name == "spellchecker") {
		dialog->setController(new ControlSpellchecker(*dialog));
		dialog->setView(new GuiSpellchecker(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabular") {
		dialog->setController(new ControlTabular(*dialog));
		dialog->setView(new GuiTabular(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabularcreate") {
		dialog->setController(new ControlTabularCreate(*dialog));
		dialog->setView(new GuiTabularCreate(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "texinfo") {
		dialog->setController(new ControlTexinfo(*dialog));
		dialog->setView(new GuiTexinfo(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
#ifdef HAVE_LIBAIKSAURUS
	} else if (name == "thesaurus") {
		dialog->setController(new ControlThesaurus(*dialog));
		dialog->setView(new GuiThesaurus(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
#endif
	} else if (name == "toc") {
		GuiViewBase & gui_view = static_cast<GuiViewBase &>(lyxview_);
		GuiToc * qtoc = new GuiToc(*dialog, &gui_view);
		dialog->setController(qtoc);
#ifdef Q_WS_MACX
		// On Mac show as a drawer at the right
		dialog->setView(new DockView<GuiToc, TocWidget>(
			*dialog, qtoc, &gui_view, _("Outline"), Qt::RightDockWidgetArea, Qt::Drawer));
#else
		dialog->setView(new DockView<GuiToc, TocWidget>(
			*dialog, qtoc, &gui_view, _("Outline")));
#endif
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "url") {
		dialog->setController(new ControlCommand(*dialog, name, name));
		dialog->setView(new UrlView(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "vspace") {
		dialog->setController(new ControlVSpace(*dialog));
		dialog->setView(new GuiVSpace(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "wrap") {
		dialog->setController(new ControlWrap(*dialog));
		dialog->setView(new GuiWrap(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "listings") {
		dialog->setController(new ControlListings(*dialog));
		dialog->setView(new GuiListings(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
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
