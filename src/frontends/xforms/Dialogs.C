/**
 * \file xforms/Dialogs.C
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs_impl.h"

#include "Tooltips.h"
#include "xformsBC.h"
#include "combox.h"

#include "ControlAboutlyx.h"
#include "FormAboutlyx.h"
#include "forms/form_aboutlyx.h"

#include "ControlBibitem.h"
#include "FormBibitem.h"
#include "forms/form_bibitem.h"

#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "forms/form_bibtex.h"

#include "FormBrowser.h"
#include "forms/form_browser.h"

#include "ControlCharacter.h"
#include "FormCharacter.h"
#include "forms/form_character.h"

#include "ControlCitation.h"
#include "FormCitation.h"
#include "forms/form_citation.h"

#include "FormDocument.h"
#include "forms/form_document.h"

#include "ControlError.h"
#include "FormError.h"
#include "forms/form_error.h"

#include "ControlERT.h"
#include "FormERT.h"
#include "forms/form_ert.h"

#include "ControlExternal.h"
#include "FormExternal.h"
#include "forms/form_external.h"

#include "ControlFloat.h"
#include "FormFloat.h"
#include "forms/form_float.h"

#include "ControlForks.h"
#include "FormForks.h"
#include "forms/form_forks.h"

#include "ControlGraphics.h"
#include "FormGraphics.h"
#include "forms/form_graphics.h"

#include "ControlInclude.h"
#include "FormInclude.h"
#include "forms/form_include.h"

#include "ControlIndex.h"
#include "FormIndex.h"
#include "forms/form_index.h"

#include "ControlLog.h"
#include "FormLog.h"

#include "ControlShowFile.h"
#include "FormShowFile.h"

#include "FormMathsBitmap.h"

#include "FormMathsPanel.h"
#include "forms/form_maths_panel.h"

#include "FormMathsDeco.h"
#include "forms/form_maths_deco.h"

#include "FormMathsDelim.h"
#include "forms/form_maths_delim.h"

#include "FormMathsMatrix.h"
#include "forms/form_maths_matrix.h"

#include "FormMathsSpace.h"
#include "forms/form_maths_space.h"

#include "FormMathsStyle.h"
#include "forms/form_maths_style.h"

#include "ControlMinipage.h"
#include "FormMinipage.h"
#include "forms/form_minipage.h"

#include "ControlParagraph.h"
#include "FormParagraph.h"
#include "forms/form_paragraph.h"

#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "forms/form_preamble.h"

#include "FormPreferences.h"
#include "forms/form_preferences.h"

#include "ControlPrint.h"
#include "FormPrint.h"
#include "forms/form_print.h"

#include "ControlRef.h"
#include "FormRef.h"
#include "forms/form_ref.h"

#include "ControlSearch.h"
#include "FormSearch.h"
#include "forms/form_search.h"

#include "ControlSendto.h"
#include "FormSendto.h"
#include "forms/form_sendto.h"

#include "ControlSpellchecker.h"
#include "FormSpellchecker.h"
#include "forms/form_spellchecker.h"

#include "FormTabular.h"
#include "forms/form_tabular.h"

#include "ControlTabularCreate.h"
#include "FormTabularCreate.h"
#include "forms/form_tabular_create.h"

#include "ControlTexinfo.h"
#include "FormTexinfo.h"
#include "forms/form_texinfo.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "FormThesaurus.h"
#include "forms/form_thesaurus.h"
#endif

#include "ControlToc.h"
#include "FormToc.h"
#include "forms/form_toc.h"

#include "ControlUrl.h"
#include "FormUrl.h"
#include "forms/form_url.h"

#include "ControlVCLog.h"
#include "FormVCLog.h"


Dialogs::Dialogs(LyXView & lv)
	: pimpl_(new Impl(lv, *this))
{
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
}


Dialogs::~Dialogs()
{}


void Dialogs::toggleTooltips()
{
	Tooltips::toggleEnabled();
}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}
