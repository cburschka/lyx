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

#include <gtk--/radiobutton.h>
#include <gtk--/button.h>

FormERT::FormERT(ControlERT & c)
	: FormCB<ControlERT>(c, "FormERT")
{}


FormERT::~FormERT()
{}


void FormERT::build()
{
	// Connect the buttons.
	ok_btn()->clicked.connect(SigC::slot(this, &FormERT::OKClicked));
	cancel_btn()->clicked.connect(SigC::slot(this, &FormERT::CancelClicked));
	apply_btn()->clicked.connect(SigC::slot(this, &FormERT::ApplyClicked));

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


void FormERT::connect_signals()
{
	slot_open = open()->clicked.connect(SigC::slot(this, &FormERT::InputChanged));
	slot_collapsed = collapsed()->clicked.connect(SigC::slot(this, &FormERT::InputChanged));
	slot_inlined = inlined()->clicked.connect(SigC::slot(this, &FormERT::InputChanged));
}


void FormERT::disconnect_signals()
{
	slot_open.disconnect();
	slot_collapsed.disconnect();
	slot_inlined.disconnect();
}


void FormERT::apply()
{

	if (open()->get_active())
		controller().params().status = InsetERT::Open;
	else if (collapsed()->get_active())
		controller().params().status = InsetERT::Collapsed;
	else 
		controller().params().status = InsetERT::Inlined;

}


void FormERT::update()
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

bool FormERT::validate() const
{
	return true;
}


Gtk::Button * FormERT::ok_btn() const 
{
        return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * FormERT::apply_btn() const 
{
        return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * FormERT::cancel_btn() const 
{
        return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::RadioButton * FormERT::open() const 
{
        return getWidget<Gtk::RadioButton>("r_open");
}
Gtk::RadioButton * FormERT::collapsed() const 
{
        return getWidget<Gtk::RadioButton>("r_collapsed");
}
Gtk::RadioButton * FormERT::inlined() const 
{
        return getWidget<Gtk::RadioButton>("r_inlined");
}
