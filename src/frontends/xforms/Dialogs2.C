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

#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "forms/form_preamble.h"

#include "ControlPrefs.h"
#include "FormPreferences.h"
#include "FormColorpicker.h"
#include "forms/form_preferences.h"

#include "ControlSpellchecker.h"
#include "FormSpellchecker.h"
#include "forms/form_spellchecker.h"

typedef GUI<ControlPreamble, FormPreamble, NoRepeatedApplyReadOnlyPolicy, xformsBC>
PreambleDialog;

typedef GUI<ControlPrefs, FormPreferences, OkApplyCancelPolicy, xformsBC>
PreferencesDialog;

typedef GUI<ControlSpellchecker, FormSpellchecker, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SpellcheckerDialog;


struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	PreambleDialog      preamble;
	PreferencesDialog   preferences;
	SpellcheckerDialog  spellchecker;
};


Dialogs::Impl::Impl(LyXView & lv, Dialogs & d)
	: preamble(lv, d),
	  preferences(lv, d),
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


void Dialogs::showPreamble()
{
	pimpl_->preamble.controller().show();
}


void Dialogs::showPreferences()
{
	pimpl_->preferences.controller().show();
}


void Dialogs::showSpellchecker()
{
	pimpl_->spellchecker.controller().show();
}
