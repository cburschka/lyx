// -*- C++ -*-
/**
 * \file gnomeBC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GNOMEBC_H
#define GNOMEBC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonController.h"

namespace Gtk {
class Button;
class Widget;
};

class gnomeBC : public GuiBC<Gtk::Button, Gtk::Widget>
{
public:
	///
	gnomeBC(string const & cancel, string const & close);

private:
	/// Updates the button sensitivity (enabled/disabled)
	void setButtonEnabled(Gtk::Button * btn, bool enabled);

	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(Gtk::Widget * obj, bool enabled);

	/// Set the label on the button
	void setButtonLabel(Gtk::Button * btn, string const & label);
};

#endif // GNOMEBC_H
