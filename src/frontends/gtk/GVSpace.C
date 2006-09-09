/**
 * \file GVSpace.C
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

#include "GVSpace.h"
#include "ControlVSpace.h"
#include "ghelpers.h"

#include <libglademm.h>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

GVSpace::GVSpace(Dialog & parent)
	: GViewCB<ControlVSpace, GViewGladeB>(parent, lyx::to_utf8(_("VSpace Settings")), false)
{}


void GVSpace::doBuild()
{
	string const gladeName = findGladeFile("vspace");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("OK", button);
	setOK(button);

	xml_->get_widget("Spacing", spacingcombo_);
	xml_->get_widget_derived("Size", sizelengthentry_);
	sizelengthentry_->set_relative(false);

	xml_->get_widget("Protect", protectcheck_);

	spacingcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GVSpace::onSpacingComboChanged));
}


void GVSpace::update()
{
	VSpace const space = controller().params();

	int pos = 0;
	switch (space.kind()) {
	case VSpace::DEFSKIP:
		pos = 0;
		break;
	case VSpace::SMALLSKIP:
		pos = 1;
		break;
	case VSpace::MEDSKIP:
		pos = 2;
		break;
	case VSpace::BIGSKIP:
		pos = 3;
		break;
	case VSpace::VFILL:
		pos = 4;
		break;
	case VSpace::LENGTH:
		pos = 5;
		break;
	}

	spacingcombo_->set_active(pos);

	protectcheck_->set_active(space.keep());

	bool const custom_vspace = space.kind() == VSpace::LENGTH;
	if (custom_vspace) {
		sizelengthentry_->set_length(space.length().len());
	} else {
		sizelengthentry_->set_length("");
	}
}


void GVSpace::apply()
{
	VSpace space;
	switch (spacingcombo_->get_active_row_number()) {
	case 0:
		space = VSpace(VSpace::DEFSKIP);
		break;
	case 1:
		space = VSpace(VSpace::SMALLSKIP);
		break;
	case 2:
		space = VSpace(VSpace::MEDSKIP);
		break;
	case 3:
		space = VSpace(VSpace::BIGSKIP);
		break;
	case 4:
		space = VSpace(VSpace::VFILL);
		break;
	case 5:
		space = VSpace(LyXGlueLength(sizelengthentry_->get_length()));
		break;
	}

	space.setKeep(protectcheck_->get_active());

	controller().params() = space;
}


void GVSpace::onSpacingComboChanged()
{
	bool const custom = spacingcombo_->get_active_row_number() == 5;
	sizelengthentry_->set_sensitive(custom);
}

} // namespace frontend
} // namespace lyx
