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

#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCopyright.h"
#include "GnomeBase.h"

namespace Gtk {
class Button;
class Label;
}

/**
 * This class implements the dialog to show the copyright.
 */
class FormCopyright : public FormCB<ControlCopyright> {
public:
	///
	FormCopyright(ControlCopyright & c);
	///
	~FormCopyright() {};

	void apply() {}; 
	void update() {};
	
private:
	
	/// Build the dialog
	void build();
	Gtk::Button * ok();
	void CancelClicked() { CancelButton(); }
	Gtk::Label * disclaimer();
	Gtk::Label * copyright();
	Gtk::Label * license();
	/// The ok button
	
};

#endif
