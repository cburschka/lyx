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
 * \author Baruch Even
 * */

#ifndef FORMURL_H
#define FORMURL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlUrl.h"
#include "GnomeBase.h"

namespace Gtk {
class Button;
class CheckButton;
class Entry;
}

/**
 * This class implements the dialog to insert/modify urls.
 */
class FormUrl : public FormCB<ControlUrl> {
public:
	///
	FormUrl(ControlUrl & c);
	///
	~FormUrl();

	void apply();
	void update();
	
private:
	/// Build the dialog
	void build();

	/// Returns true if the dialog input is in a valid state.
	bool validate() const;

	/// Do the connection of signals
	void connect_signals();
	/// Disconnect the signals.
	void disconnect_signals();

	void OKClicked() { OKButton(); }
	void CancelClicked() { CancelButton(); }
	void ApplyClicked() { ApplyButton(); }
	void RestoreClicked() { RestoreButton(); }
	void InputChanged() { bc().valid(validate()); }
	
	/// The url entry
	Gtk::Entry * url() const;
	/// The name entry
	Gtk::Entry * name() const;
	/// The html type checkbutton
	Gtk::CheckButton * html() const;
	/// The ok button
	Gtk::Button * ok_btn() const;
	/// The cancel button
	Gtk::Button * cancel_btn() const;
	/// The apply button
	Gtk::Button * apply_btn() const;
	/// The restore button
	Gtk::Button * restore_btn() const;

	/// Keeps the connection to the input validator.
	SigC::Connection slot_url_;
	SigC::Connection slot_name_;
	SigC::Connection slot_html_;
};

#endif
