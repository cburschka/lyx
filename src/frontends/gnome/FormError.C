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
	: FormCB<ControlError>(c, "FormError")
{}


void FormError::build()
{
	// Connect the buttons.
	button_close()->clicked.connect(SigC::slot(this, &FormError::CloseClicked));

	// Manage the buttons state
	bc().setCancel(button_close());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void FormError::update()
{
	textarea()->insert(controller().params());
}

Gtk::Button * FormError::button_close() const 
{
        return getWidget<Gtk::Button>("r_button_close");
}

Gtk::Text * FormError::textarea() const 
{
        return getWidget<Gtk::Text>("r_textarea");
}
