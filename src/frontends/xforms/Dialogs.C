/**
 * \file xforms/Dialogs.C
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

#include "ControlAboutlyx.h"
#include "ControlBibtex.h"
#include "ControlChanges.h"
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlCommand.h"
#include "ControlError.h"
#include "ControlErrorList.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlGraphics.h"
#include "ControlInclude.h"
#include "ControlLog.h"
#include "ControlMinipage.h"
#include "ControlParagraph.h"
#include "ControlRef.h"
#include "ControlShowFile.h"
#include "ControlTabular.h"
#include "ControlTabularCreate.h"
#include "ControlToc.h"
#include "ControlVCLog.h"
#include "ControlWrap.h"

#include "FormAboutlyx.h"
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormChanges.h"
#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormError.h"
#include "FormErrorList.h"
#include "FormERT.h"
#include "FormExternal.h"
#include "FormFloat.h"
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormLog.h"
#include "FormMinipage.h"
#include "FormParagraph.h"
#include "FormRef.h"
#include "FormTabular.h"
#include "FormShowFile.h"
#include "FormTabularCreate.h"
#include "FormText.h"
#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"
#include "FormWrap.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "FormThesaurus.h"
#endif

#include "xformsBC.h"
#include "ButtonController.h"


namespace {

char const * const dialognames[] = { "about", "bibitem", "bibtex", "changes", "character", "citation", "error", "errorlist" , "ert", "external", "file", "float", "graphics", "include", "index", "label", "log", "minipage", "paragraph", "ref", "tabular", "tabularcreate",

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
	dialog->bc().view(new xformsBC(dialog->bc()));

	if (name == "about") {
		dialog->setController(new ControlAboutlyx(*dialog));
		dialog->setView(new FormAboutlyx(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "bibitem") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormBibitem(*dialog));
		dialog->bc().bp(new OkCancelReadOnlyPolicy);
	} else if (name == "bibtex") {
		dialog->setController(new ControlBibtex(*dialog));
		dialog->setView(new FormBibtex(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "character") {
		dialog->setController(new ControlCharacter(*dialog));
		dialog->setView(new FormCharacter(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "changes") {
		dialog->setController(new ControlChanges(*dialog));
		dialog->setView(new FormChanges(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "citation") {
		dialog->setController(new ControlCitation(*dialog));
		dialog->setView(new FormCitation(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "error") {
		dialog->setController(new ControlError(*dialog));
		dialog->setView(new FormError(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "errorlist") {
		dialog->setController(new ControlErrorList(*dialog));
		dialog->setView(new FormErrorList(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "ert") {
		dialog->setController(new ControlERT(*dialog));
		dialog->setView(new FormERT(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "external") {
		dialog->setController(new ControlExternal(*dialog));
		dialog->setView(new FormExternal(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "file") {
		dialog->setController(new ControlShowFile(*dialog));
		dialog->setView(new FormShowFile(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "float") {
		dialog->setController(new ControlFloat(*dialog));
		dialog->setView(new FormFloat(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "graphics") {
		dialog->setController(new ControlGraphics(*dialog));
		dialog->setView(new FormGraphics(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "include") {
		dialog->setController(new ControlInclude(*dialog));
		dialog->setView(new FormInclude(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "index") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormText(*dialog,
					     _("Index"), _("Keyword:|#K")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "label") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormText(*dialog,
					     _("Label"), _("Label:|#L")));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "log") {
		dialog->setController(new ControlLog(*dialog));
		dialog->setView(new FormLog(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "minipage") {
		dialog->setController(new ControlMinipage(*dialog));
		dialog->setView(new FormMinipage(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "paragraph") {
		dialog->setController(new ControlParagraph(*dialog));
		dialog->setView(new FormParagraph(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
	} else if (name == "ref") {
		dialog->setController(new ControlRef(*dialog));
		dialog->setView(new FormRef(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabular") {
		dialog->setController(new ControlTabular(*dialog));
		dialog->setView(new FormTabular(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "tabularcreate") {
		dialog->setController(new ControlTabularCreate(*dialog));
		dialog->setView(new FormTabularCreate(*dialog));
		dialog->bc().bp(new IgnorantPolicy);
#ifdef HAVE_LIBAIKSAURUS
	} else if (name == "thesaurus") {
		dialog->setController(new ControlThesaurus(*dialog));
		dialog->setView(new FormThesaurus(*dialog));
		dialog->bc().bp(new OkApplyCancelReadOnlyPolicy);
#endif
	} else if (name == "toc") {
		dialog->setController(new ControlToc(*dialog));
		dialog->setView(new FormToc(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "url") {
		dialog->setController(new ControlCommand(*dialog, name));
		dialog->setView(new FormUrl(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
	} else if (name == "vclog") {
		dialog->setController(new ControlVCLog(*dialog));
		dialog->setView(new FormVCLog(*dialog));
		dialog->bc().bp(new OkCancelPolicy);
	} else if (name == "wrap") {
		dialog->setController(new ControlWrap(*dialog));
		dialog->setView(new FormWrap(*dialog));
		dialog->bc().bp(new NoRepeatedApplyReadOnlyPolicy);
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
