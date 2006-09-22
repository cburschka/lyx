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

#include "BufferView.h"
#include "debug.h"

#include "frontends/Application.h"
#include "frontends/Gui.h"
#include "frontends/LyXView.h"

#include <gtkmm.h>

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

docstring const GuiSelection::get() const
{
	Glib::RefPtr<Gtk::Clipboard> clipboard =
		Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
	string const str = clipboard->wait_for_text();
	lyxerr[Debug::ACTION] << "GuiSelection::get: " << str << endl;
	return lyx::from_utf8(str);
}


void GuiSelection::put(docstring const & str)
{
	string const utf8 = lyx::to_utf8(str);
	lyxerr[Debug::ACTION] << "GuiSelection::put: " << utf8 << endl;
	Glib::RefPtr<Gtk::Clipboard> clipboard =
		Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
	clipboard->set_text(utf8);
}


void GuiSelection::onClipboardGet(Gtk::SelectionData & /*selection_data*/,
		guint /*info*/)
{
	// FIXME: This assumes only one LyXView
	lyx::docstring const sel = theApp->gui().view(0).view()->requestSelection();
	if (!sel.empty())
		put(sel);
}


void GuiSelection::onClipboardClear()
{
	// FIXME: This assumes only one LyXView
	theApp->gui().view(0).view()->clearSelection();
}


void GuiSelection::haveSelection(bool toHave)
{
	if (toHave) {
		Glib::RefPtr<Gtk::Clipboard> clipboard =
			Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
		std::vector<Gtk::TargetEntry> listTargets;
		listTargets.push_back(Gtk::TargetEntry("UTF8_STRING"));
		clipboard->set(listTargets,
				sigc::mem_fun(const_cast<GuiSelection&>(*this),
					&GuiSelection::onClipboardGet),
				sigc::mem_fun(const_cast<GuiSelection&>(*this),
					&GuiSelection::onClipboardClear));
	}
}

} // namespace frontend
} // namespace lyx
