/**
 * \file GSendto.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GSendto.h"
#include "ControlSendto.h"
#include "ghelpers.h"

#include "format.h"

#include <libglademm.h>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

GSendto::GSendto(Dialog & parent)
	: GViewCB<ControlSendto, GViewGladeB>(parent, _("Send document to command"), false)
{}


void GSendto::doBuild()
{
	string const gladeName = findGladeFile("sendto");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);
	xml_->get_widget("Execute", button);
	setOK(button);

	xml_->get_widget("Format", formatview_);
	xml_->get_widget("Command", commandentry_);

	cols_.add(stringcol_);
	cols_.add(indexcol_);

	formatstore_ = Gtk::ListStore::create(cols_);
	formatview_->set_model(formatstore_);
	formatview_->append_column("Format", stringcol_);
	formatview_->get_selection()->set_mode(Gtk::SELECTION_BROWSE);

	commandentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GSendto::onCommandEntryChanged));
}


void GSendto::onCommandEntryChanged()
{
	bc().valid(!commandentry_->get_text().empty());
}


void GSendto::update()
{
	vector<Format const *> new_formats;
	new_formats = controller().allFormats();

	if (new_formats == all_formats_)
		return;

	all_formats_ = new_formats;

	vector<string> keys;
	keys.resize(all_formats_.size());

	vector<string>::iterator result = keys.begin();
	vector<Format const *>::const_iterator it  = all_formats_.begin();
	vector<Format const *>::const_iterator end = all_formats_.end();
	for (; it != end; ++it, ++result) {
		*result = (*it)->prettyname();
	}

	formatstore_->clear();

	vector<string>::const_iterator keyend = keys.end();
	vector<string>::const_iterator keyit = keys.begin();
	for (int rowindex = 0;
	     keyit < keyend; ++keyit, ++rowindex) {
		Gtk::TreeModel::iterator row = formatstore_->append();
		(*row)[stringcol_] = *keyit;
		(*row)[indexcol_] = rowindex;
	}

	commandentry_->set_text(controller().getCommand());
}


void GSendto::apply()
{
	int const line =
		(*formatview_->get_selection()->get_selected())[indexcol_];

	string const cmd = commandentry_->get_text();

	controller().setFormat(all_formats_[line]);
	controller().setCommand(cmd);
}

} // namespace frontend
} // namespace lyx
