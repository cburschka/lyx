/**
 * ile GLog.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Michael Koziarski <michael@koziarski.org>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include <fstream>

#include "gnomeBC.h"
#include "GLog.h"

#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <gtkmm/dialog.h>

GLog::GLog(ControlLog & c)
	: FormCB<ControlLog>(c, "GLog")
{}


GLog::~GLog()
{}


void GLog::build()
{
	// Connect the buttons.
	close_btn()->signal_clicked().connect(SigC::slot(*this, &GLog::CancelClicked));
	refresh_btn()->signal_clicked().connect(SigC::slot(*this, &GLog::update));

	// Manage the buttons state
	bc().setCancel(close_btn());
	bc().refresh();
}

void GLog::apply()
{}


void GLog::update()
{
	using namespace std;
	pair<Buffer::LogType, string> const logfile = controller().logfile();

	if (logfile.first == Buffer::buildlog)
		dialog()->set_title(_("Build log"));
	else
		dialog()->set_title(_("LaTeX log"));

	log_text()->get_buffer()->set_text("");

	ifstream ifstr(logfile.second.c_str());
	if (!ifstr) {
		if (logfile.first == Buffer::buildlog)
			log_text()->get_buffer()->set_text(_("No build log file found"));
		else
			log_text()->get_buffer()->set_text(_("No LaTeX log file found"));
		return;
	}

	string text;
	string line;

	while (getline(ifstr, line))
		text += line + "\n";

	log_text()->get_buffer()->set_text(text.c_str());
}


Gtk::Button * GLog::refresh_btn() const 
{
        return getWidget<Gtk::Button>("r_refresh_btn");
}
Gtk::Button * GLog::close_btn() const 
{
        return getWidget<Gtk::Button>("r_close_btn");
}
Gtk::TextView * GLog::log_text() const 
{
        return getWidget<Gtk::TextView>("r_log_text");
}
