/**
 * \file qt2/Dialogs2.C
 * Copyright 1995 Matthias Ettrich
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "Dialogs.h"
#include "controllers/GUI.h"
#include "ButtonController.h"

#include "ControlDocument.h"
#include "ControlPrint.h"
#include "ControlSpellchecker.h"

#include "QDocument.h"
#include "QDocumentDialog.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QPrefs.h"
#include "QPrefsDialog.h"
#include "QPrint.h"
#include "QLPrintDialog.h"
#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"




typedef GUI<ControlDocument, QDocument, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
DocumentDialog;

typedef GUI<ControlPrefs, QPrefs, OkApplyCancelPolicy, Qt2BC>
PrefsDialog;

typedef GUI<ControlPrint, QPrint, OkApplyCancelPolicy, Qt2BC>
PrintDialog;

typedef GUI<ControlSpellchecker, QSpellchecker, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SpellcheckerDialog;

struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	DocumentDialog      document;
	PrefsDialog         prefs;
	PrintDialog         print;
	SpellcheckerDialog  spellchecker;
};


Dialogs::Impl::Impl(LyXView & lv, Dialogs & d)
	: document(lv, d),
	  prefs(lv, d),
	  print(lv, d),
	  spellchecker(lv, d)
{}


void Dialogs::init_pimpl()
{
	pimpl_ = new Impl(lyxview_, *this);
}


Dialogs::~Dialogs()
{
	delete pimpl_;
}


void Dialogs::showDocument()
{
	pimpl_->document.controller().show();
}


void Dialogs::showPreamble()
{
	pimpl_->document.controller().show();
	// Oh Angus, won't you help a poor child ?
	//pimpl_->document.view()->showPreamble();
}


void Dialogs::showPreferences()
{
	pimpl_->prefs.controller().show();
}


void Dialogs::showPrint()
{
	pimpl_->print.controller().show();
}


void Dialogs::showSpellchecker()
{
	pimpl_->spellchecker.controller().show();
}
