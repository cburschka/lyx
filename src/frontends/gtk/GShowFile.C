/**
 * \file GShowFile.C
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

#include "GShowFile.h"
#include "ControlShowFile.h"

#include "ghelpers.h"

using std::string;

namespace lyx {
namespace frontend {

GShowFile::GShowFile(Dialog & parent)
	: GViewCB<ControlShowFile, GViewGladeB>(parent, _("Show File"), false)
{}


void GShowFile::doBuild()
{
	string const gladeName = findGladeFile("showfile");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * closebutton;
	xml_->get_widget("Close", closebutton);
	setCancel(closebutton);

	Gtk::TextView * contentview;
	xml_->get_widget("ContentView", contentview);
	contentbuffer_ = contentview->get_buffer();
}


void GShowFile::update()
{
	string const title = controller().getFileName();

	if (!title.empty())
		setTitle(title);

	string const contents = controller().getFileContents();

	if (!contents.empty())
		contentbuffer_->set_text(contents);
	else
		contentbuffer_->set_text(_("Error reading file!"));
}

} // namespace frontend
} // namespace lyx
