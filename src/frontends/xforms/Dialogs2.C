/**
 * \file xforms/Dialogs.C
 * Copyright 1995 Matthias Ettrich
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs_impl.h"


void Dialogs::showAboutlyx()
{
	pimpl_->aboutlyx.controller().show();
}


void Dialogs::showBibitem(InsetCommand * ic)
{
	pimpl_->bibitem.controller().showInset(ic);
}


void Dialogs::showBibtex(InsetCommand * ic)
{
	pimpl_->bibtex.controller().showInset(ic);
}


void Dialogs::showCharacter()
{
	pimpl_->character.controller().show();
}


void Dialogs::setUserFreeFont()
{
	pimpl_->character.controller().apply();
}


void Dialogs::showCitation(InsetCommand * ic)
{
	pimpl_->citation.controller().showInset(ic);
}


void Dialogs::createCitation(string const & s)
{
	pimpl_->citation.controller().createInset(s);
}


void Dialogs::showDocument()
{
	pimpl_->document.show();
}


void Dialogs::showError(InsetError * ie)
{
	pimpl_->error.controller().showInset(ie);
}


void Dialogs::showERT(InsetERT * ie)
{
	pimpl_->ert.controller().showInset(ie);
}


void Dialogs::updateERT(InsetERT * ie)
{
	pimpl_->ert.controller().showInset(ie);
}


void Dialogs::showExternal(InsetExternal * ie)
{
	pimpl_->external.controller().showInset(ie);
}


void Dialogs::showFile(string const & f)
{
	pimpl_->file.controller().showFile(f);
}


void Dialogs::showFloat(InsetFloat * ifl)
{
	pimpl_->floats.controller().showInset(ifl);
}


void Dialogs::showForks()
{
	pimpl_->forks.controller().show();
}


void Dialogs::showGraphics(InsetGraphics * ig)
{
	pimpl_->graphics.controller().showInset(ig);
}


void Dialogs::showInclude(InsetInclude * ii)
{
	pimpl_->include.controller().showInset(ii);
}


void Dialogs::showIndex(InsetCommand * ic)
{
	pimpl_->index.controller().showInset(ic);
}


void Dialogs::createIndex()
{
	pimpl_->index.controller().createInset(string());
}


void Dialogs::showLogFile()
{
	pimpl_->logfile.controller().show();
}


void Dialogs::showMathPanel()
{
	pimpl_->mathpanel.show();
}


void Dialogs::showMinipage(InsetMinipage * im)
{
	pimpl_->minipage.controller().showInset(im);
}


void Dialogs::updateMinipage(InsetMinipage * im)
{
	pimpl_->minipage.controller().showInset(im);
}


void Dialogs::showParagraph()
{
	pimpl_->paragraph.controller().show();
}


void Dialogs::updateParagraph()
{
	pimpl_->paragraph.controller().changedParagraph();
}


void Dialogs::showPreamble()
{
	pimpl_->preamble.controller().show();
}


void Dialogs::showPreferences()
{
	pimpl_->preferences.show();
}


void Dialogs::showPrint()
{
	pimpl_->print.controller().show();
}


void Dialogs::showRef(InsetCommand * ic)
{
	pimpl_->ref.controller().showInset(ic);
}


void Dialogs::createRef(string const & s)
{
	pimpl_->ref.controller().createInset(s);
}


void Dialogs::showSearch()
{
	pimpl_->search.controller().show();
}


void Dialogs::showSendto()
{
	pimpl_->sendto.controller().show();
}


void Dialogs::showSpellchecker()
{
	pimpl_->spellchecker.controller().show();
}


void Dialogs::showTabular(InsetTabular * it)
{
	pimpl_->tabular.showInset(it);
}


void Dialogs::updateTabular(InsetTabular * it)
{
	pimpl_->tabular.updateInset(it);
}


void Dialogs::showTabularCreate()
{
	pimpl_->tabularcreate.controller().show();
}


void Dialogs::showTexinfo()
{
	pimpl_->texinfo.controller().show();
}


#ifdef HAVE_LIBAIKSAURUS

void Dialogs::showThesaurus(string const & s)
{
	pimpl_->thesaurus.controller().showEntry(s);
}

#else

void Dialogs::showThesaurus(string const &)
{}

#endif


void Dialogs::showTOC(InsetCommand * ic)
{
	pimpl_->toc.controller().showInset(ic);
}


void Dialogs::createTOC(string const & s)
{
	pimpl_->toc.controller().createInset(s);
}


void Dialogs::showUrl(InsetCommand * ic)
{
	pimpl_->url.controller().showInset(ic);
}


void Dialogs::createUrl(string const & s)
{
	pimpl_->url.controller().createInset(s);
}


void Dialogs::showVCLogFile()
{
	pimpl_->vclogfile.controller().show();
}


void Dialogs::showWrap(InsetWrap * iw)
{
	pimpl_->wrap.controller().showInset(iw);
}
