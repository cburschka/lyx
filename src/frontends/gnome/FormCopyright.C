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

#include "debug.h"

#include "gnomeBC.h"
#include "FormCopyright.h"
#include "gnome_helpers.h"

#include <gtk--/button.h>
#include <gtk--/label.h>


FormCopyright::FormCopyright(ControlCopyright & c)
	: FormCB<ControlCopyright>(c, "diahelpcopyright.glade", "DiaHelpCopyright")
{
}


void FormCopyright::build()
{
	ok()->clicked.connect(SigC::slot(this, &FormCopyright::CancelClicked));
	copyright()->set(controller().getCopyright());
	license()->set(controller().getLicence());
	disclaimer()->set(controller().getDisclaimer());
}


Gtk::Button * FormCopyright::ok()
{
	return getWidget<Gtk::Button>("copyright_button_ok");
}

Gtk::Label * FormCopyright::disclaimer()
{
	return getWidget<Gtk::Label>("copyright_disclaimer");
}

Gtk::Label * FormCopyright::copyright()
{
	return getWidget<Gtk::Label>("copyright_copyright");
}

Gtk::Label * FormCopyright::license()
{
	return getWidget<Gtk::Label>("copyright_license");
}
