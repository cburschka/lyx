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
#include <utility>

#include "gnomeBC.h"
#include "FormTabularCreate.h"

#include <gtk--/spinbutton.h>
#include <gtk--/button.h>

FormTabularCreate::FormTabularCreate(ControlTabularCreate & c)
	: FormCB<ControlTabularCreate>(c, "diainserttabular.glade", 
                                      "DiaInsertTabular")
{}


FormTabularCreate::~FormTabularCreate()
{
	// Note that there is no need to destroy the class itself, it seems
	// like everything is managed inside it. Deleting the class itself will
	// a crash at the end of the program.
	//dialog_->destroy();
}


void FormTabularCreate::build()
{
	// Connect the buttons.
	ok_btn()->clicked.connect(SigC::slot(this, 
	                                 &FormTabularCreate::OKClicked));
	cancel_btn()->clicked.connect(SigC::slot(this, 
	                                 &FormTabularCreate::CancelClicked));
	apply_btn()->clicked.connect(SigC::slot(this, 
                                     &FormTabularCreate::ApplyClicked));
	
	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void FormTabularCreate::apply()
{
	unsigned int ysize = (unsigned int)(rows_spin()->get_value_as_int());
	unsigned int xsize = (unsigned int)(columns_spin()->get_value_as_int());

	controller().params() = std::make_pair(xsize, ysize);
}


void FormTabularCreate::update()
{
}


bool FormTabularCreate::validate() const
{
	// Always valid! (not really so, needs fixing).
	return true;
}


Gtk::SpinButton * FormTabularCreate::rows_spin() const
{
	return getWidget<Gtk::SpinButton>("tabular_spin_rows");
}


Gtk::SpinButton * FormTabularCreate::columns_spin() const
{
	return getWidget<Gtk::SpinButton>("tabular_spin_columns");
}


Gtk::Button * FormTabularCreate::ok_btn() const
{
	return getWidget<Gtk::Button>("button_ok");
}


Gtk::Button * FormTabularCreate::cancel_btn() const
{
	return getWidget<Gtk::Button>("button_cancel");
}


Gtk::Button * FormTabularCreate::apply_btn() const
{
	return getWidget<Gtk::Button>("button_apply");
}
