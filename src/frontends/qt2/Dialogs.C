/**
 * \file qt2/Dialogs.C
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
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlError.h"
#include "ControlErrorList.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlGraphics.h"
#include "ControlInclude.h"
#include "ControlLog.h"
#include "ControlMath.h"
#include "ControlMinipage.h"
#include "ControlParagraph.h"
#include "ControlRef.h"
#include "ControlShowFile.h"
#include "ControlTabular.h"
#include "ControlTabularCreate.h"
#include "ControlToc.h"
#include "ControlVCLog.h"
#include "ControlWrap.h"

#include "QAbout.h"
#include "QBibitem.h"
#include "QBibtex.h"
#include "QChanges.h"
#include "QCharacter.h"
#include "QCitation.h"
#include "QError.h"
#include "QErrorList.h"
#include "QERT.h"
#include "QExternal.h"
#include "QFloat.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QGraphics.h"
#include "QInclude.h"
#include "QIndex.h"
#include "QLog.h"
#include "QMath.h"
#include "QMinipage.h"
#include "QParagraph.h"
#include "QRef.h"
#include "QShowFile.h"
#include "QTabular.h"
#include "QTabularCreate.h"
#include "QToc.h"
#include "QURL.h"
#include "QVCLog.h"
#include "QWrap.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "QThesaurus.h"
#endif

#include "Qt2BC.h"
#include "ButtonController.h"
#include "qt_helpers.h"


namespace {

char const * const dialognames[] = { "about", "bibitem", "bibtex", "changes",
"character", "citation", "error", "errorlist", "ert", "external", "file",
"float", "graphics", "include", "index", "label", "log",
"math", "mathdelimiter", "mathmatrix",
"minipage", "paragraph", "ref", "tabular", "tabularcreate",

#ifdef HAVE_LIBAIKSAURUS
"thesaurus",
#endif

"toc", "url", "vclog", "wrap" };

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
	} else if (name == "character") {
		dialog->setController(new ControlCharacter(*dialog));
		dialog->setView(new QCharacter(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "citation") {
		dialog->setController(new ControlCitation(*dialog));
		dialog->setView(new QCitation(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "error") {
		dialog->setController(new ControlError(*dialog));
		dialog->setView(new QError(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "errorlist") {
		dialog->setController(new ControlErrorList(*dialog));
		dialog->setView(new QErrorList(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new QERT(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new QExternal(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "file") {
		dialog->setController(new ControlShowFile(*dialog));
		dialog->setView(new QShowFile(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
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
					   _("LyX: Index Entry"),
					   qt_("&Keyword")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "label") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new QIndex(*dialog,
					   _("LyX: Label"),
					   qt_("&Label")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "log") {
		dialog->setController(new ControlLog(*dialog));
		dialog->setView(new QLog(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "math") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new QMath(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "mathdelimiter") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new QMathDelimiter(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "mathmatrix") {
		dialog->setController(new ControlMath(*dialog));
		dialog->setView(new QMathMatrix(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
	} else if (name == "minipage") {
		dialog->setController(new ControlMinipage(*dialog));
		dialog->setView(new QMinipage(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "paragraph") {
		dialog->setController(new ControlParagraph(*dialog));
		dialog->setView(new QParagraph(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
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
#ifdef HAVE_LIBAIKSAURUS
	} else if (name == "thesaurus") {
		dialog->setController(new ControlThesaurus(*dialog));
		dialog->setView(new QThesaurus(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
#endif
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new QToc(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "url") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new QURL(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "vclog") {
		dialog->setController(new ControlVCLog(*dialog));
		dialog->setView(new QVCLog(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
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
