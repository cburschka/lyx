/**
 * \file qt2/Dialogs2.C
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

#include "Dialogs_impl.h"


void Dialogs::showAboutlyx()
{
	pimpl_->aboutlyx.controller().show();
}


void Dialogs::showMergeChanges()
{
	pimpl_->changes.controller().show();
}


void Dialogs::showCharacter()
{
	pimpl_->character.controller().show();
}


void Dialogs::setUserFreeFont()
{
	pimpl_->character.controller().apply();
}


void Dialogs::showDocument()
{
	pimpl_->document.controller().show();
}


void Dialogs::showFile(string const & f)
{
	pimpl_->file.controller().showFile(f);
}


void Dialogs::showForks()
{}


void Dialogs::showGraphics(InsetGraphics * ig)
{
	pimpl_->graphics.controller().showInset(ig);
}


void Dialogs::showLogFile()
{
	pimpl_->logfile.controller().show();
}


void Dialogs::showMathPanel()
{
	// FIXME FIXME FIXME
	extern void createMathPanel();

	createMathPanel();
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
	// FIXME
}


void Dialogs::showPreferences()
{
	pimpl_->prefs.controller().show();
}


void Dialogs::showPrint()
{
	pimpl_->print.controller().show();
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
	pimpl_->tabular.controller().showInset(it);
}


void Dialogs::updateTabular(InsetTabular * it)
{
	pimpl_->tabular.controller().updateInset(it);
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


void Dialogs::showVCLogFile()
{
	pimpl_->vclogfile.controller().show();
}
