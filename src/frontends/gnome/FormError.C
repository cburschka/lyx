/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= 
 *
 * \author Baruch Even
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gnomeBC.h"
#include "FormError.h"

#include <gtk--/button.h>
#include <gtk--/text.h>

FormError::FormError(ControlError & c)
	: FormCB<ControlError>(c, "diaerror.glade", "DiaError")
{}


void FormError::build()
{
	// Connect the buttons.
	close_btn()->clicked.connect(SigC::slot(this, &FormError::CloseClicked));

	// Manage the buttons state
	bc().setCancel(close_btn());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void FormError::update()
{
	textarea()->insert(controller().params());
}


Gtk::Button * FormError::close_btn() const
{
	return getWidget<Gtk::Button>("button_close");
}


Gtk::Text * FormError::textarea() const
{
	return getWidget<Gtk::Text>("textarea");
}
