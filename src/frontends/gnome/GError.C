/**
 * \file GError.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Michael Koziarski
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gnomeBC.h"
#include "GError.h"

#include <gtkmm/button.h>
#include <gtkmm/textview.h>

GErrorDialog::GErrorDialog(ControlError & c)
	: FormCB<ControlError>(c, "GError")
{}


void GErrorDialog::build()
{
	// Connect the buttons.
	button_close()->signal_clicked().connect(
		SigC::slot(*this, &GErrorDialog::CloseClicked)
		);

	// Manage the buttons state
	bc().setCancel(button_close());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void GErrorDialog::update()
{
	textarea()->get_buffer()->set_text(controller().params());
}

Gtk::Button * GErrorDialog::button_close() const 
{
        return getWidget<Gtk::Button>("r_button_close");
}

Gtk::TextView * GErrorDialog::textarea() const 
{
        return getWidget<Gtk::TextView>("r_textarea");
}
