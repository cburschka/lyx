/**
 * \file GErrorList.C
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

#include "GErrorList.h"
#include "ControlErrorList.h"

#include "ghelpers.h"

using std::string;

namespace lyx {
namespace frontend {

GErrorList::GErrorList(Dialog & parent)
	: GViewCB<ControlErrorList, GViewGladeB>(parent, _("Errors"), false)
{}


void GErrorList::doBuild()
{
	string const gladeName = findGladeFile("errors");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * closebutton;
	xml_->get_widget("Close", closebutton);
	setCancel(closebutton);

	xml_->get_widget("ErrorList", errlistview_);
	listCols_.add(listCol_);
	listCols_.add(listColIndex_);
	errliststore_ = Gtk::ListStore::create(listCols_);
	errlistview_->set_model(errliststore_);
	errlistview_->append_column("Error", listCol_);
	errlistsel_ = errlistview_->get_selection();

	xml_->get_widget("ErrorDescription", errdescview_);

	errlistsel_->signal_changed().connect(
		sigc::mem_fun(*this, &GErrorList::onErrListSelection));
}


void GErrorList::update()
{
	setTitle(controller().name());
	updateContents();
}


void GErrorList::onErrListSelection()
{
	int const choice =
		(*errlistsel_->get_selected())[listColIndex_];

	ErrorList const & errors = controller().errorList();
	errdescview_->get_buffer()->set_text(errors[choice].description);
}


void GErrorList::updateContents()
{
	errliststore_->clear();
	ErrorList const & errors = controller().errorList();
	if (errors.empty()) {
		(*errliststore_->append())[listCol_] = _("*** No Errors ***");
		errlistview_->set_sensitive(false);
		return;
	}

	errlistview_->set_sensitive(true);

	ErrorList::const_iterator cit = errors.begin();
	ErrorList::const_iterator end = errors.end();
	for (int rowindex = 0; cit != end; ++cit, ++rowindex) {
		Gtk::ListStore::Row row = *errliststore_->append();
		if (rowindex == 0)
			errlistsel_->select(*row);

		(*row)[listCol_] = cit->error;
		(*row)[listColIndex_] = rowindex;
	}
}

} // namespace frontend
} // namespace lyx
