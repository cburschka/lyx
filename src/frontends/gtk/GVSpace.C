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

namespace {
string defaultUnit("cm");
} // namespace anon

GVSpace::GVSpace(Dialog & parent)
	: GViewCB<ControlVSpace, GViewGladeB>(parent, _("VSpace Settings"), false)
{}


void GVSpace::doBuild()
{
	string const gladeName = findGladeFile("vspace");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("Insert", button);
	setOK(button);

	xml_->get_widget("Spacing", spacingcombo_);
	xml_->get_widget("Value", valuespin_);
	xml_->get_widget("ValueUnits", valueunitscombo_);
	xml_->get_widget("Protect", protectcheck_);

	cols_.add(stringcol_);

	PopulateComboBox(valueunitscombo_, buildLengthNoRelUnitList());

	spacingcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GVSpace::onSpacingComboChanged));
}


void GVSpace::PopulateComboBox(Gtk::ComboBox * combo,
				  vector<string> const & strings)
{
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(cols_);
	vector<string>::const_iterator it = strings.begin();
	vector<string>::const_iterator end = strings.end();
	for (int rowindex = 0; it != end; ++it, ++rowindex) {
		Gtk::TreeModel::iterator row = model->append();
		(*row)[stringcol_] = *it;
	}

	combo->set_model(model);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	combo->pack_start(*cell, true);
	combo->add_attribute(*cell, "text", 0);
}


void GVSpace::update()
{
	// set the right default unit
	defaultUnit = getDefaultUnit();

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
		LyXLength length(space.length().asString());
		valuespin_->get_adjustment()->set_value(length.value());
		unitsComboFromLength(valueunitscombo_, stringcol_,
		                     length, defaultUnit);
	} else {
		valuespin_->get_adjustment()->set_value(0.0f);
		unitsComboFromLength(valueunitscombo_, stringcol_,
		                     LyXLength(defaultUnit), defaultUnit);
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
		Glib::ustring const valueunit =
			(*valueunitscombo_->get_active())[stringcol_];
		space = VSpace(LyXGlueLength(valuespin_->get_text() + valueunit));
		break;
	}

	space.setKeep(protectcheck_->get_active());

	controller().params() = space;
}


void GVSpace::onSpacingComboChanged()
{
	bool const custom = spacingcombo_->get_active_row_number() == 5;
	valueunitscombo_->set_sensitive(custom);
	valuespin_->set_sensitive(custom);
}

} // namespace frontend
} // namespace lyx
