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
#include "GERT.h"

#include <gtkmm/radiobutton.h>
#include <gtkmm/button.h>

GERT::GERT(ControlERT & c)
	: FormCB<ControlERT>(c, "GERT")
{}


GERT::~GERT()
{}


void GERT::build()
{
	// Connect the buttons.
	ok_btn()->signal_clicked().connect(SigC::slot(*this, &GERT::OKClicked));
	cancel_btn()->signal_clicked().connect(SigC::slot(*this, &GERT::CancelClicked));
	apply_btn()->signal_clicked().connect(SigC::slot(*this, &GERT::ApplyClicked));

	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());

	// Make sure everything is in the correct state.
	bc().refresh();
	
	// Manage the read-only aware widgets.
	bc().addReadOnly(open());
	bc().addReadOnly(inlined());
	bc().addReadOnly(collapsed());

}


void GERT::connect_signals()
{
	slot_open = open()->signal_clicked().connect(SigC::slot(*this, &GERT::InputChanged));
	slot_collapsed = collapsed()->signal_clicked().connect(SigC::slot(*this, &GERT::InputChanged));
	slot_inlined = inlined()->signal_clicked().connect(SigC::slot(*this, &GERT::InputChanged));
}


void GERT::disconnect_signals()
{
	slot_open.disconnect();
	slot_collapsed.disconnect();
	slot_inlined.disconnect();
}


void GERT::apply()
{

	if (open()->get_active())
		controller().params().status = InsetERT::Open;
	else if (collapsed()->get_active())
		controller().params().status = InsetERT::Collapsed;
	else 
		controller().params().status = InsetERT::Inlined;

}


void GERT::update()
{
	disconnect_signals();
	switch (controller().params().status) {
	case InsetERT::Open:
		open()->set_active(true);
		break;
	case InsetERT::Collapsed:
		collapsed()->set_active(true);
		break;
	case InsetERT::Inlined:
		inlined()->set_active(true);
		break;
	}
	connect_signals();

}

bool GERT::validate() const
{
	return true;
}


Gtk::Button * GERT::ok_btn() const 
{
        return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * GERT::apply_btn() const 
{
        return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * GERT::cancel_btn() const 
{
        return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::RadioButton * GERT::open() const 
{
        return getWidget<Gtk::RadioButton>("r_open");
}
Gtk::RadioButton * GERT::collapsed() const 
{
        return getWidget<Gtk::RadioButton>("r_collapsed");
}
Gtk::RadioButton * GERT::inlined() const 
{
        return getWidget<Gtk::RadioButton>("r_inlined");
}
