/**
 * \file GTexinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
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

#include "GTexinfo.h"

#include "ghelpers.h"

#include "support/filetools.h"

using std::string;

namespace lyx {

using support::OnlyFilename;

namespace frontend {


GTexinfo::GTexinfo(Dialog & parent)
	: GViewCB<ControlTexinfo, GViewGladeB>(parent, _("TeX Information"), false),
	  activeStyle(ControlTexinfo::cls)
{}


void GTexinfo::doBuild() {
	string const gladeName = findGladeFile("texinfo");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);

	xml_->get_widget("Refresh", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GTexinfo::onRefresh));

	xml_->get_widget("FullPath", fullpathcheck_);
	fullpathcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTexinfo::updateStyles));

	// For both liststores
	listCols_.add(listCol_);
	listCols_.add(listColIndex_);

	// Items ListView
	xml_->get_widget("Items", itemsview_);
	itemsstore_ = Gtk::ListStore::create(listCols_);
	itemsview_->set_model(itemsstore_);
	itemsview_->append_column("Item", listCol_);
	listSel_ = itemsview_->get_selection();

	itemsview_->signal_row_activated().connect(
		sigc::mem_fun(*this, &GTexinfo::onItemActivate));

	// Type Selection Combobox
	xml_->get_widget("Type", typecombo_);
	typestore_ = Gtk::ListStore::create(listCols_);
	typecombo_->set_model(typestore_);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	typecombo_->pack_start(*cell, true);
	typecombo_->add_attribute(*cell, "text", 0);
	typecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GTexinfo::onTypeComboChanged));

	Gtk::TreeModel::iterator row = typestore_->append();
	(*row)[listCol_] = _("LaTeX classes");
	(*row)[listColIndex_] = ControlTexinfo::cls;
	// This is the default selection
	typecombo_->set_active(row);
	activeStyle = ControlTexinfo::cls;

	row = typestore_->append();
	(*row)[listCol_] = _("LaTeX styles");
	(*row)[listColIndex_] = ControlTexinfo::sty;

	row = typestore_->append();
	(*row)[listCol_] = _("BibTeX styles");
	(*row)[listColIndex_] = ControlTexinfo::bst;

	updateStyles();
}


void GTexinfo::onItemActivate(
	Gtk::TreeModel::Path const & path,
	Gtk::TreeViewColumn * /* col */)
{
	int const choice =
		(*itemsstore_->get_iter(path))[listColIndex_];

	ContentsType const & data = texdata_[activeStyle];

	string file = data[choice];
	if (!fullpathcheck_->get_active())
		file = getTexFileFromList(data[choice],
				controller().getFileType(activeStyle));

	if (choice >= 0 && choice <= data.size() - 1)
		controller().viewFile(file);
}


void GTexinfo::onTypeComboChanged()
{
	int const typeindex =
		(*typecombo_->get_active())[listColIndex_];
	activeStyle = static_cast<ControlTexinfo::texFileSuffix>(typeindex);
	updateStyles();
}


void GTexinfo::onRefresh()
{
	// makes sense only if the rights are set well for
	// users (/var/lib/texmf/ls-R)
	texhash();
	rescanTexStyles();
	updateStyles();
}


void GTexinfo::updateStyles()
{
	ContentsType & data = texdata_[activeStyle];
	bool const withFullPath = fullpathcheck_->get_active();
	getTexFileList(activeStyle, data, withFullPath);

	itemsstore_->clear();
	ContentsType::const_iterator it  = data.begin();
	ContentsType::const_iterator end = data.end();
	for (int rowindex = 0; it != end; ++it, ++rowindex) {
		Gtk::TreeModel::iterator row = itemsstore_->append();
		(*row)[listCol_] = *it;
		(*row)[listColIndex_] = rowindex;
	}
}

} // namespace frontend
} // namespace lyx
