/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= 
 *
 * \author Baruch Even
 * \author Michael Koziarski
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "gnomeBC.h"
#include "GUrl.h"

#include <gtk--/entry.h>
#include <gtk--/checkbutton.h>

GUrl::GUrl(ControlUrl & c)
	: FormCB<ControlUrl>(c, "GUrl")
{}


GUrl::~GUrl()
{}


void GUrl::build()
{
	// Connect the buttons.
	ok_btn()->clicked.connect(SigC::slot(this, &GUrl::OKClicked));
	cancel_btn()->clicked.connect(SigC::slot(this, &GUrl::CancelClicked));
	apply_btn()->clicked.connect(SigC::slot(this, &GUrl::ApplyClicked));
	restore_btn()->clicked.connect(SigC::slot(this, &GUrl::RestoreClicked));

	// Manage the buttons state
	bc().setOK(ok_btn());
	bc().setCancel(cancel_btn());
	bc().setApply(apply_btn());
	bc().setRestore(restore_btn());

	// Manage the read-only aware widgets.
	bc().addReadOnly(html_cb());
	bc().addReadOnly(name());
	bc().addReadOnly(url());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void GUrl::connect_signals()
{
	// Get notifications on input change
	slot_url_ = url()->changed.connect(SigC::slot(this, &GUrl::InputChanged));
	slot_name_ = name()->changed.connect(SigC::slot(this, &GUrl::InputChanged));
	slot_html_ = html_cb()->toggled.connect(SigC::slot(this, &GUrl::InputChanged));
}


void GUrl::disconnect_signals()
{
	slot_url_.disconnect();
	slot_name_.disconnect();
	slot_html_.disconnect();
}


void GUrl::apply()
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


void GUrl::update()
{
	disconnect_signals();
	
	url()->set_text(controller().params().getContents());
	name()->set_text(controller().params().getOptions());

	html_cb()->set_active("url" != controller().params().getCmdName());

	connect_signals();
}


bool GUrl::validate() const
{
	return !url()->get_text().empty() && !name()->get_text().empty();
}

Gtk::Button * GUrl::restore_btn() const 
{
        return getWidget<Gtk::Button>("r_restore_btn");
}
Gtk::Button * GUrl::ok_btn() const 
{
        return getWidget<Gtk::Button>("r_ok_btn");
}
Gtk::Button * GUrl::apply_btn() const 
{
        return getWidget<Gtk::Button>("r_apply_btn");
}
Gtk::Button * GUrl::cancel_btn() const 
{
        return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::Entry * GUrl::url() const 
{
        return getWidget<Gtk::Entry>("r_url");
}
Gtk::Entry * GUrl::name() const 
{
        return getWidget<Gtk::Entry>("r_name");
}
Gtk::CheckButton * GUrl::html_cb() const 
{
        return getWidget<Gtk::CheckButton>("r_html_cb");
}


