/**
 * \file paradlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "support/lstrings.h"

#include "dlg/helpers.h"
#include "dlg/lengthentry.h"

#include "paradlg.h"

#include "gettext.h"
#include "debug.h"

using kde_helpers::setSizeHint;
using std::endl;

ParaDialog::ParaDialog(FormParagraph * form, QWidget * parent, char const * name, bool, WFlags)
	: ParaDialogData(parent,name), form_(form)
{
	setCaption(name);

	setUpdatesEnabled(false);

	generalpage = new ParaGeneralDialog(this, "generalpage");
	tabstack->addTabPage(generalpage, _("&General"));
#ifndef NO_PEXTRA
	extrapage = new ParaExtraDialog(this, "extrapage");
	tabstack->addTabPage(extrapage, _("&Extra"));
#endif

	setUpdatesEnabled(true);
	update();
}


ParaDialog::~ParaDialog()
{
}


void ParaDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}


void ParaDialog::setReadOnly(bool readonly)
{
	/* 
	generalpage->combo_justification->setEnabled(!readonly);
	generalpage->check_lineabove->setEnabled(!readonly);
	generalpage->check_linebelow->setEnabled(!readonly);
	generalpage->abovepage->pagebreakabove->setEnabled(!readonly);
	generalpage->belowpage->pagebreakbelow->setEnabled(!readonly);
	generalpage->abovepage->keepabove->setEnabled(!readonly);
	generalpage->belowpage->keepbelow->setEnabled(!readonly);
	generalpage->noindent->setEnabled(!readonly);
	generalpage->abovepage->spaceabove->setEnabled(!readonly);
	generalpage->belowpage->spacebelow->setEnabled(!readonly);
	generalpage->abovepage->spaceabove->setEnabled(!readonly);
	generalpage->belowpage->spacebelow->setEnabled(!readonly);
	generalpage->abovepage->spaceaboveplus->setEnabled(!readonly);
	generalpage->belowpage->spacebelowplus->setEnabled(!readonly);
	generalpage->abovepage->spaceaboveminus->setEnabled(!readonly);
	generalpage->belowpage->spacebelowminus->setEnabled(!readonly);
	generalpage->labelwidth->setEnabled(!readonly);
	extrapage->specialalignment->setEnabled(!readonly);
	extrapage->type->setEnabled(!readonly);
	extrapage->widthvalue->setEnabled(!readonly);
	extrapage->widthvalueunits->setEnabled(!readonly);
	extrapage->hfillbetween->setEnabled(!readonly);
	extrapage->startnewminipage->setEnabled(!readonly);
	extrapage->top->setEnabled(!readonly);
	extrapage->middle->setEnabled(!readonly);
	extrapage->bottom->setEnabled(!readonly);
	ok->setEnabled(!readonly);
	apply->setEnabled(!readonly);
	restore->setEnabled(!readonly);
	cancel->setText(readonly ? _("&Close") : _("&Cancel"));
	*/ 
}


void ParaDialog::setLabelWidth(char const * text)
{
	// FIXME: should be cleverer here
	if (!compare(_("Senseless with this layout!"), text)) {
		generalpage->line_labelwidth->setText("");
		generalpage->line_labelwidth->setEnabled(false);
	} else {
		generalpage->line_labelwidth->setText(text);
		generalpage->line_labelwidth->setEnabled(true);
	}
}


void ParaDialog::setAlign(int type)
{
	switch (type) {
		case LYX_ALIGN_RIGHT: type = 3; break;
		case LYX_ALIGN_LEFT: type = 2; break;
		case LYX_ALIGN_CENTER: type = 1; break;
		default: type = 0; break;
	}

	generalpage->combo_justification->setCurrentItem(type);
}


void ParaDialog::setChecks(bool labove, bool lbelow, bool pabove, bool pbelow, bool noindent)
{
	generalpage->check_lineabove->setChecked(labove);
	generalpage->check_linebelow->setChecked(lbelow);
	generalpage->abovepage->check_pagebreakabove->setChecked(pabove);
	generalpage->belowpage->check_pagebreakbelow->setChecked(pbelow);
	generalpage->check_noindent->setChecked(noindent);
}


void ParaDialog::setSpace(VSpace::vspace_kind kindabove, VSpace::vspace_kind kindbelow, bool keepabove, bool keepbelow)
{
	int item=0;

	switch (kindabove) {
		case VSpace::NONE: item = 0; break;
		case VSpace::DEFSKIP: item = 1; break;
		case VSpace::SMALLSKIP: item = 2; break;
		case VSpace::MEDSKIP: item = 3; break;
		case VSpace::BIGSKIP: item = 4; break;
		case VSpace::VFILL: item = 5; break;
		case VSpace::LENGTH: item = 6; break;
	}
	generalpage->abovepage->combo_spaceabove->setCurrentItem(item);

	switch (kindbelow) {
		case VSpace::NONE: item = 0; break;
		case VSpace::DEFSKIP: item = 1; break;
		case VSpace::SMALLSKIP: item = 2; break;
		case VSpace::MEDSKIP: item = 3; break;
		case VSpace::BIGSKIP: item = 4; break;
		case VSpace::VFILL: item = 5; break;
		case VSpace::LENGTH: item = 6; break;
	}
	generalpage->belowpage->combo_spacebelow->setCurrentItem(item);
	
	generalpage->abovepage->length_spaceabove->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->length_spaceaboveplus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->length_spaceaboveminus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->belowpage->length_spacebelow->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->length_spacebelowplus->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->length_spacebelowminus->setEnabled(kindbelow == VSpace::LENGTH);

	generalpage->abovepage->check_keepabove->setChecked(keepabove);
	generalpage->belowpage->check_keepbelow->setChecked(keepbelow);
}


void ParaDialog::setAboveLength(float val, float plus, float minus,
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit == LyXLength::UNIT_NONE) {
		generalpage->abovepage->length_spaceabove->setValue("");
		generalpage->abovepage->length_spaceaboveplus->setValue("");
		generalpage->abovepage->length_spaceaboveminus->setValue("");
	} else {
		generalpage->abovepage->length_spaceabove->setValue(tostr(val));
		generalpage->abovepage->length_spaceaboveplus->setValue(tostr(plus));
		generalpage->abovepage->length_spaceaboveminus->setValue(tostr(minus));
	}

	generalpage->abovepage->length_spaceabove->setUnits(vunit);
	generalpage->abovepage->length_spaceaboveplus->setUnits(punit);
	generalpage->abovepage->length_spaceaboveminus->setUnits(munit);
}


void ParaDialog::setBelowLength(float val, float plus, float minus,
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit == LyXLength::UNIT_NONE) {
		generalpage->belowpage->length_spacebelow->setValue("");
		generalpage->belowpage->length_spacebelowplus->setValue("");
		generalpage->belowpage->length_spacebelowminus->setValue("");
	} else {
		generalpage->belowpage->length_spacebelow->setValue(tostr(val));
		generalpage->belowpage->length_spacebelowplus->setValue(tostr(plus));
		generalpage->belowpage->length_spacebelowminus->setValue(tostr(minus));
	}

	generalpage->belowpage->length_spacebelow->setUnits(vunit);
	generalpage->belowpage->length_spacebelowplus->setUnits(punit);
	generalpage->belowpage->length_spacebelowminus->setUnits(munit);
}


#ifndef NO_PEXTRA
void ParaDialog::setExtra(float widthval, LyXLength::UNIT units, const string percent, int align,
	bool hfill, bool startminipage, LyXParagraph::PEXTRA_TYPE type)
{
	if (type!=LyXParagraph::PEXTRA_NONE) {
		lyxerr[Debug::GUI] << "percent : $" << percent << "$ widthval " << widthval << " unit " << long(units) << endl;
		if (percent != "") {
			extrapage->line_widthvalue->setText(percent.c_str());
			extrapage->combo_widthvalueunits->setCurrentItem(11);
		} else {
			int unit = 0;
			extrapage->line_widthvalue->setText(tostr(widthval).c_str());
			switch (units) {
				case LyXLength::CM: unit = 0; break;
				case LyXLength::IN: unit = 1; break;
				case LyXLength::PT: unit = 2; break;
				case LyXLength::MM: unit = 3; break;
				case LyXLength::PC: unit = 4; break;
				case LyXLength::EX: unit = 5; break;
				case LyXLength::EM: unit = 6; break;
				case LyXLength::SP: unit = 7; break;
				case LyXLength::BP: unit = 8; break;
				case LyXLength::DD: unit = 9; break;
				case LyXLength::CC: unit = 10; break;
				case LyXLength::MU: unit = 0; break;
				case LyXLength::UNIT_NONE: unit = 0; break;
				default:
					lyxerr[Debug::GUI] << "Unknown unit " << long(units) << endl;
			}
			extrapage->combo_widthvalueunits->setCurrentItem(unit);
		}
	} else
		extrapage->line_widthvalue->setText("");

	switch (align) {
		case LyXParagraph::MINIPAGE_ALIGN_TOP:
			extrapage->radio_top->setChecked(true);
			break;
		case LyXParagraph::MINIPAGE_ALIGN_MIDDLE:
			extrapage->radio_middle->setChecked(true);
			break;
		case LyXParagraph::MINIPAGE_ALIGN_BOTTOM:
			extrapage->radio_bottom->setChecked(true);
			break;
	}
	
	extrapage->check_hfillbetween->setChecked(hfill);
	extrapage->check_startnewminipage->setChecked(startminipage);

	extrapage->specialalignment->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->radio_top->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->radio_middle->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->radio_bottom->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->line_widthvalue->setEnabled(type!=LyXParagraph::PEXTRA_NONE);
	extrapage->combo_widthvalueunits->setEnabled(type!=LyXParagraph::PEXTRA_NONE);
	extrapage->check_hfillbetween->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->check_startnewminipage->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);

	switch (type) {
		case LyXParagraph::PEXTRA_NONE:
			extrapage->combo_type->setCurrentItem(0);
			break;
		case LyXParagraph::PEXTRA_INDENT:
			extrapage->combo_type->setCurrentItem(1);
			break;
		case LyXParagraph::PEXTRA_MINIPAGE:
			extrapage->combo_type->setCurrentItem(2);
			break;
		case LyXParagraph::PEXTRA_FLOATFLT:
			extrapage->combo_type->setCurrentItem(3);
			break;
	}
}
#endif // NO_PEXTRA 


LyXGlueLength ParaDialog::getAboveLength() const
{
	LyXGlueLength len(
		generalpage->abovepage->length_spaceabove->getValue(),
		generalpage->abovepage->length_spaceabove->getUnits(),
		generalpage->abovepage->length_spaceaboveplus->getValue(),
		generalpage->abovepage->length_spaceaboveplus->getUnits(),
		generalpage->abovepage->length_spaceaboveminus->getValue(),
		generalpage->abovepage->length_spaceaboveminus->getUnits()
		);
	
	return len;
}


LyXGlueLength ParaDialog::getBelowLength() const
{
	LyXGlueLength len(
		generalpage->belowpage->length_spacebelow->getValue(),
		generalpage->belowpage->length_spacebelow->getUnits(),
		generalpage->belowpage->length_spacebelowplus->getValue(),
		generalpage->belowpage->length_spacebelowplus->getUnits(),
		generalpage->belowpage->length_spacebelowminus->getValue(),
		generalpage->belowpage->length_spacebelowminus->getUnits()
		);
	
	return len;
}


#ifndef NO_PEXTRA
LyXLength ParaDialog::getExtraWidth() const
{

	if (extrapage->combo_widthvalueunits->currentItem() != 11) {
		LyXLength::UNIT unit = LyXLength::CM;
		switch (extrapage->combo_widthvalueunits->currentItem()) {
			case 0: unit = LyXLength::CM; break;
			case 1: unit = LyXLength::IN; break;
			case 2: unit = LyXLength::PT; break;
			case 3: unit = LyXLength::MM; break;
			case 4: unit = LyXLength::PC; break;
			case 5: unit = LyXLength::EX; break;
			case 6: unit = LyXLength::EM; break;
			case 7: unit = LyXLength::SP; break;
			case 8: unit = LyXLength::BP; break;
			case 9: unit = LyXLength::DD; break;
			case 10: unit = LyXLength::CC; break;
			case 11: unit = LyXLength::CM; break;
			default:
				lyxerr[Debug::GUI] << "Unknown unit " << extrapage->combo_widthvalueunits->currentItem() << endl;
		}
		LyXLength len(strToDbl(extrapage->line_widthvalue->text()), unit);
		return len;
	} else {
		LyXLength len(0.0, LyXLength::UNIT_NONE);
		return len;
	}
}


string ParaDialog::getExtraWidthPercent() const
{
	double val = strToDbl(extrapage->line_widthvalue->text());
	if (val > 100.0)
		val = 100.0;
	if (val < 0.0)
		val = 0.0;
	return tostr(val);
}
#endif // NO_PEXTRA
