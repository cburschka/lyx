/**
 * \file GTabularCreate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Michael Koziarski
 *
 * Full author contact details are available in file CREDITS
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include <utility>

#include "gnomeBC.h"
#include "GTabularCreate.h"

#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>

GTabularCreate::GTabularCreate(ControlTabularCreate & c)
	: FormCB<ControlTabularCreate>(c, "GTabularCreate")
{}


GTabularCreate::~GTabularCreate()
{}


void GTabularCreate::build()
{
	// Connect the buttons.
	ok_btn()->signal_clicked().connect(SigC::slot(*this, 
	                                 &GTabularCreate::OKClicked));
	cancel_btn()->signal_clicked().connect(SigC::slot(*this, 
	                                 &GTabularCreate::CancelClicked));
	apply_btn()->signal_clicked().connect(SigC::slot(*this, 
                                     &GTabularCreate::ApplyClicked));
	
	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void GTabularCreate::apply()
{
	unsigned int ysize = (unsigned int)(rows()->get_value_as_int());
	unsigned int xsize = (unsigned int)(cols()->get_value_as_int());

	controller().params() = std::make_pair(xsize, ysize);
}


void GTabularCreate::update()
{}


bool GTabularCreate::validate() const
{
	return ( rows()->get_value_as_int() > 0 ) && 
	       ( cols()->get_value_as_int() > 0 );
}


Gtk::Button * GTabularCreate::ok_btn() const 
{
        return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * GTabularCreate::apply_btn() const 
{
        return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * GTabularCreate::cancel_btn() const 
{
        return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::SpinButton * GTabularCreate::rows() const 
{
        return getWidget<Gtk::SpinButton>("r_rows");
}
Gtk::SpinButton * GTabularCreate::cols() const 
{
        return getWidget<Gtk::SpinButton>("r_cols");
}

