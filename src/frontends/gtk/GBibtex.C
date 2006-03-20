/**
 * \file GBibtex.C
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

#include "GBibtex.h"
#include "ControlBibtex.h"
#include "ghelpers.h"

#include "support/filetools.h"
#include "support/lstrings.h"

#include <libglademm.h>

using lyx::support::ChangeExtension;
using lyx::support::split;
using lyx::support::trim;

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

GBibtex::GBibtex(Dialog & parent)
	: GViewCB<ControlBibtex, GViewGladeB>(parent, _("BibTeX Bibliography"), false)
{}


void GBibtex::doBuild()
{
	string const gladeName = findGladeFile("bibtex");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("OK", button);
	setOK(button);

	xml_->get_widget("Databases", databasesview_);
	databasessel_ = databasesview_->get_selection();
	// signal_changed gets connected at the end of this fn.

	Gtk::TreeModel::ColumnRecord listcols;
	listcols.add (stringcol_);
	databasesstore_ = Gtk::ListStore::create(listcols);
	databasesview_->set_model(databasesstore_);
	databasesview_->append_column("Database", stringcol_);

	xml_->get_widget("Add", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GBibtex::add));
	bcview().addReadOnly(button);
	xml_->get_widget("Remove", removebutton_);
	removebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GBibtex::remove));
	bcview().addReadOnly(removebutton_);
	xml_->get_widget("Browse", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GBibtex::browse));
	bcview().addReadOnly(button);

	Gtk::HBox *box;
	xml_->get_widget("Style", box);
	box->pack_start(stylecombo_);
	stylecombo_.show();
	bcview().addReadOnly(&stylecombo_);

	xml_->get_widget("Content", contentcombo_);
	bcview().addReadOnly(contentcombo_);
	xml_->get_widget("TOC", toccheck_);
	bcview().addReadOnly(toccheck_);

	databasessel_->signal_changed().connect(
		sigc::mem_fun(*this, &GBibtex::validate));
}


void GBibtex::update()
{
	string bibs(controller().params().getContents());
	string bib;

	databasesstore_->clear();
	while (!bibs.empty()) {
		bibs = split(bibs, bib, ',');
		bib = trim(bib);
		if (!bib.empty()) {
			Gtk::TreeModel::iterator const row = databasesstore_->append();
			(*row)[stringcol_] = bib;
		}
	}

	string bibstyle(controller().getStylefile());

	bool const bibtopic = controller().usingBibtopic();
	if (controller().bibtotoc() && !bibtopic)
		toccheck_->set_active(true);
	else
		toccheck_->set_active(false);

	toccheck_->set_sensitive(!bibtopic);

	string btprint(controller().params().getSecOptions());
	int btp = 0;
	if (btprint == "btPrintNotCited")
		btp = 1;
	else if (btprint == "btPrintAll")
		btp = 2;

	contentcombo_->set_active(btp);
	contentcombo_->set_sensitive(bibtopic);

	stylecombo_.clear();
	vector<string> str;
	controller().getBibStyles(str);

	int item_nr(-1);
	vector<string>::const_iterator it = str.begin();
	vector<string>::const_iterator const end = str.end();
	for (; it != end; ++it) {
		string item(ChangeExtension(*it, ""));
		if (item == bibstyle)
			item_nr = int(it - str.begin());
		stylecombo_.append_text (item);
	}

	if (item_nr == -1 && !bibstyle.empty()) {
		stylecombo_.append_text (bibstyle);
		item_nr = stylecombo_.get_model()->children().size() - 1;
	}

	if (item_nr != -1)
		stylecombo_.set_active(item_nr);
	else
		stylecombo_.get_entry()->set_text("");

	validate();
}


void GBibtex::apply()
{
	Gtk::TreeModel::iterator it = databasesstore_->children().begin();
	Gtk::TreeModel::iterator const end = databasesstore_->children().end();

	string dblist;
	for (; it != end; ++it) {
		Glib::ustring db = (*it)[stringcol_];
		dblist += string(db);
		if (it + 1 != end)
			dblist += ",";
	}

	controller().params().setContents(dblist);

	string const bibstyle = stylecombo_.get_active_text();
	bool const bibtotoc = toccheck_->get_active();
	bool const bibtopic = controller().usingBibtopic();

	if (!bibtopic && bibtotoc && (!bibstyle.empty())) {
		// both bibtotoc and style
		controller().params().setOptions("bibtotoc," + bibstyle);
	} else if (!bibtopic && bibtotoc) {
		// bibtotoc and no style
		controller().params().setOptions("bibtotoc");
	} else {
		// only style. An empty one is valid, because some
		// documentclasses have an own \bibliographystyle{}
		// command!
		controller().params().setOptions(bibstyle);
	}

	// bibtopic allows three kinds of sections:
	// 1. sections that include all cited references of the database(s)
	// 2. sections that include all uncited references of the database(s)
	// 3. sections that include all references of the database(s), cited or not
	int const btp = contentcombo_->get_active_row_number();

	switch (btp) {
	case 0:
		controller().params().setSecOptions("btPrintCited");
		break;
	case 1:
		controller().params().setSecOptions("btPrintNotCited");
		break;
	case 2:
		controller().params().setSecOptions("btPrintAll");
		break;
	}

	if (!bibtopic)
		controller().params().setSecOptions("");
}


void GBibtex::add()
{
	string new_bib = controller().browseBib("");
	if (new_bib.empty())
		return;

	new_bib = ChangeExtension(new_bib, string());

	Gtk::TreeModel::iterator const row = databasesstore_->append();
	(*row)[stringcol_] = new_bib;

	validate();
}


void GBibtex::remove()
{
	Gtk::TreeModel::iterator const selected = databasessel_->get_selected();
	if (!databasesstore_->iter_is_valid(selected))
		return;

	databasesstore_->erase(selected);

	validate();
}


void GBibtex::browse()
{
	string const file = controller().browseBst("");

	if (!file.empty()) {
		string const filen = ChangeExtension(file, "");
		bool present = false;

		for (int i = 0; i < stylecombo_.get_model()->children().size(); ++i) {
			stylecombo_.set_active(i);
			Glib::ustring const item = stylecombo_.get_active_text ();
			if (item == filen) {
				present = true;
				break;
			}
		}

		if (!present) {
			stylecombo_.append_text (filen);
			stylecombo_.set_active_text(filen);
		}
	}
}


void GBibtex::validate()
{
	if (databasesstore_->children().size())
		bc().valid(true);
	else
		bc().valid(false);

	Gtk::TreeModel::iterator selected = databasessel_->get_selected();
	bool const enableremove = databasesstore_->iter_is_valid(selected) && !readOnly();
	removebutton_->set_sensitive(enableremove);
}


} // namespace frontend
} // namespace lyx
