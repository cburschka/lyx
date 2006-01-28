/**
 * \file GWrap.C
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

#include "GWrap.h"
#include "ControlWrap.h"
#include "insets/insetwrap.h"

#include "ghelpers.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GWrap::GWrap(Dialog & parent)
	: GViewCB<ControlWrap, GViewGladeB>(parent, _("Text Wrap Settings"), false)
{}


void GWrap::doBuild()
{
	string const gladeName = findGladeFile("wrap");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * cancelbutton;
	xml_->get_widget("Close", cancelbutton);
	setCancel(cancelbutton);

	xml_->get_widget_derived ("Width", widthlengthentry_);
	xml_->get_widget ("Placement", placementcombo_);

	widthlengthentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GWrap::apply));
	placementcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GWrap::apply));

	bcview().addReadOnly(widthlengthentry_);
	bcview().addReadOnly(placementcombo_);
}


void GWrap::update()
{
	applylock_ = true;

	InsetWrapParams & params = controller().params();

	widthlengthentry_->set_length (params.width);

	int item;
	if (params.placement == "l")
		item = 1;
	else if (params.placement == "r")
		item = 2;
	else if (params.placement == "p")
		item = 3;

	placementcombo_->set_active (item);

	bc().refreshReadOnly();

	applylock_ = false;
}


void GWrap::apply()
{
	if (applylock_)
		return;

        InsetWrapParams & params = controller().params();

        params.width = widthlengthentry_->get_length();

        int const placementrow = placementcombo_->get_active_row_number();
	BOOST_ASSERT (0 <= placementrow <= 3);
	switch (placementrow) {
		case 1:
			params.placement = 'l';
			break;
		case 2:
			params.placement = 'r';
			break;
		case 3:
			params.placement = 'p';
			break;
		case 0:
		default:
			params.placement.erase();
			break;
	}

	controller().dispatchParams();
}

} // namespace frontend
} // namespace lyx
