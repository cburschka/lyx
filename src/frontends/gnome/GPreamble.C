/**
 * \file GPreamble.C
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

#include "gnomeBC.h"
#include "GPreamble.h"

#include <gtkmm/textview.h>
#include <gtkmm/button.h>

GPreamble::GPreamble(ControlPreamble & c)
	: GnomeCB<ControlPreamble>(c, "GPreamble")
{}


GPreamble::~GPreamble()
{}


void GPreamble::build()
{
	// Connect the buttons.
	ok_btn()->signal_clicked().connect(SigC::slot(*this, &GPreamble::OKClicked));
	cancel_btn()->signal_clicked().connect(SigC::slot(*this, &GPreamble::CancelClicked));
	apply_btn()->signal_clicked().connect(SigC::slot(*this, &GPreamble::ApplyClicked));
	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());
	bc().refresh();
}

void GPreamble::apply()
{
	controller().params() = preamble()->get_buffer()->get_text(preamble()->get_buffer()->get_start_iter(),
								   preamble()->get_buffer()->get_end_iter(),
								   false);
}


void GPreamble::update()
{
	disconnect_signals();
	preamble()->get_buffer()->set_text(controller().params());
	connect_signals();
}

void GPreamble::connect_signals()
{
	slot_preamble_ = preamble()->
		get_buffer()->
		signal_changed().connect(SigC::slot(*this, &GPreamble::InputChanged));
}

void GPreamble::disconnect_signals() 
{
	slot_preamble_.disconnect();
}

Gtk::Button * GPreamble::ok_btn() const 
{
        return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * GPreamble::apply_btn() const 
{
        return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * GPreamble::cancel_btn() const 
{
        return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::TextView * GPreamble::preamble() const 
{
        return getWidget<Gtk::TextView>("r_preamble");
}
