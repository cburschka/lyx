/**
 * \file GSpellchecker.C
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

#include "GSpellchecker.h"
#include "controllers/ControlSpellchecker.h"

#include "ghelpers.h"

#include "support/convert.h"

using std::string;

namespace lyx {
namespace frontend {

GSpellchecker::GSpellchecker(Dialog & parent)
	: GViewCB<ControlSpellchecker, GViewGladeB>
		(parent, _("Spell-check document"), false)
{}


void GSpellchecker::doBuild()
{
	string const gladeName = findGladeFile("spellcheck");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);

	xml_->get_widget("Suggestions", suggestionsview_);
	xml_->get_widget("Unknown", unknownentry_);
	xml_->get_widget("Replacement", replacemententry_);
	xml_->get_widget("Progress", progress_);

	listCols_.add(listCol_);
	suggestionsstore_ = Gtk::ListStore::create(listCols_);
	suggestionsview_->set_model(suggestionsstore_);
	suggestionsview_->append_column("Suggestion", listCol_);
	suggestionssel_ = suggestionsview_->get_selection();

	// Single click in suggestion list
	suggestionssel_->signal_changed().connect(
		sigc::mem_fun(*this, &GSpellchecker::onSuggestionSelection));

	// Double click in suggestion list
	suggestionsview_->signal_row_activated().connect(
		sigc::mem_fun(*this, &GSpellchecker::onSuggestionActivate));

	// Because it's like a Replace button when double clicked
	bcview().addReadOnly(suggestionsview_);

	xml_->get_widget("Replace", button);
	bcview().addReadOnly(button);
	button->signal_clicked().connect(
		sigc::bind<bool>(
			sigc::mem_fun(*this, &GSpellchecker::onReplace), false));

	xml_->get_widget("ReplaceAll", button);
	bcview().addReadOnly(button);
	button->signal_clicked().connect(
		sigc::bind<bool>(
			sigc::mem_fun(*this, &GSpellchecker::onReplace), true));

	xml_->get_widget("Ignore", ignorebutton_);
	ignorebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GSpellchecker::onIgnore));

	xml_->get_widget("IgnoreAll", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GSpellchecker::onIgnoreAll));

	xml_->get_widget("Add", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GSpellchecker::onAdd));
}


void GSpellchecker::show()
{
	if (!window()) {
		build();
	}
	bcview().refreshReadOnly();
	controller().check();
	if (!controller().getWord().empty())
		window()->show();
}

void GSpellchecker::partialUpdate(int s)
{
	ControlSpellchecker::State const state =
		static_cast<ControlSpellchecker::State>(s);

	if (state == ControlSpellchecker::SPELL_FOUND_WORD) {
		string word = controller().getWord();
		Glib::ustring utfword = Glib::locale_to_utf8(word);
		unknownentry_->set_text(utfword);
		replacemententry_->set_text(utfword);

		// Get the list of suggestions
		suggestionsstore_->clear();
		while (!(word = controller().getSuggestion()).empty()) {
			utfword = Glib::locale_to_utf8(word);
			(*suggestionsstore_->append())[listCol_] = utfword;
		}

		if (readOnly())
			// In readonly docs the user must just be browsing through
			ignorebutton_->grab_focus();
		else
			// In general we expect the user to type their replacement
			replacemententry_->grab_focus();
	}

	int const progress = controller().getProgress();
	if (progress != 0) {
		progress_->set_fraction(float(progress)/100.0f);
		progress_->set_text(convert<string>(progress) + "% " + _("checked"));
	}
}


void GSpellchecker::onSuggestionActivate(
	Gtk::TreeModel::Path const & path,
	Gtk::TreeViewColumn * /*col*/)
{
	Glib::ustring const suggestion =
		(*suggestionsstore_->get_iter(path))[listCol_];

	if (!suggestion.empty())
		controller().replace(suggestion);
}


void GSpellchecker::onSuggestionSelection()
{
	Glib::ustring const suggestion =
		(*suggestionssel_->get_selected())[listCol_];

	if (!suggestion.empty())
		replacemententry_->set_text(suggestion);
}


void GSpellchecker::onIgnore()
{
	controller().check();
}


void GSpellchecker::onIgnoreAll()
{
	controller().ignoreAll();
}


void GSpellchecker::onAdd()
{
	controller().insert();
}


void GSpellchecker::onReplace(bool const all)
{
	Glib::ustring const replacement = replacemententry_->get_text();
	if (all)
		controller().replaceAll(replacement);
	else
		controller().replace(replacement);
}


} // namespace frontend
} // namespace lyx
