/**
 * \file gnomeBC.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gnomeBC.h"

#include <gtkmm/widget.h>
#include <gtkmm/button.h>

gnomeBC::gnomeBC(string const & cancel, string const & close)
	: GuiBC<Gtk::Button, Gtk::Widget>(cancel, close)
{}


void gnomeBC::setWidgetEnabled(Gtk::Widget * obj, bool enabled)
{
	if (obj)
		obj->set_sensitive(enabled);
}


void gnomeBC::setButtonEnabled(Gtk::Button * btn, bool enabled)
{
	if (btn)
		btn->set_sensitive(enabled);
}


void gnomeBC::setButtonLabel(Gtk::Button * obj, string const & label)
{}
