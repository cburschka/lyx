/**
 * \file GBox.C
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

#include "GBox.h"

#include "ghelpers.h"

#include "controllers/ControlBox.h"

#include "insets/insetbox.h"
#include "lengthcommon.h"
#include "lyxrc.h" // to set the default length values

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

namespace {
string defaultUnit("cm");
} // namespace anon

GBox::GBox(Dialog & parent)
	: GViewCB<ControlBox, GViewGladeB>(parent, _("Box Settings"), false)
{}


void GBox::doBuild()
{
	string const gladeName = findGladeFile("box");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * closebutton;
	xml_->get_widget("Close", closebutton);
	setCancel(closebutton);

	xml_->get_widget("Type", typecombo_);
	bcview().addReadOnly(typecombo_);
	xml_->get_widget("InnerBox", innerboxcombo_);
	bcview().addReadOnly(innerboxcombo_);
	xml_->get_widget("WidthUnits", widthunitscombo_);
	bcview().addReadOnly(widthunitscombo_);
	xml_->get_widget("HeightUnits", heightunitscombo_);
	bcview().addReadOnly(heightunitscombo_);
	xml_->get_widget("BoxVertical", boxvertcombo_);
	bcview().addReadOnly(boxvertcombo_);
	xml_->get_widget("ContentVertical", contentvertcombo_);
	bcview().addReadOnly(contentvertcombo_);
	xml_->get_widget("ContentHorizontal", contenthorzcombo_);
	bcview().addReadOnly(contenthorzcombo_);
	xml_->get_widget("Width", widthspin_);
	bcview().addReadOnly(widthspin_);
	xml_->get_widget("Height", heightspin_);
	bcview().addReadOnly(heightspin_);

	cols_.add(stringcol_);

	// fill the box type choice
	box_gui_tokens(ids_, gui_names_);
	PopulateComboBox(typecombo_, gui_names_);
	typecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onTypeComboChanged));

	// set up innerbox (populated in setInnerType)
	innerboxstore_ = Gtk::ListStore::create(cols_);
	innerboxcombo_->set_model(innerboxstore_);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	innerboxcombo_->pack_start(*cell, true);
	innerboxcombo_->add_attribute(*cell, "text", 0);

	innerboxcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onInnerBoxComboChanged));

	boxvertcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onAlignChanged));
	contenthorzcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onAlignChanged));
	contentvertcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onAlignChanged));

	heightunitscombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onHeightChanged));
	widthunitscombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GBox::onWidthChanged));

	heightspin_->signal_value_changed().connect(
		sigc::mem_fun(*this, &GBox::onHeightChanged));
	widthspin_->signal_value_changed().connect(
		sigc::mem_fun(*this, &GBox::onWidthChanged));


	widthunitsstore_ = Gtk::ListStore::create(cols_);
	widthunitscombo_->set_model(widthunitsstore_);
	cell = Gtk::manage(new Gtk::CellRendererText);
	widthunitscombo_->pack_start(*cell, true);
	widthunitscombo_->add_attribute(*cell, "text", 0);
	//widthunitscombo_ is populated in setSpecial

	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	vector<string> heightunits = buildLengthUnitList();
	// Append special entries, skipping the first item "None"
	heightunits.insert(heightunits.end(),
		++gui_names_spec_.begin(), gui_names_spec_.end());

	PopulateComboBox(heightunitscombo_, heightunits);
}


void GBox::PopulateComboBox(Gtk::ComboBox * combo,
				  vector<string> const & strings
				  )
{
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(cols_);
	vector<string>::const_iterator it = strings.begin();
	vector<string>::const_iterator end = strings.end();
	for(; it != end; ++it)
		(*model->append())[stringcol_] = *it;

	combo->set_model(model);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	combo->pack_start(*cell, true);
	combo->add_attribute(*cell, "text", 0);
}


void GBox::update()
{
	applylock_ = true;

	defaultUnit = getDefaultUnit();

	char c = controller().params().pos;
	boxvertcombo_->set_active(string("tcb").find(c, 0));
	c = controller().params().inner_pos;
	contentvertcombo_->set_active(string("tcbs").find(c, 0));
	c = controller().params().hor_pos;
	contenthorzcombo_->set_active(string("lcrs").find(c, 0));

	string type(controller().params().type);
	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			typecombo_->set_active(i);
	}

	applylock_ = false;
	updateInnerBoxCombo();
	applylock_ = true;

	bool ibox = controller().params().inner_box;
	boxvertcombo_->set_sensitive(ibox);
	contentvertcombo_->set_sensitive(ibox);
	contenthorzcombo_->set_sensitive(!ibox);
	setSpecial(ibox);

	widthspin_->get_adjustment()->set_value(controller().params().width.value());
	unitsComboFromLength(widthunitscombo_, stringcol_,
	                     controller().params().width, defaultUnit);

	string const special(controller().params().special);
	if (!special.empty() && special != "none") {
		string spc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); ++i) {
			if (special == ids_spec_[i])
				spc = gui_names_spec_[i];
		}
		for (unsigned int j = 0; j 
			< widthunitsstore_->children().size(); ++j) {
			if (widthunitsstore_->children()[j][stringcol_] == spc)
				widthunitscombo_->set_active(j);
		}
	}

	heightspin_->get_adjustment()->set_value(controller().params().height.value());
	unitsComboFromLength(heightunitscombo_, stringcol_,
	                     controller().params().height, defaultUnit);

	string const height_special(controller().params().height_special);
	if (!height_special.empty() && height_special != "none") {
		string hspc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); ++i) {
			if (height_special == ids_spec_[i]) {
				hspc = gui_names_spec_[i];
			}
		}
		for (unsigned int j = 0; j < heightunitscombo_->get_model()->children().size(); ++j) {
			if (heightunitscombo_->get_model()->children()[j][stringcol_] == hspc) {
				heightunitscombo_->set_active(j);
			}
		}
	}

	heightspin_->set_sensitive(ibox);
	heightunitscombo_->set_sensitive(ibox);
	applylock_ = false;
}


void GBox::setSpecial(bool ibox)
{
	bool const oldlock = applylock_;
	applylock_ = true;

	unsigned int const initselection = widthunitscombo_->get_active_row_number();
	widthunitsstore_->clear();
	vector<string> normalunits = buildLengthUnitList();
	if (ibox) {
		vector<string>::const_iterator it = normalunits.begin();
		vector<string>::const_iterator end = normalunits.end();
		for(; it != end; ++it)
			(*widthunitsstore_->append())[stringcol_] = *it;
	} else {
		vector<string>::const_iterator it = normalunits.begin();
		vector<string>::const_iterator end = normalunits.end();
		for(; it != end; ++it)
			(*widthunitsstore_->append())[stringcol_] = *it;
		// Skip the first item "None"
		it = ++gui_names_spec_.begin();
		end = gui_names_spec_.end();
		for(; it != end; ++it)
			(*widthunitsstore_->append())[stringcol_] = *it;
	}

	unsigned int const store_size = widthunitsstore_->children().size();
	if (initselection >= store_size) {
		widthunitscombo_->set_active(0);
		onWidthChanged();
	} else {
		widthunitscombo_->set_active(initselection);
	}
	applylock_ = oldlock;
}


void GBox::updateInnerBoxCombo()
{
	bool const oldlock = applylock_;
	applylock_ = true;
	// with "frameless" boxes, inner box is mandatory (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo

	// default: minipage
	int i = 2;
	if (!controller().params().inner_box)
		// none
		i = 0;
	if (controller().params().use_parbox)
		// parbox
		i = 1;
	bool frameless = (controller().params().type == "Frameless");

	int const oldsize = innerboxstore_->children().size();
	// Store the initial selection in 0,1,2 format
	int oldselection = -1;
	if (oldsize == 2)
		oldselection = innerboxcombo_->get_active_row_number() + 1;
	else if (oldsize == 3)
		oldselection = innerboxcombo_->get_active_row_number();

	if (frameless && oldsize != 2) {
		innerboxstore_->clear();
		(*innerboxstore_->append())[stringcol_] = _("Parbox");
		(*innerboxstore_->append())[stringcol_] = _("Minipage");
		// Cope when the backend asks for no inner box in
		// a frameless box
		if (i == 0) {
			applylock_ = false;
			innerboxcombo_->set_active(i);
			applylock_ = true;
		} else
			innerboxcombo_->set_active(i - 1);
	} else if (!frameless && oldsize != 3) {
		innerboxstore_->clear();
		(*innerboxstore_->append())[stringcol_] = _("None");
		(*innerboxstore_->append())[stringcol_] = _("Parbox");
		(*innerboxstore_->append())[stringcol_] = _("Minipage");
		innerboxcombo_->set_active(i);
	} else {
		// we're not changing the liststore, just selecting i
		if (frameless)
			innerboxcombo_->set_active(i - 1);
		else
			innerboxcombo_->set_active(i);
	}

	// Update the width units list if we've changed inner box type
	if (i != oldselection)
		setSpecial(i != 0);

	applylock_ = oldlock;
}


void GBox::onInnerBoxComboChanged()
{
	if (applylock_)
		return;

	controller().params().use_parbox =
		(*innerboxcombo_->get_active())[stringcol_] ==  _("Parbox");

	bool const ibox = (*innerboxcombo_->get_active())[stringcol_] != _("None");
	controller().params().inner_box = ibox;
	setSpecial(ibox);

	boxvertcombo_->set_sensitive(ibox);
	contentvertcombo_->set_sensitive(ibox);
	contenthorzcombo_->set_sensitive(!ibox);
	heightspin_->set_sensitive(ibox);
	heightunitscombo_->set_sensitive(ibox);
	// wtf? form_->set_sensitive(ibox);

	controller().dispatchParams();
}


void GBox::onTypeComboChanged()
{
	int const index = typecombo_->get_active_row_number();
	controller().params().type = ids_[index];

	bool frameless = (index == 0);
	if (frameless) {
		boxvertcombo_->set_sensitive(true);
		contentvertcombo_->set_sensitive(true);
		contenthorzcombo_->set_sensitive(false);
		heightspin_->set_sensitive(true);
		heightunitscombo_->set_sensitive(true);
		//wtf? form_->setSpecial(true);
	}
	//int itype = innerboxcombo_->get_active_row_number();
	controller().dispatchParams();

	updateInnerBoxCombo();
}


void GBox::onHeightChanged()
{
	if (applylock_)
		return;

	// "None"
	int i = 0;
	bool spec = false;
	Glib::ustring special = (*heightunitscombo_->get_active())[stringcol_];
	for (unsigned int j = 1; j < gui_names_spec_.size() ; ++j) {
		if (gui_names_spec_[j] == special) {
			i=j;
			spec = true;
		}
	}
	controller().params().height_special = ids_spec_[i];

	string height;
	if (spec) {
		height = heightspin_->get_text();
		// beware: bogosity! the unit is simply ignored in this case
		height += "in";
	} else {
		Glib::ustring const heightunit =
			(*heightunitscombo_->get_active())[stringcol_];
		height = heightspin_->get_text() + heightunit;
	}

	controller().params().height = LyXLength(height);
	controller().dispatchParams();
}


void GBox::onWidthChanged()
{
	if (applylock_)
		return;

	int i = 0;
	bool spec = false;
	Glib::ustring special = (*widthunitscombo_->get_active())[stringcol_];
	for (unsigned int j = 1; j < gui_names_spec_.size() ; ++j) {
		if (gui_names_spec_[j] == special) {
			i = j;
			spec = true;
		}
	}
	controller().params().special = ids_spec_[i];

	string width;
	if (spec) {
		width = widthspin_->get_text();
		// beware: bogosity! the unit is simply ignored in this case
		width += "in";
	} else {
		Glib::ustring const widthunit =
			(*widthunitscombo_->get_active())[stringcol_];
		width = widthspin_->get_text() + widthunit;
	}

	controller().params().width = LyXLength(width);
	controller().dispatchParams();
}


void GBox::onAlignChanged()
{
	if (applylock_)
		return;

	controller().params().pos =
		"tcb"[boxvertcombo_->get_active_row_number()];
	controller().params().inner_pos =
		"tcbs"[contenthorzcombo_->get_active_row_number()];
	controller().params().hor_pos =
		"lcrs"[contentvertcombo_->get_active_row_number()];

	controller().dispatchParams();
}

} // namespace frontend
} // namespace lyx
