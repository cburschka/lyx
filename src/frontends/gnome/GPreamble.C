/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= 
 *
 * \author Michael Koziarski 
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gnomeBC.h"
#include "GPreamble.h"

#include <gtk--/text.h>
#include <gtk--/button.h>

GPreamble::GPreamble(ControlPreamble & c)
	: FormCB<ControlPreamble>(c, "GPreamble")
{}


GPreamble::~GPreamble()
{}


void GPreamble::build()
{
	// Connect the buttons.
	ok_btn()->clicked.connect(SigC::slot(this, &GPreamble::OKClicked));
	cancel_btn()->clicked.connect(SigC::slot(this, &GPreamble::CancelClicked));
	apply_btn()->clicked.connect(SigC::slot(this, &GPreamble::ApplyClicked));
	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());
	bc().refresh();
}

void GPreamble::apply()
{
	controller().params() = preamble()->get_chars(0,-1);
}


void GPreamble::update()
{
	disconnect_signals();
	preamble()->set_point(0);
	preamble()->forward_delete(preamble()->get_length());
	preamble()->insert(controller().params());
	connect_signals();
}

void GPreamble::connect_signals()
{
	slot_preamble_ = preamble()->changed.connect(SigC::slot(this, &GPreamble::InputChanged));
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
Gtk::Text * GPreamble::preamble() const 
{
        return getWidget<Gtk::Text>("r_preamble");
}
