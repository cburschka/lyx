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


void Dialogs::showDocument()
{
	pimpl_->document.controller().show();
}


void Dialogs::showForks()
{}


void Dialogs::showMathPanel()
{
	// FIXME FIXME FIXME
	extern void createMathPanel();

	createMathPanel();
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


void Dialogs::showTexinfo()
{
	pimpl_->texinfo.controller().show();
}
