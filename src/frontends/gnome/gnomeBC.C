#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gnomeBC.h"
#include "ButtonController.tmpl"
#include "gtk--/widget.h"
#include "gtk--/button.h"

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
