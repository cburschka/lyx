/**
 * \file GAbout.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Michael Koziarski
 *
 * Full author contact details are available in file CREDITS
 */


#include <config.h>

#include "support/lstrings.h"
#include "Lsstream.h"

#include "gnome_helpers.h"
#include "gnomeBC.h"
#include "GAbout.h"

#include <gtkmm/button.h>
#include <gtkmm/textview.h>

GAbout::GAbout()
	: GnomeCB<ControlAboutlyx>("GAbout")
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

	istringstream ss(in.str());

	string s;
	Glib::RefPtr<Gtk::TextBuffer> buf = credits()->get_buffer();

	addDefaultTags(buf);
	while (getline(ss, s)) {

		if (prefixIs(s, "@b")) 
			buf->insert_with_tag(buf->end(), 
					     Glib::locale_to_utf8(s.substr(2)), 
					     "bold");
		else if (prefixIs(s, "@i"))
			buf->insert_with_tag(buf->end(), 
					     Glib::locale_to_utf8(s.substr(2)), 
					     "italic");
		else
			buf->insert(buf->end(), 
				    Glib::locale_to_utf8(s.substr(2)));
		buf->insert(buf->end(),"\n");

	}

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
