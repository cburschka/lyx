// -*- C++ -*-
/* This file is part of
 * =================================================
 *
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 * =================================================
 *
 * \author Michael Koziarski <michael@koziarski.org>
 * */

#ifndef FORMCREDITS_H
#define FORMCREDITS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCredits.h"
#include "GnomeBase.h"

namespace Gtk {
class Button;
class Text;
}

/**
 * This class implements the dialog to show the credits.
 */
class FormCredits : public FormCB<ControlCredits> {
public:
	///
	FormCredits(ControlCredits & c);
	///
	~FormCredits() {};

	void apply() {};
	void update() {};

private:

	/// Build the dialog
	void build();

	void CancelClicked() { CancelButton(); }

	/// The ok button
	Gtk::Button * ok();

	Gtk::Text * text();
};

#endif
