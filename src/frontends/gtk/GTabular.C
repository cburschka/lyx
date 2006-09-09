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


#ifdef WITH_WARNINGS
#warning Implement booktabs settings
#endif

namespace lyx {
namespace frontend {

GTabular::GTabular(Dialog & parent)
	: GViewCB<ControlTabular, GViewGladeB>(parent, lyx::to_utf8(_("Table Settings")), false)
{}


void GTabular::doBuild()
{
	string const gladeName = findGladeFile("tabular");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button *button;
	xml_->get_widget("Close", button);
	setCancel(button);

	// **** Current cell coords ****
	xml_->get_widget("CurrentRow", cellrowspin_);
	xml_->get_widget("CurrentColumn", cellcolspin_);

	// **** Table tab ****
	Gtk::HBox *box;
	xml_->get_widget("HorizontalAlignment", box);
	horzaligncombo_ = Gtk::manage (new Gtk::ComboBoxText);
	box->add(*horzaligncombo_);
	box->show_all();
	xml_->get_widget("VerticalAlignment", vertaligncombo_);
	xml_->get_widget("SpecifyWidth", specifywidthcheck_);
	specifywidthcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSensitivity));

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
	multicolcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSensitivity));
	xml_->get_widget("NoBorders", button);
	button->signal_clicked().connect(
		sigc::bind(sigc::mem_fun(*this, &GTabular::allBorders), false));
	xml_->get_widget("AllBorders", button);
	button->signal_clicked().connect(
		sigc::bind(sigc::mem_fun(*this, &GTabular::allBorders), true));

	// **** Longtable tab ****
	xml_->get_widget("LongTable", longtablecheck_);

	longtablecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));

	xml_->get_widget("PageBreak", pagebreakcheck_);

	xml_->get_widget("SpecialRows", specialrowstable_);

	xml_->get_widget("HeaderOn",headeroncheck_);
	xml_->get_widget("FooterOn",footeroncheck_);
	xml_->get_widget("FirstHeaderOn",firstheaderoncheck_);
	xml_->get_widget("LastFooterOn",lastfooteroncheck_);
	xml_->get_widget("FirstHeaderEmpty",firstheaderemptycheck_);
	xml_->get_widget("LastFooterEmpty",lastfooteremptycheck_);
	xml_->get_widget("HeaderUpperBorderDouble",headerupperborderdoublecheck_);
	xml_->get_widget("FooterUpperBorderDouble",footerupperborderdoublecheck_);
	xml_->get_widget("FirstHeaderUpperBorderDouble",firstheaderupperborderdoublecheck_);
	xml_->get_widget("LastFooterUpperBorderDouble",lastfooterupperborderdoublecheck_);
	xml_->get_widget("HeaderLowerBorderDouble",headerlowerborderdoublecheck_);
	xml_->get_widget("FooterLowerBorderDouble",footerlowerborderdoublecheck_);
	xml_->get_widget("FirstHeaderLowerBorderDouble",firstheaderlowerborderdoublecheck_);
	xml_->get_widget("LastFooterLowerBorderDouble",lastfooterlowerborderdoublecheck_);

	headeroncheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	footeroncheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	firstheaderoncheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	lastfooteroncheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	firstheaderemptycheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	lastfooteremptycheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	headerupperborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	footerupperborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	firstheaderupperborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	lastfooterupperborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	headerlowerborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	footerlowerborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	firstheaderlowerborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));
	lastfooterlowerborderdoublecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GTabular::updateSpecialRowsSensitivity));

/*	headeroncheck_;
	firstheaderoncheck_;
	lastfooteroncheck_;
	firstheaderemptycheck_;
	lastfooteremptycheck_;
	headerupperborderdoublecheck_;
	footerupperborderdoublecheck_;
	firstheaderupperborderdoublecheck_;
	lastfooterupperborderdoublecheck_;
	headerlowerborderdoublecheck_;
	footerlowerborderdoublecheck_;
	firstheaderlowerborderdoublecheck_;
	lastfooterlowerborderdoublecheck_;*/

	// Disable for read-only documents.
//	bcview().addReadOnly(browsebutton_);

}


void GTabular::apply()
{

}


void GTabular::update()
{
	updating_ = true;

	LyXTabular const & tabular = controller().tabular();
	LyXTabular::idx_type const cell = controller().getActiveCell();
	bool const multicol = tabular.isMultiColumn(cell);

	cellcolspin_->get_adjustment()->set_value(tabular.column_of_cell (cell)+1);
	cellrowspin_->get_adjustment()->set_value(tabular.row_of_cell (cell));

	// **** Table tab ****

	LyXLength pwidth;
	string special;

	if (multicol) {
		special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
		pwidth = tabular.getMColumnPWidth(cell);
	} else {
		special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
		pwidth = tabular.getColumnPWidth(cell);
	}

	updateHorzAlignCombo(!multicol && !pwidth.zero());

	int halign = tabular.getAlignment(cell);
	if (halign == LYX_ALIGN_RIGHT)
		horzaligncombo_->set_active(2);
	else if (halign == LYX_ALIGN_CENTER)
		horzaligncombo_->set_active(1);
	else if (halign == LYX_ALIGN_BLOCK)
		horzaligncombo_->set_active(3);
	else
		horzaligncombo_->set_active(0);

	int valign = tabular.getVAlignment(cell);
	if (valign == LyXTabular::LYX_VALIGN_BOTTOM)
		vertaligncombo_->set_active(2);
	else if (valign == LyXTabular::LYX_VALIGN_MIDDLE)
		vertaligncombo_->set_active(1);
	else
		vertaligncombo_->set_active(0);

	latexargsentry_->set_text(special);
	specifywidthcheck_->set_active(!pwidth.zero());
	widthlength_->set_length(pwidth);
	rotatetablecheck_->set_active(tabular.getRotateTabular());

	// **** This Cell tab ****
	multicolcheck_->set_active(multicol);
	rotatecellcheck_->set_active(tabular.getRotateCell(cell));

	upperbordercheck_->set_active(tabular.topLine(cell));
	lowerbordercheck_->set_active(tabular.bottomLine(cell));
	leftbordercheck_->set_active(tabular.leftLine(cell));
	rightbordercheck_->set_active(tabular.rightLine(cell));

	// **** Long-table tab ****
	LyXTabular::row_type const row(tabular.row_of_cell(cell));
	bool const longtable = tabular.isLongTabular();
	longtablecheck_->set_active(longtable);
	pagebreakcheck_->set_active(tabular.getLTNewPage(row));

	if (longtable) {
		LyXTabular::ltType ltt;
		bool use_empty;
		bool row_set = tabular.getRowOfLTHead(row, ltt);

		headeroncheck_->set_active(row_set);
		lyxerr << "row_set for header = " << row_set << "\n";
		if (ltt.set) {
			headerupperborderdoublecheck_->set_active(ltt.topDL);
			headerlowerborderdoublecheck_->set_active(ltt.bottomDL);
			use_empty = true;
		} else {
			headerupperborderdoublecheck_->set_active(false);
			headerlowerborderdoublecheck_->set_active(false);
			/*headerupperborderdoublecheck_->set_sensitive(false);
			headerlowerborderdoublecheck_->set_sensitive(false);*/
			firstheaderemptycheck_->set_active(false);
			/*firstheaderemptycheck_->set_sensitive(false);*/
			use_empty = false;
		}

		row_set = tabular.getRowOfLTFirstHead(row, ltt);
		firstheaderoncheck_->set_active(row_set);
		if (ltt.set && (!ltt.empty || !use_empty)) {
			firstheaderupperborderdoublecheck_->set_active(ltt.topDL);
			firstheaderlowerborderdoublecheck_->set_active(ltt.bottomDL);
		} else {
			firstheaderupperborderdoublecheck_->set_active(false);
			firstheaderlowerborderdoublecheck_->set_active(false);
			/*firstheaderupperborderdoublecheck_->set_sensitive(false);
			firstheaderlowerborderdoublecheck_->set_sensitive(false);*/
			if (use_empty) {
				firstheaderemptycheck_->set_active(ltt.empty);
				//if (ltt.empty)
					//firstheaderoncheck_->set_sensitive(false);
			}
		}

		row_set = tabular.getRowOfLTFoot(row, ltt);
		footeroncheck_->set_active(row_set);
		if (ltt.set) {
			footerupperborderdoublecheck_->set_active(ltt.topDL);
			footerlowerborderdoublecheck_->set_active(ltt.bottomDL);
			use_empty = true;
		} else {
			footerupperborderdoublecheck_->set_active(false);
			footerlowerborderdoublecheck_->set_active(false);
			//footerupperborderdoublecheck_->set_sensitive(false);
			//footerlowerborderdoublecheck_->set_sensitive(false);
			lastfooteremptycheck_->set_active(false);
			//lastfooteremptycheck_->set_sensitive(false);
			use_empty = false;
		}

		row_set = tabular.getRowOfLTLastFoot(row, ltt);
		lastfooteroncheck_->set_active(row_set);
		if (ltt.set && (!ltt.empty || !use_empty)) {
			lastfooterupperborderdoublecheck_->set_active(ltt.topDL);
			lastfooterlowerborderdoublecheck_->set_active(ltt.bottomDL);
		} else {
			lastfooterupperborderdoublecheck_->set_active(false);
			lastfooterlowerborderdoublecheck_->set_active(false);
			//lastfooterupperborderdoublecheck_->set_sensitive(false);
			//lastfooterlowerborderdoublecheck_->set_sensitive(false);
			if (use_empty) {
				lastfooteremptycheck_->set_active(ltt.empty);
				//if (ltt.empty)
					//lastfooteroncheck_->set_sensitive(false);
			}
		}

	}

	updating_ = false;

	updateSensitivity();
	updateSpecialRowsSensitivity();
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
	if (updating_)
		return;

	bool const specifywidth = specifywidthcheck_->get_active();
	bool const multicol = multicolcheck_->get_active();

	widthlength_->set_sensitive(specifywidth);
	vertaligncombo_->set_sensitive(specifywidth);

	updateHorzAlignCombo(!multicol && specifywidth);
}


void GTabular::updateHorzAlignCombo(bool extraoption)
{
	if (updating_)
		return;

	//FIXME: check if we really need to completely rewrite combo data
	// Deprecated in favor of clear_items in gtkmm >= 2.8
	unsigned int const oldselection = horzaligncombo_->get_active_row_number();
	horzaligncombo_->clear();
	horzaligncombo_->append_text(lyx::to_utf8(_("Left")));
	horzaligncombo_->append_text(lyx::to_utf8(_("Center")));
	horzaligncombo_->append_text(lyx::to_utf8(_("Right")));
	if (extraoption)
		horzaligncombo_->append_text(lyx::to_utf8(_("Block")));
	if (oldselection < horzaligncombo_->get_model()->children().size())
		horzaligncombo_->set_active(oldselection);
	else
		horzaligncombo_->set_active(0);
}


void GTabular::updateSpecialRowsSensitivity()
{
	if (updating_)
		return;

	bool const header = headeroncheck_->get_active();
	bool const footer = footeroncheck_->get_active();
	bool const firstheader = firstheaderoncheck_->get_active();
	bool const firstheaderempty = firstheaderemptycheck_->get_active();
	bool const lastfooter = lastfooteroncheck_->get_active();
	bool const lastfooterempty = lastfooteremptycheck_->get_active();
	firstheaderemptycheck_->set_sensitive(!firstheader);
	lastfooteremptycheck_->set_sensitive(!lastfooter);
	headerupperborderdoublecheck_->set_sensitive(header);
	headerlowerborderdoublecheck_->set_sensitive(header);
	footerupperborderdoublecheck_->set_sensitive(footer);
	footerlowerborderdoublecheck_->set_sensitive(footer);

	firstheaderoncheck_->set_sensitive(!firstheaderempty);
	firstheaderupperborderdoublecheck_->set_sensitive(firstheaderempty || firstheader);
	firstheaderlowerborderdoublecheck_->set_sensitive(firstheaderempty || firstheader);
	lastfooteroncheck_->set_sensitive(!lastfooterempty);
	lastfooterupperborderdoublecheck_->set_sensitive(lastfooterempty || lastfooter);
	lastfooterlowerborderdoublecheck_->set_sensitive(lastfooterempty || lastfooter);
	bool const longtable = longtablecheck_->get_active();
	specialrowstable_->set_sensitive(longtable);
	pagebreakcheck_->set_sensitive(longtable);
}

} // namespace frontend
} // namespace lyx
