/**
 * \file GERT.C
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
	close_btn()->signal_clicked().connect(SigC::slot(*this, &GERT::OKClicked));


	// Make sure everything is in the correct state.
	bc().refresh();
	
	// Manage the read-only aware widgets.
	bc().addReadOnly(open());
	bc().addReadOnly(inlined());
	bc().addReadOnly(collapsed());

}


void GERT::connect_signals()
{
	slot_open = open()->signal_clicked().connect(
		SigC::slot(*this, &GERT::ApplyClicked)
		);
	slot_collapsed = collapsed()->signal_clicked().connect(
		SigC::slot(*this, &GERT::ApplyClicked)
		);
	slot_inlined = inlined()->signal_clicked().connect(
		SigC::slot(*this, &GERT::ApplyClicked)
		);
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

Gtk::Button * GERT::close_btn() const 
{
        return getWidget<Gtk::Button>("r_close_btn");
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
