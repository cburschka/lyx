/**
 * \file xforms/Dialogs2.C
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

#include "FormBrowser.h"
#include "forms/form_browser.h"

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

void Dialogs::showMathPanel()
{
	pimpl_->dialog(pimpl_->mathpanel).show();
}


void Dialogs::showMinipage(InsetMinipage * im)
{
	pimpl_->dialog(pimpl_->minipage).controller().showInset(im);
}


void Dialogs::updateMinipage(InsetMinipage * im)
{
	pimpl_->dialog(pimpl_->minipage).controller().showInset(im);
}


void Dialogs::showParagraph()
{
	pimpl_->dialog(pimpl_->paragraph).controller().show();
}


void Dialogs::updateParagraph()
{
	pimpl_->dialog(pimpl_->paragraph).controller().changedParagraph();
}


void Dialogs::showPreamble()
{
	pimpl_->dialog(pimpl_->preamble).controller().show();
}


void Dialogs::showPreferences()
{
	pimpl_->dialog(pimpl_->preferences).show();
}


void Dialogs::showPrint()
{
	pimpl_->dialog(pimpl_->print).controller().show();
}


void Dialogs::showRef(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->ref).controller().showInset(ic);
}


void Dialogs::createRef(string const & s)
{
	pimpl_->dialog(pimpl_->ref).controller().createInset(s);
}


void Dialogs::showSearch()
{
	pimpl_->dialog(pimpl_->search).controller().show();
}


void Dialogs::showSendto()
{
	pimpl_->dialog(pimpl_->sendto).controller().show();
}


void Dialogs::showSpellchecker()
{
	pimpl_->dialog(pimpl_->spellchecker).controller().show();
}


void Dialogs::showTabular(InsetTabular * it)
{
	pimpl_->dialog(pimpl_->tabular).showInset(it);
}


void Dialogs::updateTabular(InsetTabular * it)
{
	pimpl_->dialog(pimpl_->tabular).updateInset(it);
}


void Dialogs::showTabularCreate()
{
	pimpl_->dialog(pimpl_->tabularcreate).controller().show();
}


void Dialogs::showTexinfo()
{
	pimpl_->dialog(pimpl_->texinfo).controller().show();
}


#ifdef HAVE_LIBAIKSAURUS

void Dialogs::showThesaurus(string const & s)
{
	pimpl_->dialog(pimpl_->thesaurus).controller().showEntry(s);
}

#else

void Dialogs::showThesaurus(string const &)
{}

#endif


void Dialogs::showTOC(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->toc).controller().showInset(ic);
}


void Dialogs::createTOC(string const & s)
{
	pimpl_->dialog(pimpl_->toc).controller().createInset(s);
}


void Dialogs::showUrl(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->url).controller().showInset(ic);
}


void Dialogs::createUrl(string const & s)
{
	pimpl_->dialog(pimpl_->url).controller().createInset(s);
}


void Dialogs::showVCLogFile()
{
	pimpl_->dialog(pimpl_->vclogfile).controller().show();
}
