/**
 * \file GBC.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GBC.h"

using std::string;

namespace lyx {
namespace frontend {

GBC::GBC(ButtonController const & parent,
	 string const & cancel, string const & close)
	: GuiBC<Gtk::Button, Gtk::Widget>(parent, cancel, close)
{
}


void GBC::setButtonEnabled(Gtk::Button * btn, bool enabled) const
{
	btn->set_sensitive(enabled);
}


void GBC::setWidgetEnabled(Gtk::Widget * widget, bool enabled) const
{
	widget->set_sensitive(enabled);
}


void GBC::setButtonLabel(Gtk::Button * btn, string const & label) const
{
	// GTK+ Stock buttons take precedence
	if (!btn->get_use_stock())
		btn->set_label(Glib::locale_to_utf8(label));
}

} // namespace frontend
} // namespace lyx
