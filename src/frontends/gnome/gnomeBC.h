// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2000 The LyX Team.
 *
 * ======================================================
 *
 * Author: Baruch Even  <baruch@lyx.org>
 */

#ifndef GNOMEBC_H
#define GNOMEBC_H

#include <list>

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonControllerBase.h"
#include "ButtonController.h"

namespace Gtk {
class Button;
class Widget;
}

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
	void setButtonLabel(Gtk::Button * btn, string const & label)
};

#endif // GNOMEBC_H
