#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gnomeBC.h"
#include "gtk--/widget.h"
#include "gtk--/button.h"

gnomeBC::gnomeBC(string const & cancel, string const & close)
	: ButtonControllerBase(cancel, close),
	  okay_(0), apply_(0), cancel_(0), undo_all_(0)
{}


void gnomeBC::setSensitive(Gtk::Button * btn, ButtonPolicy::Button id)
{
	if (btn) {
		bool const enabled = bp().buttonStatus(id);
		btn->set_sensitive(enabled);
	}
}


void gnomeBC::refresh()
{
	setSensitive(okay_, ButtonPolicy::OKAY);
	setSensitive(apply_, ButtonPolicy::APPLY);
	setSensitive(undo_all_, ButtonPolicy::UNDO_ALL);

#warning Handle the cancel button correctly! (be 20010327)
#if 0
	if (cancel_) {
		bool const enabled = bp().buttonStatus(ButtonPolicy::CANCEL);
//		if (enabled)
//			Change label to cancel_label_
//		else
//			Change label to close_label_
//Need to adapt it somehow since we use stock Gnome buttons.
	}
#endif
	
	if (!read_only_.empty()) {
		bool enable = true;
		if (bp().isReadOnly()) enable = false;
		
		WidgetList::const_iterator end = read_only_.end();
		for (WidgetList::const_iterator iter = read_only_.begin();
		     iter != end; ++iter) {
			(*iter)->set_sensitive(enable);
		}
	}
}
