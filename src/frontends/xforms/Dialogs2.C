/**
 * \file xforms/Dialogs2.C
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

#include "xformsBC.h"
#include "xforms_helpers.h"

#include "ControlDocument.h"
#include "FormDocument.h"
#include "forms/form_document.h"

#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "forms/form_preamble.h"

#include "ControlPrefs.h"
#include "FormPreferences.h"
#include "FormColorpicker.h"
#include "forms/form_preferences.h"

#include "ControlPrint.h"
#include "FormPrint.h"
#include "forms/form_print.h"

#include "ControlSendto.h"
#include "FormSendto.h"
#include "forms/form_sendto.h"

#include "ControlSpellchecker.h"
#include "FormSpellchecker.h"
#include "forms/form_spellchecker.h"

typedef GUI<ControlDocument, FormDocument, NoRepeatedApplyReadOnlyPolicy, xformsBC>
DocumentDialog;

typedef GUI<ControlPreamble, FormPreamble, NoRepeatedApplyReadOnlyPolicy, xformsBC>
PreambleDialog;

typedef GUI<ControlPrefs, FormPreferences, OkApplyCancelPolicy, xformsBC>
PreferencesDialog;

typedef GUI<ControlPrint, FormPrint, OkApplyCancelPolicy, xformsBC>
PrintDialog;

typedef GUI<ControlSendto, FormSendto, OkApplyCancelPolicy, xformsBC>
SendtoDialog;

typedef GUI<ControlSpellchecker, FormSpellchecker, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SpellcheckerDialog;


struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	DocumentDialog      document;
	PreambleDialog      preamble;
	PreferencesDialog   preferences;
	PrintDialog         print;
	SendtoDialog        sendto;
	SpellcheckerDialog  spellchecker;
};


Dialogs::Impl::Impl(LyXView & lv, Dialogs & d)
	: document(lv, d),
	  preamble(lv, d),
	  preferences(lv, d),
	  print(lv, d),
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


void Dialogs::showPreamble()
{
	pimpl_->preamble.controller().show();
}


void Dialogs::showPreferences()
{
	pimpl_->preferences.controller().show();
}


void Dialogs::showPrint()
{
	pimpl_->print.controller().show();
}


void Dialogs::showSendto()
{
	pimpl_->sendto.controller().show();
}


void Dialogs::showSpellchecker()
{
	pimpl_->spellchecker.controller().show();
}
