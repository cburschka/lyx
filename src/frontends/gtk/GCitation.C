/**
 * \file GCitation.C
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

#include "GCitation.h"
#include "ControlCitation.h"
#include "ghelpers.h"

#include "bufferparams.h"

#include "support/lstrings.h"

#include <libglademm.h>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

class bibModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	bibModelColumns() { add(name); add(cite); add(bib_order); add(info); }

	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<bool> cite;
	Gtk::TreeModelColumn<int> bib_order;
	Gtk::TreeModelColumn<Glib::ustring> info;
};

bibModelColumns bibColumns;

class styleModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	styleModelColumns() { add(name); }

	Gtk::TreeModelColumn<Glib::ustring> name;
};

bool GCitation::bib_visible(const Gtk::TreeModel::const_iterator& iter)
{
	if (iter)
		return !((*iter)[bibColumns.cite]);
	return true;
}

styleModelColumns styleColumns;


GCitation::GCitation(Dialog & parent)
	: GViewCB<ControlCitation, GViewGladeB>(parent, _("Citation"), false)
{}


void GCitation::doBuild()
{
	string const gladeName = findGladeFile("citation");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	xml_->get_widget("Restore", restorebutton_);
	setRestore(restorebutton_);
	xml_->get_widget("Cancel", cancelbutton_);
	setCancel(cancelbutton_);
	xml_->get_widget("Apply", applybutton_);
	setApply(applybutton_);
	xml_->get_widget("OK", okbutton_);
	setOK(okbutton_);

	xml_->get_widget("Remove", removebutton_);
	xml_->get_widget("Add", addbutton_);
	xml_->get_widget("Back", backbutton_);
	xml_->get_widget("Forward", forwardbutton_);
	xml_->get_widget("Up", upbutton_);
	xml_->get_widget("Down", downbutton_);

	xml_->get_widget("CiteKeys", citekeysview_);
	xml_->get_widget("BibKeys", bibkeysview_);
	xml_->get_widget("Info", infoview_);
	xml_->get_widget("CaseSensitive", casecheck_);

	xml_->get_widget("SearchCite", citeradio_);
	xml_->get_widget("SearchBib", bibradio_);
	xml_->get_widget("SearchString", findentry_);
	xml_->get_widget("CaseSensitive", casecheck_);
	xml_->get_widget("RegularExpression", regexpcheck_);

	xml_->get_widget("StyleLabel", stylelabel_);
	xml_->get_widget("Style", stylecombo_);
	xml_->get_widget("TextBeforeLabel", beforelabel_);
	xml_->get_widget("TextBefore", beforeentry_);
	xml_->get_widget("TextAfter", afterentry_);
	xml_->get_widget("FullAuthorList", authorcheck_);
	xml_->get_widget("ForceUpperCase", uppercasecheck_);

	info_ = Gtk::TextBuffer::create();
	infoview_->set_buffer(info_);

	allListStore_ = Gtk::ListStore::create(bibColumns);

	Gtk::TreeModel::Path rootpath; //required for gtkmm < 2.6

	citekeysview_->append_column(_("CiteKeys"), bibColumns.name);
	citeFilter_ = Gtk::TreeModelFilter::create(allListStore_, rootpath);
	citeFilter_->set_visible_column(bibColumns.cite);
	citekeysview_->set_model(citeFilter_);
	citeselection_ = citekeysview_->get_selection();

	bibkeysview_->append_column(_("BibKeys"), bibColumns.name);
	bibSort_ = Gtk::TreeModelSort::create(allListStore_);
	bibSort_->set_sort_column(bibColumns.bib_order, Gtk::SORT_ASCENDING );
	bibFilter_ = Gtk::TreeModelFilter::create(bibSort_, rootpath);
	bibFilter_->set_visible_func(&GCitation::bib_visible);
	bibkeysview_->set_model(bibFilter_);
	bibselection_ = bibkeysview_->get_selection();

	styleStore_ = Gtk::ListStore::create(styleColumns);
	stylecombo_->set_model(styleStore_);

	upbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GCitation::up));
	downbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GCitation::down));
	addbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GCitation::add));
	removebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GCitation::remove));
	backbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GCitation::previous));
	forwardbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GCitation::next));

	bibradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GCitation::set_search_buttons));
	citeradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GCitation::set_search_buttons));
	findentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GCitation::set_search_buttons));

	bibselection_->signal_changed().connect(
		sigc::mem_fun(*this, &GCitation::bib_selected));
	citeselection_->signal_changed().connect(
		sigc::mem_fun(*this, &GCitation::cite_selected));

	beforeentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GCitation::enable_apply));
	afterentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GCitation::enable_apply));
	stylecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCitation::enable_apply));
	authorcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GCitation::enable_apply));
	uppercasecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GCitation::enable_apply));

}


void GCitation::enable_apply() {
	// if we passed !applylock_ directly as an argument, the restore button
	// would be activated
	if (!applylock_ && !(citeFilter_->children()).empty())
		bc().valid(true);
}


void GCitation::fill_styles()
{
	if ((citeFilter_->children()).empty()) {
		stylecombo_->set_sensitive(false);
		stylelabel_->set_sensitive(false);
		return;
	}

	int orig = stylecombo_->get_active_row_number();

	Gtk::TreeModel::iterator iter = citeselection_->get_selected();
	if(!iter)
		iter = (citeFilter_->children()).begin();
	string key = Glib::locale_from_utf8((*iter)[bibColumns.name]);

	std::vector<string> const & sty = controller().getCiteStrings(key);

	biblio::CiteEngine const engine = controller().getEngine();
	bool const basic_engine = engine == biblio::ENGINE_BASIC;

	stylecombo_->set_sensitive(!sty.empty() && !basic_engine);
	stylelabel_->set_sensitive(!sty.empty() && !basic_engine);

	vector<string>::const_iterator it = sty.begin();
	vector<string>::const_iterator const end = sty.end();

	styleStore_->clear();
	for (; it != end; ++it) {
	 	Gtk::TreeModel::iterator iter2 = styleStore_->append();
	 	(*iter2)[styleColumns.name] = Glib::locale_to_utf8(*it);
	}

	if(orig)
		stylecombo_->set_active(orig);
}


void GCitation::update_style()
{
	biblio::CiteEngine const engine = controller().getEngine();
	bool const natbib_engine =
		engine == biblio::ENGINE_NATBIB_AUTHORYEAR ||
		engine == biblio::ENGINE_NATBIB_NUMERICAL;
	bool const basic_engine = engine == biblio::ENGINE_BASIC;

	authorcheck_->set_sensitive(natbib_engine);
	uppercasecheck_->set_sensitive(natbib_engine);
	beforeentry_->set_sensitive(!basic_engine);
	beforelabel_->set_sensitive(!basic_engine);

	string const & command = controller().params().getCmdName();

	// Find the style of the citekeys
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();
	biblio::CitationStyle const cs(command);

	vector<biblio::CiteStyle>::const_iterator cit =
		std::find(styles.begin(), styles.end(), cs.style);

	//restore the latest natbib style
	if (style_ >= 0 && Gtk::TreeModel::Children::size_type(style_) <
		(styleStore_->children()).size())
			stylecombo_->set_active(style_);
	else
		stylecombo_->unset_active();

	authorcheck_->set_active(false);
	uppercasecheck_->set_active(false);

	if (cit != styles.end()) {
		stylecombo_->set_active(cit - styles.begin());
		authorcheck_->set_active(cs.full);
		uppercasecheck_->set_active(cs.forceUCase);
	}
}


void GCitation::update_contents()
{
	// Make a list of all available bibliography keys
	biblio::InfoMap const & theMap = controller().bibkeysInfo();
	std::vector<std::string> bibkeys = biblio::getKeys(theMap);
	std::vector<std::string> citekeys = support::getVectorFromString(
		controller().params().getContents());

	int bib_order = 0;
	allListStore_->clear();

	for (std::vector<std::string>::const_iterator cit = bibkeys.begin();
		cit != bibkeys.end(); ++cit) {

		Gtk::TreeModel::iterator iter = allListStore_->append();
		(*iter)[bibColumns.name] = Glib::locale_to_utf8(*cit);
		(*iter)[bibColumns.cite] = false; //reset state
		(*iter)[bibColumns.bib_order] = ++bib_order;
		(*iter)[bibColumns.info] = Glib::locale_to_utf8(
			biblio::getInfo(theMap,*cit));
	}

	// Now mark cite keys by setting their bibColumns.cite property to true
	// so they will be filtered and displayed in citekeysview_
	for (std::vector<std::string>::const_iterator ccit = citekeys.begin();
		ccit != citekeys.end(); ++ccit) {

		for (Gtk::TreeModel::const_iterator cbit =
			(allListStore_->children()).begin();
			cbit != (allListStore_->children()).end(); ++cbit) {

			if ((*cbit)[bibColumns.name] == (*ccit)) {
				(*cbit)[bibColumns.cite] = true;
				allListStore_->move(cbit,
					(allListStore_->children()).end());
				break;
			}

		}
	}
}


void GCitation::update()
{
	applylock_ = true;

	update_contents();

	info_->set_text(""); // Clear Info field

	// Initialise style tab widgets
	beforeentry_->set_text(Glib::locale_to_utf8(
		controller().params().getSecOptions()));
	afterentry_->set_text(Glib::locale_to_utf8(
		controller().params().getOptions()));

	fill_styles();
	update_style();

	// Deactivate all buttons
	upbutton_->set_sensitive(false);
	downbutton_->set_sensitive(false);
	removebutton_->set_sensitive(false);
	addbutton_->set_sensitive(false);

	set_search_buttons();
	applylock_ = false;
}


void GCitation::up()
{
	Gtk::TreeModel::iterator src =
		citeselection_->get_selected();
	Gtk::TreeModel::iterator dest = src;

	if(--dest == (citeFilter_->children()).begin())
		upbutton_->set_sensitive(false);

	src = citeFilter_->convert_iter_to_child_iter(src);
	dest = citeFilter_->convert_iter_to_child_iter(dest);
	allListStore_->iter_swap(src, dest);

	bc().valid(true);
	downbutton_->set_sensitive(true);
}


void GCitation::down()
{
	Gtk::TreeModel::iterator src =
		citeselection_->get_selected();
	Gtk::TreeModel::iterator dest = src;
	// Avoid slow operator-- by using an extra variable
	Gtk::TreeModel::iterator endtest = ++dest;

	if(++endtest == (citeFilter_->children()).end())
		downbutton_->set_sensitive(false);

	src = citeFilter_->convert_iter_to_child_iter(src);
	dest = citeFilter_->convert_iter_to_child_iter(dest);
	allListStore_->iter_swap(src, dest);

	bc().valid(true);
	upbutton_->set_sensitive(true);
}


void GCitation::add()
{
	Gtk::TreeModel::iterator iter = bibselection_->get_selected();

	if (iter) {
		Gtk::TreeModel::iterator next_iter = iter;

		// Select the right key in bibkeysview_ afterwards
		if(++next_iter == (bibFilter_->children()).end()) {
			if(iter != (bibFilter_->children()).begin()) {
				bibselection_->select(--iter);
				++iter;
			} else { // bibkeysview_ will be left empty...
				addbutton_->set_sensitive(false);
			}
		} else {
			bibselection_->select(next_iter);
		}

		iter = bibFilter_->convert_iter_to_child_iter(iter);
		iter = bibSort_->convert_iter_to_child_iter(iter);
		(*iter)[bibColumns.cite] = true;

		// Move key to the right position
		// If a cite key is selected, move bib key to the position above
		// Otherwise to the last position in citekeysview_
		Gtk::TreeModel::iterator cite_iter(citeselection_->get_selected());
		if (cite_iter) {
			cite_iter = citeFilter_->convert_iter_to_child_iter(cite_iter);
		} else {
			cite_iter = (allListStore_->children()).end();
		}
		allListStore_->move(iter,cite_iter);

		// Highlight and scroll to newly inserted key
		iter = citeFilter_->convert_child_iter_to_iter(iter);
		citeselection_->select(iter);
		citekeysview_->scroll_to_row(Gtk::TreePath(iter));

		// Set button states
		removebutton_->set_sensitive(true);
		set_search_buttons();

		bc().valid(true);
	}
}


void GCitation::remove()
{
	Gtk::TreeModel::iterator iter(citeselection_->get_selected());

	if (iter) {
		Gtk::TreeModel::iterator next_iter(iter);

		if(++next_iter == (citeFilter_->children()).end()) {
			if(iter != (citeFilter_->children()).begin()) {
				citeselection_->select(--iter);
				++iter;
				bc().valid(true);
			} else { // citekeysview_ will be left empty...
				removebutton_->set_sensitive(false);
				bc().valid(false);
			}
		} else {
			citeselection_->select(next_iter);
			bc().valid(true);
		}

		// Get an iterator to allListStore_
		iter = citeFilter_->convert_iter_to_child_iter(iter);
		(*iter)[bibColumns.cite] = false;

		// Highlight and scroll to newly inserted key
		iter = bibSort_->convert_child_iter_to_iter(iter);
		iter = bibFilter_->convert_child_iter_to_iter(iter);
		bibselection_->select(iter);
		bibkeysview_->scroll_to_row(Gtk::TreePath(iter));

		// Set button states
		addbutton_->set_sensitive(true);
		set_search_buttons();
	}
}

void GCitation::cite_selected()
{
	Gtk::TreeModel::iterator iter =
		citeselection_->get_selected();

	if (iter) {
		info_->set_text((*iter)[bibColumns.info]);
		removebutton_->set_sensitive(true);

		// Set sensitivity of Up/Down buttons
		if (iter == (citeFilter_->children()).begin()) {
			upbutton_->set_sensitive(false);
		} else {
			upbutton_->set_sensitive(true);
		}

		if (++iter == (citeFilter_->children()).end()) {
			downbutton_->set_sensitive(false);
		} else {
			downbutton_->set_sensitive(true);
		}

	} else {
		info_->set_text("");
		removebutton_->set_sensitive(false);

		// Set sensitivity of Up/Down buttons
		upbutton_->set_sensitive(false);
		downbutton_->set_sensitive(false);
	}

}


void GCitation::bib_selected()
{
	Gtk::TreeModel::iterator iter =
		bibselection_->get_selected();

	if (iter) {
		info_->set_text((*iter)[bibColumns.info]);
		addbutton_->set_sensitive(true);
	} else {
		info_->set_text("");
		addbutton_->set_sensitive(false);
	}
}

void GCitation::apply()
{
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	int const choice = stylecombo_->get_active_row_number();
	style_ = stylecombo_->get_active_row_number();

	bool const full  = authorcheck_->get_active();
	bool const force = uppercasecheck_->get_active();

	string const command =
		biblio::CitationStyle(styles[choice], full, force)
		.asLatexStr();

	Gtk::TreeNodeChildren children(citeFilter_->children());

	string citekeys;
	int i = 0;
	for (Gtk::TreeModel::const_iterator cit=children.begin();
		cit!=children.end(); ++cit) {

		string item(support::trim(Glib::locale_from_utf8((*cit)[bibColumns.name])));
		if (item.empty())
			continue;
		if (i++ > 0)
			citekeys += ",";
		citekeys += item;
	}

	controller().params().setCmdName(command);
	controller().params().setContents(citekeys);

	controller().params().setSecOptions(Glib::locale_from_utf8(beforeentry_->get_text()));
	controller().params().setOptions(Glib::locale_from_utf8(afterentry_->get_text()));

	update();
}

void GCitation::find(biblio::Direction dir)
{
	biblio::InfoMap const & theMap = controller().bibkeysInfo();
	std::vector<std::string> bibkeys;

	biblio::Search const type = regexpcheck_->get_active()
		? biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start;

	bool search_cite = citeradio_->get_active();
	bool const casesens = casecheck_->get_active();
	string const str = Glib::locale_from_utf8(findentry_->get_text());

	Gtk::TreeModel::iterator iter;
	Gtk::TreeModel::Children::difference_type sel = 0;

	if (search_cite) {
		for (iter = (citeFilter_->children()).begin();
			iter != (citeFilter_->children()).end(); ++iter) {

			bibkeys.push_back(Glib::locale_from_utf8(
				(*iter)[bibColumns.name]));
		}

		iter = citeselection_->get_selected();
		if (iter)
			sel = std::distance((citeFilter_->children()).begin(), iter);
	} else {
		for (iter = (bibFilter_->children()).begin();
			iter != (bibFilter_->children()).end(); ++iter) {

			bibkeys.push_back(Glib::locale_from_utf8(
				(*iter)[bibColumns.name]));
		}

		iter = bibselection_->get_selected();
		if (iter)
			sel = std::distance(
				(bibFilter_->children()).begin(), iter);
	}

	start = bibkeys.begin();

	if (sel >= 0 &&
		Gtk::TreeModel::Children::size_type(sel) < bibkeys.size())
			std::advance(start, sel);

	bool is_cite = !search_cite;
	while(is_cite != search_cite) {

		// Find the NEXT instance...
		if (dir == biblio::FORWARD)
			++start;

		vector<string>::const_iterator cit =
			biblio::searchKeys(theMap, bibkeys, str,
				   start, type, dir, casesens);

		if (cit == bibkeys.end()) {
			if (dir == biblio::FORWARD) {
				start = bibkeys.begin();
			}
			else {
				start = bibkeys.end();
				--start;
			}

			cit = biblio::searchKeys(theMap, bibkeys, str,
				 start, type, dir, casesens);

			if (cit == bibkeys.end()) {
				return;
			}
		}
		vector<string>::const_iterator bibstart = bibkeys.begin();
		vector<string>::difference_type const found =
			std::distance(bibstart, cit);
		if (found == sel)
			return;

		start = cit;
		if (search_cite)
			iter = (citeFilter_->children()).begin();
		else
			iter = (bibFilter_->children()).begin();
		std::advance(iter, found);
		is_cite = (*iter)[bibColumns.cite];
	}

	// Highlight and scroll to the key that was found
	if (search_cite) {
		citeselection_->select(iter);
		citekeysview_->set_cursor(Gtk::TreePath(iter));
		citekeysview_->scroll_to_row(Gtk::TreePath(iter));
		cite_selected();
	} else {
		bibselection_->select(iter);
		bibkeysview_->set_cursor(Gtk::TreePath(iter));
		bibkeysview_->scroll_to_row(Gtk::TreePath(iter));
		bib_selected();
	}
}

void GCitation::set_search_buttons()
{
	bool val = findentry_->get_text_length() && (
		(citeradio_->get_active() && !(citeFilter_->children()).empty())
		|| (bibradio_->get_active() && !(bibFilter_->children()).empty())
		);
	backbutton_->set_sensitive(val);
	forwardbutton_->set_sensitive(val);
}

void GCitation::previous()
{
	find(biblio::BACKWARD);
}


void GCitation::next()
{
	find(biblio::FORWARD);
}

} // namespace frontend
} // namespace lyx
