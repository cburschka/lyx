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


void Dialogs::showAboutlyx()
{
	pimpl_->dialog(pimpl_->aboutlyx).controller().show();
}


void Dialogs::showBibitem(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->bibitem).controller().showInset(ic);
}


void Dialogs::showBibtex(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->bibtex).controller().showInset(ic);
}


void Dialogs::showCharacter()
{
	pimpl_->dialog(pimpl_->character).controller().show();
}


void Dialogs::setUserFreeFont()
{
	pimpl_->dialog(pimpl_->character).controller().apply();
}


void Dialogs::showCitation(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->citation).controller().showInset(ic);
}


void Dialogs::createCitation(string const & s)
{
	pimpl_->dialog(pimpl_->citation).controller().createInset(s);
}


void Dialogs::showDocument()
{
	pimpl_->dialog(pimpl_->document).show();
}


void Dialogs::showError(InsetError * ie)
{
	pimpl_->dialog(pimpl_->error).controller().showInset(ie);
}


void Dialogs::showERT(InsetERT * ie)
{
	pimpl_->dialog(pimpl_->ert).controller().showInset(ie);
}


void Dialogs::updateERT(InsetERT * ie)
{
	pimpl_->dialog(pimpl_->ert).controller().showInset(ie);
}


void Dialogs::showExternal(InsetExternal * ie)
{
	pimpl_->dialog(pimpl_->external).controller().showInset(ie);
}


void Dialogs::showFile(string const & f)
{
	pimpl_->dialog(pimpl_->file).controller().showFile(f);
}


void Dialogs::showFloat(InsetFloat * ifl)
{
	pimpl_->dialog(pimpl_->floats).controller().showInset(ifl);
}


void Dialogs::showForks()
{
	pimpl_->dialog(pimpl_->forks).controller().show();
}


void Dialogs::showGraphics(InsetGraphics * ig)
{
	pimpl_->dialog(pimpl_->graphics).controller().showInset(ig);
}


void Dialogs::showInclude(InsetInclude * ii)
{
	pimpl_->dialog(pimpl_->include).controller().showInset(ii);
}


void Dialogs::showIndex(InsetCommand * ic)
{
	pimpl_->dialog(pimpl_->index).controller().showInset(ic);
}


void Dialogs::createIndex()
{
	pimpl_->dialog(pimpl_->index).controller().createInset(string());
}


void Dialogs::showLogFile()
{
	pimpl_->dialog(pimpl_->logfile).controller().show();
}
