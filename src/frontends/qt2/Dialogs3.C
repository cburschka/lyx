/**
 * \file qt2/Dialogs3.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "Dialogs.h"
#include "Dialog.h"

#include "ControlAboutlyx.h"
#include "ControlBibtex.h"
#include "ControlChanges.h"
#include "ControlCitation.h"
#include "ControlError.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlGraphics.h"
#include "ControlInclude.h"
#include "ControlMinipage.h"
#include "ControlRef.h"
#include "ControlTabular.h"
#include "ControlTabularCreate.h"
#include "ControlToc.h"
#include "ControlWrap.h"

#include "QAbout.h"
#include "QAboutDialog.h"
#include "QBibitem.h"
#include "QBibitemDialog.h"
#include "QBibtex.h"
#include "QBibtexDialog.h"
#include "QChanges.h"
#include "QChangesDialog.h"
#include "QCitation.h"
#include "QCitationDialog.h"
#include "QError.h"
#include "QErrorDialog.h"
#include "QERT.h"
#include "QERTDialog.h"
#include "QExternal.h"
#include "QExternalDialog.h"
#include "QFloat.h"
#include "QFloatDialog.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QGraphics.h"
#include "QGraphicsDialog.h"
#include "QInclude.h"
#include "QIncludeDialog.h"
#include "QIndex.h"
#include "QIndexDialog.h"
#include "QMinipage.h"
#include "QMinipageDialog.h"
#include "QRef.h"
#include "QRefDialog.h"
#include "QTabular.h"
#include "QTabularDialog.h"
#include "QTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QToc.h"
#include "QTocDialog.h"
#include "QURL.h"
#include "QURLDialog.h"
#include "QWrap.h"
#include "QWrapDialog.h"

#include "Qt2BC.h"
#include "ButtonController.h"
#include "qt_helpers.h"


namespace {

char const * const dialognames[] = { "about", "bibitem", "bibtex", "changes",
				     "citation", "error", "ert", "external",
				     "float", "graphics", "include", "index",
				     "label", "minipage", "ref", "tabular",
				     "tabularcreate", "toc", "url", "wrap" };

char const * const * const end_dialognames =
	dialognames + (sizeof(dialognames) / sizeof(char *));

struct cmpCStr {
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


Dialog * Dialogs::build(string const & name)
{
	if (!isValidName(name))
		return 0;

	Dialog * dialog = new Dialog(lyxview_, name);
	dialog->bc().view(new Qt2BC(dialog->bc()));

	if (name == "about") {
		dialog->setController(new ControlAboutlyx(*dialog));
		dialog->setView(new QAbout(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "bibitem") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new QBibitem(*dialog));
		dialog->bc().bp(new OkCancelReadOnlyPolicy);
	} else if (name == "bibtex") {
		dialog->setController(new ControlBibtex(*dialog));
		dialog->setView(new QBibtex(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "changes") {
		dialog->setController(new ControlChanges(*dialog));
		dialog->setView(new QChanges(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "citation") {
		dialog->setController(new ControlCitation(*dialog));
		dialog->setView(new QCitation(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "error") {
		dialog->setController(new ControlError(*dialog));
		dialog->setView(new QError(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new QERT(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new QExternal(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "float") {
		dialog->setController(new ControlFloat(*dialog));
		dialog->setView(new QFloat(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "graphics") {
		dialog->setController(new ControlGraphics(*dialog));
		dialog->setView(new QGraphics(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "include") {
		dialog->setController(new ControlInclude(*dialog));
		dialog->setView(new QInclude(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "index") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new QIndex(*dialog,
					   qt_("LyX: Insert Index Entry"),
					   qt_("&Keyword")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "label") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new QIndex(*dialog,
					   qt_("LyX: Insert Label"),
					   qt_("&Label")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "minipage") {
		dialog->setController(new ControlMinipage(*dialog));
		dialog->setView(new QMinipage(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "ref") {
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new QRef(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabular") {
		dialog->setController(new ControlTabular(*dialog));
		dialog->setView(new QTabular(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabularcreate") {
		dialog->setController(new ControlTabularCreate(*dialog));
		dialog->setView(new QTabularCreate(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new QToc(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "url") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new QURL(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "wrap") {
		dialog->setController(new ControlWrap(*dialog));
		dialog->setView(new QWrap(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	}

	return dialog;
}


void Dialogs::toggleTooltips()
{}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return false;
}
