// -*- C++ -*-
/**
 * \file gtk/GuiSelection.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 * \author Abdelrazak Younes
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

#include "GuiSelection.h"
#include "debug.h"

#include <gtkmm.h>

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

// ENCODING: Gtk::Clipboard returns UTF-8, we assume that the backend
// wants ISO-8859-1 and convert it to that.
// FIXME: Wrong!
string const GuiSelection::get() const
{
	Glib::RefPtr<Gtk::Clipboard> clipboard =
		Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
	string const str = Glib::convert_with_fallback(
			clipboard->wait_for_text(), "ISO-8859-1", "UTF-8");
	lyxerr[Debug::ACTION] << "GuiClipboard::get: " << str << endl;
	return str;
}


// ENCODING: we assume that the backend passes us ISO-8859-1 and
// convert from that to UTF-8 before passing to GTK
// FIXME: Wrong!
void GuiSelection::put(string const & str)
{
	lyxerr[Debug::ACTION] << "GuiClipboard::put: " << str << endl;
	Glib::RefPtr<Gtk::Clipboard> clipboard =
		Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
	clipboard->set_text(Glib::convert(str, "UTF-8", "ISO-8859-1"));
}

} // namespace frontend
} // namespace lyx
