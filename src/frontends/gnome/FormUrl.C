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
#include "FormUrl.h"

#include <gtk--/entry.h>
#include <gtk--/checkbutton.h>

FormUrl::FormUrl(ControlUrl & c)
	: FormCB<ControlUrl>(c, "FormUrl")
{}


FormUrl::~FormUrl()
{
	// Note that there is no need to destroy the class itself, it seems
	// like everything is managed inside it. Deleting the class itself will
	// a crash at the end of the program.
	//dialog_->destroy();
}


void FormUrl::build()
{
	// Connect the buttons.
	ok_btn()->clicked.connect(SigC::slot(this, &FormUrl::OKClicked));
	cancel_btn()->clicked.connect(SigC::slot(this, &FormUrl::CancelClicked));
	apply_btn()->clicked.connect(SigC::slot(this, &FormUrl::ApplyClicked));
	restore_btn()->clicked.connect(SigC::slot(this, &FormUrl::RestoreClicked));

	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());
	bc().setRestore(restore_btn());

	// Make sure everything is in the correct state.
	bc().refresh();

	// Manage the read-only aware widgets.
	bc().addReadOnly(html_cb());
	bc().addReadOnly(name());
	bc().addReadOnly(url());
}


void FormUrl::connect_signals()
{
	// Get notifications on input change
	slot_url_ = url()->changed.connect(SigC::slot(this, &FormUrl::InputChanged));
	slot_name_ = name()->changed.connect(SigC::slot(this, &FormUrl::InputChanged));
	slot_html_ = html_cb()->toggled.connect(SigC::slot(this, &FormUrl::InputChanged));
}


void FormUrl::disconnect_signals()
{
	slot_url_.disconnect();
	slot_name_.disconnect();
	slot_html_.disconnect();
}


void FormUrl::apply()
{
	disconnect_signals();
	controller().params().setContents(url()->get_text());
	controller().params().setOptions(name()->get_text());

	string cmdname("url");
	if (html_cb()->get_active())
		cmdname = "htmlurl";

	controller().params().setCmdName(cmdname);
	connect_signals();
}


void FormUrl::update()
{
	// Disconnect signals so we dont trigger the input changed state.
	// This avoids the problem of having the buttons enabled when the dialog
	// starts.
	disconnect_signals();

	url()->set_text(controller().params().getContents());
	name()->set_text(controller().params().getOptions());

	html_cb()->set_active("url" != controller().params().getCmdName());

	// Reconnect the signals.
	connect_signals();
}


bool FormUrl::validate() const
{
	return !url()->get_text().empty() && !name()->get_text().empty();
}

Gtk::Button * FormUrl::restore_btn() const
{
	return getWidget<Gtk::Button>("r_restore_btn");
}
Gtk::Button * FormUrl::ok_btn() const
{
	return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * FormUrl::apply_btn() const
{
	return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * FormUrl::cancel_btn() const
{
	return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::Entry * FormUrl::url() const
{
	return getWidget<Gtk::Entry>("r_url");
}
Gtk::Entry * FormUrl::name() const
{
	return getWidget<Gtk::Entry>("r_name");
}
Gtk::CheckButton * FormUrl::html_cb() const
{
	return getWidget<Gtk::CheckButton>("r_html_cb");
}
