// -*- C++ -*-
/**
 * \file GBC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GBUTTONCONTROLLER_H
#define GBUTTONCONTROLLER_H

#include <gtkmm.h>

#include "ButtonController.h"
#include "BCView.h"
#include "LString.h"
#include "gettext.h"

class GBC : public GuiBC<Gtk::Button, Gtk::Widget>
{
public:
	GBC(ButtonController const & parent,
	    string const & cancel = _("Cancel"),
	    string const & close = _("Close"));
private:
	/// Updates the button sensitivity (enabled/disabled)
	void setButtonEnabled(Gtk::Button *, bool enabled) const;

	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(Gtk::Widget *, bool enabled) const;

	/// Set the label on the button
	void setButtonLabel(Gtk::Button *, string const & label) const;
};

#endif
