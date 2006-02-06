/**
 * \file GTabular.C
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

#include "GTabular.h"
#include "ControlTabular.h"

#include "ghelpers.h"

#include "controllers/helper_funcs.h"

#include "debug.h"

using std::string;
using std::vector;

/*
class GtkLengthEntry : public Gtk::HBox {
public:
	GtkLengthEntry::GtkLengthEntry(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

protected:
	Gtk::SpinButton spin_;
	Gtk::ComboBoxText combo_;

};

GtkLengthEntry::GtkLengthEntry(
	BaseObjectType* cobject,
	const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::HBox(cobject), spin_(0.1, 2)
{
	pack_start (spin_, true, true, 0);
	pack_start (combo_, true, true, 0);
	spin_.set_range(0.0, 99999.0f);
	set_spacing(6);
	show_all();
}*/



namespace lyx {

namespace frontend {

namespace {
string defaultUnit("cm");
} // namespace anon

GTabular::GTabular(Dialog & parent)
	: GViewCB<ControlTabular, GViewGladeB>(parent, _("Table Settings"), false)
{}


void GTabular::doBuild()
{
	string const gladeName = findGladeFile("tabular");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);


	// **** Current cell coords ****
	xml_->get_widget("CurrentRow", cellrowspin_);
	xml_->get_widget("CurrentColumn", cellcolspin_);

	// **** Table tab ****
	xml_->get_widget("HorizontalAlignment", horzaligncombo_);
	xml_->get_widget("VerticalAlignment", vertaligncombo_);
	xml_->get_widget("SpecifyWidth", specifywidthcheck_);
	xml_->get_widget_derived ("Width", widthlength_);
	xml_->get_widget("LaTeXArgument", latexargsentry_);
	xml_->get_widget("RotateTable", rotatetablecheck_);

	// **** This Cell tab ****
	xml_->get_widget("UpperBorder", upperbordercheck_);
	xml_->get_widget("LowerBorder", lowerbordercheck_);
	xml_->get_widget("LeftBorder", leftbordercheck_);
	xml_->get_widget("RightBorder", rightbordercheck_);
	xml_->get_widget("RotateCell", rotatecellcheck_);
	xml_->get_widget("MultiColumn", multicolcheck_);
	xml_->get_widget("NoBorders", button);
	button->signal_clicked().connect(
		sigc::bind(sigc::mem_fun(*this, &GTabular::allBorders), false));
	xml_->get_widget("AllBorders", button);
	button->signal_clicked().connect(
		sigc::bind(sigc::mem_fun(*this, &GTabular::allBorders), true));

	// **** Longtable tab ****

	// Disable for read-only documents.
//	bcview().addReadOnly(browsebutton_);

}


void GTabular::apply()
{

}


void GTabular::update()
{
	LyXTabular const & tabular = controller().tabular();
	LyXTabular::idx_type const cell = controller().getActiveCell();
	bool multicol = tabular.isMultiColumn(cell);

	cellcolspin_->get_adjustment()->set_value(tabular.column_of_cell (cell)+1);
	cellrowspin_->get_adjustment()->set_value(tabular.row_of_cell (cell));

	// **** Table tab ****
	int align = tabular.getAlignment(cell);
	if (align == LYX_ALIGN_RIGHT)
		horzaligncombo_->set_active(2);
	else if (align == LYX_ALIGN_CENTER)
		horzaligncombo_->set_active(1);
	else
		horzaligncombo_->set_active(0);

	align = tabular.getVAlignment(cell);
	if (align == LyXTabular::LYX_VALIGN_BOTTOM)
		vertaligncombo_->set_active(2);
	else if (align == LyXTabular::LYX_VALIGN_MIDDLE)
		vertaligncombo_->set_active(1);
	else
		vertaligncombo_->set_active(0);


        LyXLength pwidth;
        string special;

        if (multicol) {
                special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
                pwidth = tabular.getMColumnPWidth(cell);
        } else {
                special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
                pwidth = tabular.getColumnPWidth(cell);
        }

	latexargsentry_->set_text(special);
	specifywidthcheck_->set_active(!pwidth.zero());
	widthlength_->set_length(pwidth);
	rotatetablecheck_->set_active(tabular.getRotateTabular());

	//TODO: insert 'Block' entry into halign as neccessary


	// **** This Cell tab ****
	multicolcheck_->set_active(multicol);
	rotatecellcheck_->set_active(tabular.getRotateCell(cell));
}


void GTabular::onInput()
{
	updateSensitivity();
}


void GTabular::allBorders(bool borders)
{
	if (upperbordercheck_->is_sensitive())
		upperbordercheck_->set_active(borders);
	if (lowerbordercheck_->is_sensitive())
		lowerbordercheck_->set_active(borders);
	if (leftbordercheck_->is_sensitive())
		leftbordercheck_->set_active(borders);
	if (rightbordercheck_->is_sensitive())
		rightbordercheck_->set_active(borders);
}


void GTabular::updateSensitivity()
{
	bool specifywidth = specifywidthcheck_->get_active();
	widthlength_->set_sensitive(specifywidth);
	vertaligncombo_->set_sensitive(specifywidth);
}

} // namespace frontend
} // namespace lyx
