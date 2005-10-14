/**
 * \file GThesaurus.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
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

#include "GThesaurus.h"
#include "ControlThesaurus.h"
#include "ghelpers.h"

#include "support/lstrings.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

class synModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	synModelColumns() { add(name); }

	Gtk::TreeModelColumn<Glib::ustring> name;
};

synModelColumns synColumns;


GThesaurus::GThesaurus(Dialog & parent)
	: GViewCB<ControlThesaurus, GViewGladeB>(parent, _("Thesaurus"), false)
{}


void GThesaurus::doBuild()
{
	string const gladeName = findGladeFile("thesaurus");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	xml_->get_widget("Cancel", cancelbutton_);
	setCancel(cancelbutton_);
	xml_->get_widget("Apply", applybutton_);
	setApply(applybutton_);
	xml_->get_widget("OK", okbutton_);
	setOK(okbutton_);

	xml_->get_widget("Keyword", keywordentry_);
	xml_->get_widget("Meanings", meaningsview_);

	meaningsview_->append_column(_("Synonym"), synColumns.name);


	// Keyword entry changed
	keywordentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GThesaurus::update_lists));

	// Single click in meanings (synonyms) list
	meaningsview_->signal_cursor_changed().connect(
		sigc::mem_fun(*this, &GThesaurus::selection_changed));

	// Double click in meanings (synonyms) list
	meaningsview_->signal_row_activated().connect(
		sigc::mem_fun(*this, &GThesaurus::meaningsview_activated));

}

void GThesaurus::update()
{
	string const contents = support::trim(controller().text());
	if (contents.empty()) {
		applylock_ = true;
		bc().valid(false);
	} else {
		applylock_ = false;
		keywordentry_->set_text(Glib::locale_to_utf8(contents));
		bc().valid(true);
	}
	keywordentry_->grab_focus();
	update_lists();

}


void GThesaurus::selection_changed()
{
	Gtk::TreeModel::iterator iter = meaningsview_->get_selection()->get_selected();
	if(iter) {
		if (!applylock_) bc().valid(true);
	}

}


void GThesaurus::meaningsview_activated(const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*)
{
	Gtk::TreeModel::iterator iter = meaningsview_->get_selection()->get_selected();
	if(iter) {
		Gtk::TreeModel::Row row = *iter;
		keywordentry_->set_text(row[synColumns.name]);
	}
	selection_changed();
	update_lists();
	if (!applylock_) bc().valid(true);
}


void GThesaurus::apply()
{
	Gtk::TreeModel::iterator iter = meaningsview_->get_selection()->get_selected();
	if(iter) {
		controller().replace(Glib::locale_from_utf8((*iter)[synColumns.name]));
	} else if (keywordentry_->get_text_length()) {
		controller().replace(Glib::locale_from_utf8(keywordentry_->get_text()));
	}
	update();
}


void GThesaurus::update_lists()
{
	Thesaurus::Meanings meanings = controller().getMeanings(keywordentry_->get_text());
	synTreeStore_ = Gtk::TreeStore::create(synColumns);

	if (!meanings.empty()) {
		for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
			cit != meanings.end(); ++cit) {

			Gtk::TreeModel::Row row = *(synTreeStore_->append());
			row[synColumns.name] = cit->first;

			for (std::vector<string>::const_iterator cit2 = cit->second.begin(); 
				cit2 != cit->second.end(); ++cit2) {
  				Gtk::TreeModel::Row childrow = *(synTreeStore_->append(row.children()));
				childrow[synColumns.name] = *cit2;
				}
		}
		meaningsview_->set_sensitive(true);
	} else {
		Gtk::TreeModel::Row row = *(synTreeStore_->append());
		row[synColumns.name] = _("No synonyms found");
		meaningsview_->set_sensitive(false);
	}
	meaningsview_->set_model(synTreeStore_);

	if (keywordentry_->get_text_length()) {
		if (!applylock_) bc().valid(true);
	} else {
		bc().valid(false);
	}
}


} // namespace frontend
} // namespace lyx
