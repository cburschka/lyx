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

#include "debug.h"

#include "gnomeBC.h"
#include "FormCredits.h"
#include "gnome_helpers.h"

#include <gtk--/button.h>
#include <gtk--/text.h>
#include <gtk--/style.h>

FormCredits::FormCredits(ControlCredits & c)
	: FormCB<ControlCredits>(c, "diahelpcredits.glade", "DiaHelpCredits")
{
}


void FormCredits::build()
{
	// It is better to show an OK button, but the policy require that we
	// get a click on "Cancel"
	ok()->clicked.connect(SigC::slot(this, &FormCredits::CancelClicked));

	// Do not update the dialog when we insert the text
	text()->freeze();
	
	// Get the credits into the string stream
	stringstream ss;
	string credits = controller().getCredits(ss).str();

	// Create the strings that we need to detect.
	string const bold("@b");
	string const italic("@i");

	// Create the drawing contexts.
	Gtk::Text_Helpers::Context c_italic;
	Gtk::Text_Helpers::Context c_bold;

	{
		string bold = get_font_name(text()->get_style()->get_font());
		//lyxerr << "Font name: " << bold << std::endl;
		string italic(bold);

		string const medium("Medium-");
		std::string::size_type index = bold.find(medium);
		bold.replace(index, medium.size()-1, "bold");

		string const r("R-");
		index = italic.find(r);
		italic.replace(index, r.size()-1, "i");
		
		//lyxerr << "Bold: " << bold << "\nItalic: " << italic << std::endl;
		c_bold.set_font(Gdk_Font(bold));
		c_italic.set_font(Gdk_Font(italic));
	}

	// Insert it into the text and parse the attributes.
	while (!credits.empty()) {
		std::string::size_type end = credits.find('\n');
		string const line = credits.substr(0, ++end);
		credits = credits.substr(end);
		
//		lyxerr << "Line got: '" << line << "'\nend = " << end << std::endl;
		
		string const prefix = line.substr(0, 2);
		if (prefix == bold) {
			text()->insert(c_bold, line.substr(2));
		} else if (prefix == italic) {
			text()->insert(c_italic, line.substr(2));
		} else {
			text()->insert(line);
		}
	}
	
	// Allow the text area to be drawn.
	text()->thaw();
}


Gtk::Text * FormCredits::text()
{
	return getWidget<Gtk::Text>("credits_text");
}

Gtk::Button * FormCredits::ok()
{
	return getWidget<Gtk::Button>("credits_button_ok");
}
