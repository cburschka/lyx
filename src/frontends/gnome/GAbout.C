/**
 * \file GAbout.C
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

#include "support/lstrings.h"
#include "Lsstream.h"


#include "gnomeBC.h"
#include "GAbout.h"

#include <gtkmm/button.h>
#include <gtkmm/textview.h>

GAbout::GAbout(ControlAboutlyx & c)
	: FormCB<ControlAboutlyx>(c, "GAbout")
{}


GAbout::~GAbout()
{}


void GAbout::build()
{
	// Connect the buttons.
	close_btn()->signal_clicked().connect(SigC::slot(*this, &GAbout::CancelClicked));

	// Manage the buttons state
	bc().setCancel(close_btn());
	bc().refresh();
}

void GAbout::apply()
{}


void GAbout::update()
{
	using namespace std;

	string cr;
	cr += controller().getCopyright();
	cr += "\n";
	cr += controller().getLicense();
	cr += "\n";
	cr += controller().getDisclaimer();
	copyright()->get_buffer()->set_text(cr);

	

	version()->set_text(controller().getVersion());

	stringstream in;
	controller().getCredits(in);

	istringstream ss(in.str().c_str());

	string s;
	string out;
	Gtk::TextIter  e;

	while (getline(ss, s)) {
	
		if (prefixIs(s, "@b"))
			out += s.substr(2);
		else if (prefixIs(s, "@i"))
			out += s.substr(2);
		else
			out += s.substr(2);

		out += "\n";
	}
	credits()->get_buffer()->set_text(out);
}



Gtk::Button * GAbout::close_btn() const 
{
        return getWidget<Gtk::Button>("r_close_btn");
}
Gtk::Label * GAbout::version() const 
{
        return getWidget<Gtk::Label>("r_version");
}
Gtk::TextView * GAbout::credits() const 
{
        return getWidget<Gtk::TextView>("r_credits");
}
Gtk::TextView * GAbout::copyright() const 
{
        return getWidget<Gtk::TextView>("r_copyright");
}
