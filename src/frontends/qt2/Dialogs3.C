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

#include "ControlBibitem.h"
#include "ControlBibtex.h"
#include "ControlCitation.h"
#include "ControlError.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlIndex.h"
#include "ControlLabel.h"
#include "ControlRef.h"
#include "ControlToc.h"
#include "ControlUrl.h"

#include "QBibitem.h"
#include "QBibitemDialog.h"
#include "QBibtex.h"
#include "QBibtexDialog.h"
#include "QCitation.h"
#include "QCitationDialog.h"
#include "QError.h"
#include "QErrorDialog.h"
#include "QERT.h"
#include "QERTDialog.h"
#include "QExternal.h"
#include "QExternalDialog.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QIndex.h"
#include "QIndexDialog.h"
#include "QRef.h"
#include "QRefDialog.h"
#include "QToc.h"
#include "QTocDialog.h"
#include "QURL.h"
#include "QURLDialog.h"

#include "Qt2BC.h"
#include "ButtonController.h"
#include "qt_helpers.h"


typedef ButtonController<OkCancelPolicy, Qt2BC>
	OkCancelBC;

typedef ButtonController<OkCancelReadOnlyPolicy, Qt2BC>
	OkCancelReadOnlyBC;

typedef ButtonController<OkApplyCancelReadOnlyPolicy, Qt2BC>
	OkApplyCancelReadOnlyBC;

typedef ButtonController<NoRepeatedApplyReadOnlyPolicy, Qt2BC>
	NoRepeatedApplyReadOnlyBC;


namespace {

char const * const dialognames[] = { "bibitem", "bibtex", "citation",
				     "error", "ert", "external", "index",
				     "label", "ref", "toc", "url" };

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

	if (name == "bibitem") {
		dialog->setController(new ControlBibitem(*dialog));
		dialog->setView(new QBibitem(*dialog));
		dialog->setButtonController(new OkCancelReadOnlyBC);
	} else if (name == "bibtex") {
		dialog->setController(new ControlBibtex(*dialog));
		dialog->setView(new QBibtex(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "citation") {
		dialog->setController(new ControlCitation(*dialog));
		dialog->setView(new QCitation(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "error") {
		dialog->setController(new ControlError(*dialog));
		dialog->setView(new QError(*dialog));
		dialog->setButtonController(new OkCancelBC);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new QERT(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new QExternal(*dialog));
		dialog->setButtonController(new OkApplyCancelReadOnlyBC);
	} else if (name == "index") {
		dialog->setController(new ControlIndex(*dialog));
		dialog->setView(new QIndex(*dialog,
					   qt_("LyX: Insert Index Entry"),
					   qt_("&Keyword")));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "label") {
		dialog->setController(new ControlLabel(*dialog));
		dialog->setView(new QIndex(*dialog,
					   qt_("LyX: Insert Label"),
					   qt_("&Label")));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "ref") {
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new QRef(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new QToc(*dialog));
		dialog->setButtonController(new OkCancelBC);
	} else if (name == "url") {
		dialog->setController(new ControlUrl(*dialog));
		dialog->setView(new QURL(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
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
