/**
 * \file GChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Michael Koziarski
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "gnomeBC.h"
#include "GChanges.h"

#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <string>

GChanges::GChanges()
	: GnomeCB<ControlChanges>("GChanges")
{}


GChanges::~GChanges()
{}


void GChanges::build()
{
	// Connect the buttons.
	cancel_btn()->signal_clicked().connect(SigC::slot(*this, &GChanges::CancelClicked));
	accept_btn()->signal_clicked().connect(SigC::slot(*this, &GChanges::accept));
	reject_btn()->signal_clicked().connect(SigC::slot(*this, &GChanges::reject));

	bc().setCancel(cancel_btn());

	// Manage the read-only aware widgets.
	bc().addReadOnly(accept_btn());
	bc().addReadOnly(cancel_btn());
	bc().addReadOnly(reject_btn());

	// Make sure everything is in the correct state.
	bc().refresh();
}


void GChanges::connect_signals()
{}


void GChanges::disconnect_signals()
{}

void GChanges::accept()
{
	controller().accept();
}

void GChanges::reject()
{
	controller().reject();
}

void GChanges::apply()
{}


void GChanges::update()
{
	using std::string;
	disconnect_signals();
	controller().find();

	string text;
	string author(controller().getChangeAuthor());
	string date(controller().getChangeDate());

	if (!author.empty())
		text += "Change by " + author + "\n\n";
	if (!date.empty())
		text += "Change made at " + date + "\n";

	changes()->get_buffer()->set_text(Glib::locale_to_utf8(text));

	connect_signals();
}


bool GChanges::validate() const
{
	return true;
}
Gtk::Button * GChanges::next_btn() const
{
	return getWidget<Gtk::Button>("r_next_btn");
}
Gtk::Button * GChanges::cancel_btn() const
{
	return getWidget<Gtk::Button>("r_cancel_btn");
}
Gtk::TextView * GChanges::changes() const
{
	return getWidget<Gtk::TextView>("r_changes");
}
Gtk::Button * GChanges::accept_btn() const
{
	return getWidget<Gtk::Button>("r_accept_btn");
}
Gtk::Button * GChanges::reject_btn() const
{
	return getWidget<Gtk::Button>("r_reject_btn");
}
