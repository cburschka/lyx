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

#include "ControlBibtex.h"
#include "ControlCitation.h"
#include "ControlCommand.h"
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

#include "FormBibitem.h"
#include "forms/form_bibitem.h"
#include "FormBibtex.h"
#include "forms/form_bibtex.h"
#include "FormCitation.h"
#include "forms/form_citation.h"
#include "FormError.h"
#include "forms/form_error.h"
#include "FormERT.h"
#include "forms/form_ert.h"
#include "FormExternal.h"
#include "forms/form_external.h"
#include "FormFloat.h"
#include "forms/form_float.h"
#include "FormGraphics.h"
#include "forms/form_graphics.h"
#include "FormInclude.h"
#include "forms/form_include.h"
#include "FormMinipage.h"
#include "forms/form_minipage.h"
#include "FormRef.h"
#include "forms/form_ref.h"
#include "FormTabular.h"
#include "forms/form_tabular.h"
#include "FormTabularCreate.h"
#include "forms/form_tabular_create.h"
#include "FormText.h"
#include "forms/form_text.h"
#include "FormToc.h"
#include "forms/form_toc.h"
#include "FormUrl.h"
#include "forms/form_url.h"
#include "FormWrap.h"
#include "forms/form_wrap.h"

#include "xformsBC.h"
#include "ButtonController.h"


typedef ButtonController<OkCancelPolicy, xformsBC>
	OkCancelBC;

typedef ButtonController<OkCancelReadOnlyPolicy, xformsBC>
	OkCancelReadOnlyBC;

typedef ButtonController<OkApplyCancelReadOnlyPolicy, xformsBC>
	OkApplyCancelReadOnlyBC;

typedef ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC>
	NoRepeatedApplyReadOnlyBC;


namespace {

char const * const dialognames[] = { "bibitem", "bibtex", "citation",
				     "error", "ert", "external", "float",
				     "graphics", "include", "index", "label",
				     "minipage", "ref", "tabular",
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

	if (name == "bibitem") {
		dialog->setController(new ControlCommand(*dialog, name));
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
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new FormExternal(*dialog));
		dialog->setButtonController(new OkApplyCancelReadOnlyBC);
	} else if (name == "float") {
		dialog->setController(new ControlFloat(*dialog));
		dialog->setView(new FormFloat(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "graphics") {
		dialog->setController(new ControlGraphics(*dialog));
		dialog->setView(new FormGraphics(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "include") {
		dialog->setController(new ControlInclude(*dialog));
		dialog->setView(new FormInclude(*dialog));
		dialog->setButtonController(new OkApplyCancelReadOnlyBC);
	} else if (name == "index") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormText(*dialog,
					     _("Index"), _("Keyword:|#K")));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "label") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormText(*dialog,
					     _("Label"), _("Label:|#L")));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "minipage") {
		dialog->setController(new ControlMinipage(*dialog));
		dialog->setView(new FormMinipage(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "ref") {
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new FormRef(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "tabular") {
		dialog->setController(new ControlTabular(*dialog));
		dialog->setView(new FormTabular(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "tabularcreate") {
		dialog->setController(new ControlTabularCreate(*dialog));
		dialog->setView(new FormTabularCreate(*dialog));
		dialog->setButtonController(new OkApplyCancelReadOnlyBC);
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new FormToc(*dialog));
		dialog->setButtonController(new OkCancelBC);
	} else if (name == "url") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormUrl(*dialog));
		dialog->setButtonController(new NoRepeatedApplyReadOnlyBC);
	} else if (name == "wrap") {
		dialog->setController(new ControlWrap(*dialog));
		dialog->setView(new FormWrap(*dialog));
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
