/**
 * \file xforms/Dialogs3.C
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

#include "Tooltips.h"

#include "ControlBibitem.h"
#include "FormBibitem.h"
#include "forms/form_bibitem.h"

#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "forms/form_bibtex.h"

#include "ControlCitation.h"
#include "FormCitation.h"
#include "forms/form_citation.h"

#include "ControlError.h"
#include "FormError.h"
#include "forms/form_error.h"

#include "ControlERT.h"
#include "FormERT.h"
#include "forms/form_ert.h"

#include "ControlIndex.h"
#include "FormIndex.h"
#include "forms/form_index.h"

#include "ControlRef.h"
#include "FormRef.h"
#include "forms/form_ref.h"

#include "ControlToc.h"
#include "FormToc.h"
#include "forms/form_toc.h"

#include "ControlUrl.h"
#include "FormUrl.h"
#include "forms/form_url.h"

#include "xformsBC.h"
#include "ButtonController.h"


typedef ButtonController<OkCancelPolicy, xformsBC>
	OkCancelBC;

typedef ButtonController<OkCancelReadOnlyPolicy, xformsBC>
	OkCancelReadOnlyBC;

typedef ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC>
	NoRepeatedApplyReadOnlyBC;


namespace {

// char const * const dialognames[] = { "bibitem", "bibtex", "citation",
// 				     "error", "ert", "external", "float",
// 				     "graphics", "include", "index",
// 				     "minipage", "ref", "tabular", "toc",
// 				     "url", "wrap" };
char const * const dialognames[] = { "bibitem", "bibtex", "citation",
				     "error", "ert", "index", "ref",
				     "toc", "url" };

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
		dialog->setView(new FormBibitem(*dialog));
		dialog->setButtonController(new OkCancelReadOnlyBC);
	} else if (name == "bibtex") {
		dialog->setController(new ControlBibtex(*dialog));
		dialog->setView(new FormBibtex(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "citation") {
		dialog->setController(new ControlCitation(*dialog));
		dialog->setView(new FormCitation(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "error") {
		dialog->setController(new ControlError(*dialog));
		dialog->setView(new FormError(*dialog));
		dialog->setButtonController(new OkCancelBC);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new FormERT(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "index") {
		dialog->setController(new ControlIndex(*dialog));
		dialog->setView(new FormIndex(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "ref") {
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new FormRef(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new FormToc(*dialog));
		dialog->setButtonController(new OkCancelBC);
	} else if (name == "url") {
		dialog->setController(new ControlUrl(*dialog));
		dialog->setView(new FormUrl(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	}

	return dialog;
}


void Dialogs::toggleTooltips()
{
	Tooltips::toggleEnabled();
}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}
