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

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlTabularCreate.h"
#include "GnomeBase.h"

namespace Gtk {
class SpinButton;
}

/**
 * This class implements the dialog to insert/modify urls.
 */
class FormTabularCreate : public FormCB<ControlTabularCreate> {
public:
	///
	FormTabularCreate(ControlTabularCreate & c);
	///
	~FormTabularCreate();

	void apply();
	
private:
	/// Build the dialog
	void build();

	/// Returns true if the dialog input is in a valid state.
	bool validate() const;
	void update();

	void OKClicked() { OKButton(); }
	void CancelClicked() { CancelButton(); }
	void ApplyClicked() { ApplyButton(); }

	/// The SpinButtons
	Gtk::SpinButton * rows_spin() const;
	Gtk::SpinButton * columns_spin() const;
	/// The ok button
	Gtk::Button * ok_btn() const;
	/// The cancel button
	Gtk::Button * cancel_btn() const;
	/// The apply button
	Gtk::Button * apply_btn() const;
};

#endif
