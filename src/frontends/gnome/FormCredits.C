// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= 
 *
 * \author Michael Koziarski <michael@koziarski.org>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gnomeBC.h"
#include "FormCredits.h"

#include <gtk--/button.h>
#include <gtk--/text.h>

FormCredits::FormCredits(ControlCredits & c)
	: FormCB<ControlCredits>(c, "diahelpcredits.glade", "DiaHelpCredits")
{
}


void FormCredits::build()
{
	// It is better to show an OK button, but the policy require that we
	// get a click on "Cancel"
	ok()->clicked.connect(SigC::slot(this, &FormCredits::CancelClicked));

	std::stringstream ss;
	text()->insert(controller().getCredits(ss).str());
}


Gtk::Text * FormCredits::text()
{
	return getWidget<Gtk::Text>("credits_text");
}

Gtk::Button * FormCredits::ok()
{
	return getWidget<Gtk::Button>("credits_button_ok");
}
