/**
 * \file GToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GToc.h"
#include "ControlToc.h"

#include "ghelpers.h"

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

GToc::GToc(Dialog & parent)
	: GViewCB<ControlToc, GViewGladeB>(parent, _("Table of Contents"), false)
{}


void GToc::doBuild()
{
	string const gladeName = findGladeFile("toc");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);
	xml_->get_widget("Refresh", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GToc::updateContents));

	changing_views_ = true;

	// For both liststores
	listCols_.add(listCol_);
	listCols_.add(listColIndex_);

	// TOC ListView
	xml_->get_widget("Contents", tocview_);
	tocstore_ = Gtk::ListStore::create(listCols_);
	tocview_->set_model(tocstore_);
	tocview_->append_column("Entry", listCol_);

	listSel_ = tocview_->get_selection();
	listSel_->signal_changed().connect(
		sigc::mem_fun(*this, &GToc::onTocViewSelected));

	// Type Selection Combobox
	xml_->get_widget("Type", typecombo_);
	typestore_ = Gtk::ListStore::create(listCols_);
	typecombo_->set_model(typestore_);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	typecombo_->pack_start(*cell, true);
	typecombo_->add_attribute(*cell,"text",0);
	typecombo_->set_size_request(130,-1);

	typecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GToc::onTypeComboChanged));

	changing_views_ = false;
}


void GToc::update()
{
	updateType();
	updateContents();
}


void GToc::updateType()
{
	changing_views_ = true;
	string const targettype =
		toc::getType(controller().params().getCmdName());

	typestore_->clear();
	vector<string> types = controller().getTypes();

	// Because tiny empty ComboBoxes just look silly
	int const emptycombosize = 130;
	typecombo_->set_size_request(types.empty() ? emptycombosize : -1, -1);

	vector<string>::iterator it = types.begin();
	vector<string>::iterator end = types.end();
	for(;it != end; ++it) {
		Gtk::TreeModel::iterator row = typestore_->append();
		(*row)[listCol_] = *it;
		if (*it == targettype)
			typecombo_->set_active(row);
	}
	changing_views_ = false;
}


void GToc::updateContents()
{
	if (typestore_->children().empty()) {
		tocstore_->clear();
		(*tocstore_->append())[listCol_] = _("*** No Lists ***");
		tocview_->set_sensitive(false);
		return;
	}

	Gtk::TreeModel::iterator it = typecombo_->get_active();
	Glib::ustring const type = (*it)[listCol_];
	toc::Toc const contents = controller().getContents(type);

	// Check if all elements are the same.
	if (toc_ == contents) {
		return;
	}

	// List has changed, so let's update our view
	toc_ = contents;

	if (contents.empty()) {
		tocstore_->clear();
		(*tocstore_->append())[listCol_] = _("*** No Items ***");
		tocview_->set_sensitive(false);
		return;
	}

	// Okay, we're definitely going to put stuff in now
	changing_views_ = true;
	tocview_->set_sensitive(true);
	tocstore_->clear();

	toc::Toc::const_iterator cit = contents.begin();
	toc::Toc::const_iterator end = contents.end();

	for (int rowindex = 0; cit != end; ++cit, ++rowindex) {
		Gtk::ListStore::iterator row = tocstore_->append();
		(*row)[listCol_] = cit->asString();
		(*row)[listColIndex_] = rowindex;
	}
	changing_views_ = false;
}


void GToc::onTocViewSelected()
{
	// If we always do this, then an item is jumped to without
	// the user clicking on one when he changes type from TOC->figures or so
	if (!changing_views_) {
		unsigned int choice = (*listSel_->get_selected())[listColIndex_];
		if (choice < toc_.size()) {
			controller().goTo(toc_[choice]);
		}
	}
}


void GToc::onTypeComboChanged()
{
	updateContents();
}


} // namespace frontend
} // namespace lyx
