/**
 * \file xforms/Dialogs.C
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae, rae@lyx.org
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"

#include "GUI.h"
#include "xformsBC.h"

#include "combox.h"       // needed for clean destruction of boost::scoped_ptr
#include "forms/form_aboutlyx.h"
#include "forms/form_bibitem.h"
#include "forms/form_bibtex.h"
#include "forms/form_browser.h"
#include "forms/form_character.h"
#include "forms/form_citation.h"
#include "forms/form_error.h"
#include "forms/form_ert.h"
#include "forms/form_external.h"
#include "forms/form_float.h"
#include "forms/form_forks.h"
#include "forms/form_graphics.h"
#include "forms/form_include.h"
#include "forms/form_index.h"
#include "forms/form_minipage.h"
#include "forms/form_paragraph.h"
#include "forms/form_preamble.h"
#include "forms/form_print.h"
#include "forms/form_ref.h"
#include "forms/form_search.h"
#include "forms/form_sendto.h"
#include "forms/form_spellchecker.h"
#include "forms/form_tabular_create.h"
#include "forms/form_texinfo.h"
#include "forms/form_thesaurus.h"
#include "forms/form_toc.h"
#include "forms/form_url.h"

#include "FormAboutlyx.h"
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormError.h"
#include "FormERT.h"
#include "FormExternal.h"
#include "FormFloat.h"
#include "FormForks.h"
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormMinipage.h"
#include "FormParagraph.h"
#include "FormPreamble.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSearch.h"
#include "FormSendto.h"
#include "FormShowFile.h"
#include "FormSpellchecker.h"
#include "FormTabularCreate.h"
#include "FormTexinfo.h"
#include "FormThesaurus.h"
#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"

#include "FormDocument.h"
#include "FormMathsPanel.h"
#include "FormPreferences.h"
#include "FormTabular.h"

#include "Tooltips.h"

/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}


Dialogs::Dialogs(LyXView * lv)
{
	add(new GUI<ControlAboutlyx, FormAboutlyx,
		    OkCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlBibitem, FormBibitem,
		    OkCancelReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlBibtex, FormBibtex,
		    OkCancelReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlCharacter, FormCharacter,
		    OkApplyCancelReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlCitation, FormCitation,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlError, FormError,
		    OkCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlERT, FormERT,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlExternal, FormExternal,
		    OkApplyCancelReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlForks, FormForks,
		    OkApplyCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlGraphics, FormGraphics,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlInclude, FormInclude,
		    OkCancelReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlIndex, FormIndex,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlLog, FormLog,
		    OkCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlMinipage, FormMinipage,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlFloat, FormFloat,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlParagraph, FormParagraph,
		    OkApplyCancelReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlPreamble, FormPreamble,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlPrint, FormPrint,
		    OkApplyCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlRef, FormRef,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlSearch, FormSearch,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlSendto, FormSendto,
		    OkApplyCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlShowFile, FormShowFile,
		    OkCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlSpellchecker, FormSpellchecker,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlTabularCreate, FormTabularCreate,
		    OkApplyCancelReadOnlyPolicy, xformsBC>(*lv, *this));
#ifdef HAVE_LIBAIKSAURUS
	add(new GUI<ControlThesaurus, FormThesaurus,
		    OkApplyCancelReadOnlyPolicy, xformsBC>(*lv, *this));
#endif
	add(new GUI<ControlTexinfo, FormTexinfo,
		    OkCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlToc, FormToc,
		    OkCancelPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlUrl, FormUrl,
		    NoRepeatedApplyReadOnlyPolicy, xformsBC>(*lv, *this));
	add(new GUI<ControlVCLog, FormVCLog,
		    OkCancelPolicy, xformsBC>(*lv, *this));

	add(new FormDocument(lv, this));
	add(new FormMathsPanel(lv, this));
	add(new FormPreferences(lv, this));
	add(new FormTabular(lv, this));

	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
}
