/**
 * \file GLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif

#include "GLog.h"
#include "ControlLog.h"

#include "ghelpers.h"

#include <sstream>

using std::string;

namespace lyx {
namespace frontend {

GLog::GLog(Dialog & parent)
	: GViewCB<ControlLog, GViewGladeB>(parent, _("Log Viewer"), false)
{}


void GLog::doBuild()
{
	string const gladeName = findGladeFile("log");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);

	xml_->get_widget("Refresh", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GLog::update));

	Gtk::TextView * contentview;
	xml_->get_widget("ContentView", contentview);
	contentbuffer_ = contentview->get_buffer();
}


void GLog::update()
{
	string const title = controller().title();

	if (!title.empty())
		setTitle(title);

	std::ostringstream contents;
	controller().getContents(contents);

	if (!contents.str().empty())
		contentbuffer_->set_text(contents.str());
	else
		contentbuffer_->set_text(_("Error reading file!"));
}

} // namespace frontend
} // namespace lyx
