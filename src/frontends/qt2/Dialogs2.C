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

#include "Dialogs.h"
#include "controllers/GUI.h"
#include "ButtonController.h"

#include "ControlDocument.h"
#include "ControlForks.h"
#include "ControlPrefs.h"
#include "ControlPrint.h"
#include "ControlSearch.h"
#include "ControlSendto.h"
#include "ControlSpellchecker.h"

#include "QCharacter.h"
#include "QCharacterDialog.h"
#include "QDocument.h"
#include "QDocumentDialog.h"
//#include "QForks.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QPrefs.h"
#include "QPrefsDialog.h"
#include "QPrint.h"
#include "QLPrintDialog.h"
#include "QSearch.h"
#include "QSearchDialog.h"
#include "QSendto.h"
#include "QSendtoDialog.h"
#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"

#include "Qt2BC.h"



typedef GUI<ControlDocument, QDocument, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
DocumentDialog;

typedef GUI<ControlPrefs, QPrefs, OkApplyCancelPolicy, Qt2BC>
PrefsDialog;

typedef GUI<ControlPrint, QPrint, OkApplyCancelPolicy, Qt2BC>
PrintDialog;

typedef GUI<ControlSearch, QSearch, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SearchDialog;

typedef GUI<ControlSendto, QSendto, OkApplyCancelPolicy, Qt2BC>
SendtoDialog;

typedef GUI<ControlSpellchecker, QSpellchecker, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SpellcheckerDialog;

struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	DocumentDialog      document;
	PrefsDialog         prefs;
	PrintDialog         print;
	SearchDialog        search;
	SendtoDialog        sendto;
	SpellcheckerDialog  spellchecker;
};


Dialogs::Impl::Impl(LyXView & lv, Dialogs & d)
	: document(lv, d),
	  prefs(lv, d),
	  print(lv, d),
	  search(lv, d),
	  sendto(lv, d),
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


void Dialogs::showForks()
{}


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
