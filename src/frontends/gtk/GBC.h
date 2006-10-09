// -*- C++ -*-
/**
 * \file GBC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GBUTTONCONTROLLER_H
#define GBUTTONCONTROLLER_H

#include "ButtonController.h"
#include "BCView.h"
#include "gettext.h"

#include <gtkmm.h>

namespace lyx {
namespace frontend {

class GBC : public GuiBC<Gtk::Button, Gtk::Widget> {
public:
	GBC(ButtonController const & parent,
	    lyx::docstring const & cancel = _("Cancel"),
	    lyx::docstring const & close = _("Close"));
private:
	/// Updates the button sensitivity (enabled/disabled)
	void setButtonEnabled(Gtk::Button *, bool enabled) const;

	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(Gtk::Widget *, bool enabled) const;

	/// Set the label on the button
	void setButtonLabel(Gtk::Button *, lyx::docstring const & label) const;
};

} // namespace frontend
} // namespace lyx

#endif
