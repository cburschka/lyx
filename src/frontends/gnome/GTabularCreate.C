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
#include "GTabularCreate.h"

#include <gtk--/spinbutton.h>
#include <gtk--/button.h>

FormTabularCreate::FormTabularCreate(ControlTabularCreate & c)
	: FormCB<ControlTabularCreate>(c, "FormTabularCreate")
{}


FormTabularCreate::~FormTabularCreate()
{}


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
	unsigned int ysize = (unsigned int)(rows()->get_value_as_int());
	unsigned int xsize = (unsigned int)(cols()->get_value_as_int());

	controller().params() = std::make_pair(xsize, ysize);
}


void FormTabularCreate::update()
{}


bool FormTabularCreate::validate() const
{
	return ( rows()->get_value_as_int() > 0 ) && 
	       ( cols()->get_value_as_int() > 0 );
}


Gtk::Button * FormTabularCreate::ok_btn() const 
{
        return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * FormTabularCreate::apply_btn() const 
{
        return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * FormTabularCreate::cancel_btn() const 
{
        return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::SpinButton * FormTabularCreate::rows() const 
{
        return getWidget<Gtk::SpinButton>("r_rows");
}
Gtk::SpinButton * FormTabularCreate::cols() const 
{
        return getWidget<Gtk::SpinButton>("r_cols");
}

