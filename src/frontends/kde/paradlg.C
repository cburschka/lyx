/*
 * paradlg.C
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>
#include "support/lstrings.h" 

#include "paradlg.h"

#include "gettext.h"
#include "debug.h"

using std::endl;

ParaDialog::ParaDialog(FormParagraph *form, QWidget *parent, const char *name, bool, WFlags)
	: ParaDialogData(parent,name), form_(form)
{
	setCaption(name);

	generalpage = new ParaGeneralDialog(this, "generalpage");
	extrapage = new ParaExtraDialog(this, "extrapage");
	
	tabstack->addTabPage(generalpage, _("&General"));
	tabstack->addTabPage(extrapage, _("&Extra"));
}

ParaDialog::~ParaDialog()
{
}

void ParaDialog::closeEvent(QCloseEvent *e)
{
	form_->close();
	e->accept();
}

void ParaDialog::setReadOnly(bool readonly)
{
	generalpage->alignment->setEnabled(!readonly);
	generalpage->lineabove->setEnabled(!readonly);
	generalpage->linebelow->setEnabled(!readonly);
	generalpage->abovepage->pagebreakabove->setEnabled(!readonly);
	generalpage->belowpage->pagebreakbelow->setEnabled(!readonly);
	generalpage->abovepage->keepabove->setEnabled(!readonly);
	generalpage->belowpage->keepbelow->setEnabled(!readonly);
	generalpage->noindent->setEnabled(!readonly);
	generalpage->abovepage->spaceabove->setEnabled(!readonly);
	generalpage->belowpage->spacebelow->setEnabled(!readonly);
	generalpage->abovepage->spaceabovevalue->setEnabled(!readonly);
	generalpage->belowpage->spacebelowvalue->setEnabled(!readonly);
	generalpage->abovepage->spaceabovevalueunits->setEnabled(!readonly);
	generalpage->belowpage->spacebelowvalueunits->setEnabled(!readonly);
	generalpage->abovepage->spaceaboveplus->setEnabled(!readonly);
	generalpage->belowpage->spacebelowplus->setEnabled(!readonly);
	generalpage->abovepage->spaceaboveplusunits->setEnabled(!readonly);
	generalpage->belowpage->spacebelowplusunits->setEnabled(!readonly);
	generalpage->abovepage->spaceaboveminus->setEnabled(!readonly);
	generalpage->belowpage->spacebelowminus->setEnabled(!readonly);
	generalpage->abovepage->spaceaboveminusunits->setEnabled(!readonly);
	generalpage->belowpage->spacebelowminusunits->setEnabled(!readonly);
	generalpage->block->setEnabled(!readonly);
	generalpage->left->setEnabled(!readonly);
	generalpage->right->setEnabled(!readonly);
	generalpage->center->setEnabled(!readonly);
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
	cancel->setText(readonly ? _("Close") : _("Cancel"));
}

void ParaDialog::setLabelWidth(const char *text)
{
	// FIXME: should be cleverer here
	if (!strcmp(_("Senseless with this layout!"),text)) {
		generalpage->labelwidth->setText("");
		generalpage->labelwidth->setEnabled(false);
	} else {
		generalpage->labelwidth->setText(text);
		generalpage->labelwidth->setEnabled(true);
	}
}

void ParaDialog::setAlign(int type)
{
	switch (type) {
		case LYX_ALIGN_RIGHT:
			generalpage->right->setChecked(true);
			break;
		case LYX_ALIGN_LEFT:
			generalpage->left->setChecked(true);
			break;
		case LYX_ALIGN_CENTER:
			generalpage->center->setChecked(true);
			break;
		default: // ewww
			generalpage->block->setChecked(true);
			break;
	}
}

void ParaDialog::setChecks(bool labove, bool lbelow, bool pabove, bool pbelow, bool noindent)
{
	generalpage->lineabove->setChecked(labove);
	generalpage->linebelow->setChecked(lbelow);
	generalpage->abovepage->pagebreakabove->setChecked(pabove); 
	generalpage->belowpage->pagebreakbelow->setChecked(pbelow); 
	generalpage->noindent->setChecked(noindent);
}

void ParaDialog::setSpace(VSpace::vspace_kind kindabove, VSpace::vspace_kind kindbelow, bool keepabove, bool keepbelow)
{
	switch (kindabove) {
		case VSpace::NONE:
			generalpage->abovepage->spaceabove->setCurrentItem(0);
			break;
		case VSpace::DEFSKIP:
			generalpage->abovepage->spaceabove->setCurrentItem(1);
			break;
		case VSpace::SMALLSKIP:
			generalpage->abovepage->spaceabove->setCurrentItem(2);
			break;
		case VSpace::MEDSKIP:
			generalpage->abovepage->spaceabove->setCurrentItem(3);
			break;
		case VSpace::BIGSKIP:
			generalpage->abovepage->spaceabove->setCurrentItem(4);
			break;
		case VSpace::VFILL:
			generalpage->abovepage->spaceabove->setCurrentItem(5);
			break;
		case VSpace::LENGTH:
			generalpage->abovepage->spaceabove->setCurrentItem(6);
			break;
	}
	switch (kindbelow) {
		case VSpace::NONE:
			generalpage->belowpage->spacebelow->setCurrentItem(0);
			break;
		case VSpace::DEFSKIP:
			generalpage->belowpage->spacebelow->setCurrentItem(1);
			break;
		case VSpace::SMALLSKIP:
			generalpage->belowpage->spacebelow->setCurrentItem(2);
			break;
		case VSpace::MEDSKIP:
			generalpage->belowpage->spacebelow->setCurrentItem(3);
			break;
		case VSpace::BIGSKIP:
			generalpage->belowpage->spacebelow->setCurrentItem(4);
			break;
		case VSpace::VFILL:
			generalpage->belowpage->spacebelow->setCurrentItem(5);
			break;
		case VSpace::LENGTH:
			generalpage->belowpage->spacebelow->setCurrentItem(6);
			break;
	}

	generalpage->abovepage->spaceabovevalue->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceabovevalueunits->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceaboveplus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceaboveplusunits->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceaboveminus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceaboveminusunits->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->belowpage->spacebelowvalue->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowvalueunits->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowplus->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowplusunits->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowminus->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowminusunits->setEnabled(kindbelow == VSpace::LENGTH);

	generalpage->abovepage->keepabove->setChecked(keepabove);
	generalpage->belowpage->keepbelow->setChecked(keepbelow);
}

void ParaDialog::setUnits(QComboBox *box, LyXLength::UNIT unit)
{
	switch (unit) {
		case LyXLength::CM: box->setCurrentItem(0); break;
		case LyXLength::IN: box->setCurrentItem(1); break;
		case LyXLength::PT: box->setCurrentItem(2); break;
		case LyXLength::MM: box->setCurrentItem(3); break;
		case LyXLength::PC: box->setCurrentItem(4); break;
		case LyXLength::EX: box->setCurrentItem(5); break;
		case LyXLength::EM: box->setCurrentItem(6); break;
		case LyXLength::SP: box->setCurrentItem(7); break;
		case LyXLength::BP: box->setCurrentItem(8); break;
		case LyXLength::DD: box->setCurrentItem(9); break;
		case LyXLength::CC: box->setCurrentItem(10); break;
		case LyXLength::MU: box->setCurrentItem(0); break;
		case LyXLength::UNIT_NONE: box->setCurrentItem(0); break;
		default:
			lyxerr[Debug::GUI] << "Unknown unit " << long(unit) << endl;
	}
}

void ParaDialog::setAboveLength(float val, float plus, float minus, 
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit==LyXLength::UNIT_NONE) {
		generalpage->abovepage->spaceabovevalue->setText(""); 
		generalpage->abovepage->spaceaboveplus->setText(""); 
		generalpage->abovepage->spaceaboveminus->setText(""); 
		setUnits(generalpage->abovepage->spaceabovevalueunits, LyXLength::CM);
		setUnits(generalpage->abovepage->spaceaboveplusunits, LyXLength::CM);
		setUnits(generalpage->abovepage->spaceaboveminusunits, LyXLength::CM);
		return;
	}

	generalpage->abovepage->spaceabovevalue->setText(tostr(val).c_str());
	generalpage->abovepage->spaceaboveplus->setText(tostr(plus).c_str());
	generalpage->abovepage->spaceaboveminus->setText(tostr(minus).c_str());
	setUnits(generalpage->abovepage->spaceabovevalueunits, vunit);
	setUnits(generalpage->abovepage->spaceaboveplusunits, punit);
	setUnits(generalpage->abovepage->spaceaboveminusunits, munit);
}

void ParaDialog::setBelowLength(float val, float plus, float minus, 
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit==LyXLength::UNIT_NONE) {
		generalpage->belowpage->spacebelowvalue->setText(""); 
		generalpage->belowpage->spacebelowplus->setText(""); 
		generalpage->belowpage->spacebelowminus->setText(""); 
		setUnits(generalpage->belowpage->spacebelowvalueunits, LyXLength::CM);
		setUnits(generalpage->belowpage->spacebelowplusunits, LyXLength::CM);
		setUnits(generalpage->belowpage->spacebelowminusunits, LyXLength::CM);
		return;
	}

	generalpage->belowpage->spacebelowvalue->setText(tostr(val).c_str());
	generalpage->belowpage->spacebelowplus->setText(tostr(plus).c_str());
	generalpage->belowpage->spacebelowminus->setText(tostr(minus).c_str());
	setUnits(generalpage->belowpage->spacebelowvalueunits, vunit); 
	setUnits(generalpage->belowpage->spacebelowplusunits, punit); 
	setUnits(generalpage->belowpage->spacebelowminusunits, munit); 
}

void ParaDialog::setExtra(float widthval, LyXLength::UNIT units, const string percent, int align, 
	bool hfill, bool startminipage, LyXParagraph::PEXTRA_TYPE type)
{

	if (type!=LyXParagraph::PEXTRA_NONE) {
		lyxerr[Debug::GUI] << "percent : $" << percent << "$ widthval " << widthval << " unit " << long(units) << endl;
		if (percent != "") {
			extrapage->widthvalue->setText(percent.c_str());
			extrapage->widthvalueunits->setCurrentItem(12);
		} else {
			extrapage->widthvalue->setText(tostr(widthval).c_str());
			setUnits(extrapage->widthvalueunits,units);
		}
	} else
		extrapage->widthvalue->setText("");

	switch (align) {
		case LyXParagraph::MINIPAGE_ALIGN_TOP:
			extrapage->top->setChecked(true);
			break;
		case LyXParagraph::MINIPAGE_ALIGN_MIDDLE:
			extrapage->middle->setChecked(true);
			break;
		case LyXParagraph::MINIPAGE_ALIGN_BOTTOM:
			extrapage->bottom->setChecked(true);
			break;
	}
	
	extrapage->hfillbetween->setChecked(hfill);
	extrapage->startnewminipage->setChecked(startminipage);

	extrapage->specialalignment->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->top->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->middle->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->bottom->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->widthvalue->setEnabled(type!=LyXParagraph::PEXTRA_NONE);
	extrapage->widthvalueunits->setEnabled(type!=LyXParagraph::PEXTRA_NONE);
	extrapage->hfillbetween->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);
	extrapage->startnewminipage->setEnabled(type==LyXParagraph::PEXTRA_MINIPAGE);

	switch (type) {
		case LyXParagraph::PEXTRA_NONE:
			extrapage->type->setCurrentItem(0); 
			break;
		case LyXParagraph::PEXTRA_INDENT:
			extrapage->type->setCurrentItem(1); 
			break;
		case LyXParagraph::PEXTRA_MINIPAGE:
			extrapage->type->setCurrentItem(2); 
			break;
		case LyXParagraph::PEXTRA_FLOATFLT:
			extrapage->type->setCurrentItem(3); 
			break;
	}
}

LyXLength::UNIT ParaDialog::getUnits(QComboBox *box) const
{
	switch (box->currentItem()) {
		case 0: return LyXLength::CM;
		case 1: return LyXLength::IN;
		case 2: return LyXLength::PT;
		case 3: return LyXLength::MM;
		case 4: return LyXLength::PC;
		case 5: return LyXLength::EX;
		case 6: return LyXLength::EM;
		case 7: return LyXLength::SP;
		case 8: return LyXLength::BP;
		case 9: return LyXLength::DD;
		case 10: return LyXLength::CC;
		default:
			lyxerr[Debug::GUI] << "Unknown combo choice " << box->currentItem() << endl;
	}
	return LyXLength::UNIT_NONE;
}

LyXGlueLength ParaDialog::getAboveLength() const
{
	LyXGlueLength len(
		strToDbl(generalpage->abovepage->spaceabovevalue->text()),
		getUnits(generalpage->abovepage->spaceabovevalueunits),
		strToDbl(generalpage->abovepage->spaceaboveplus->text()),
		getUnits(generalpage->abovepage->spaceaboveplusunits),
		strToDbl(generalpage->abovepage->spaceaboveminus->text()),
		getUnits(generalpage->abovepage->spaceaboveminusunits)
		);
	
	return len;
}

LyXGlueLength ParaDialog::getBelowLength() const
{
	LyXGlueLength len(
	 	strToDbl(generalpage->belowpage->spacebelowvalue->text()),
		getUnits(generalpage->belowpage->spacebelowvalueunits),
		strToDbl(generalpage->belowpage->spacebelowplus->text()),
		getUnits(generalpage->belowpage->spacebelowplusunits),
		strToDbl(generalpage->belowpage->spacebelowminus->text()),
		getUnits(generalpage->belowpage->spacebelowminusunits)
		);
	
	return len;
}

LyXLength ParaDialog::getExtraWidth() const
{

	if (extrapage->widthvalueunits->currentItem()!=11) {
		LyXLength len(strToDbl(extrapage->widthvalue->text()), getUnits(extrapage->widthvalueunits));
		return len;
	} else {
		LyXLength len(0.0, LyXLength::UNIT_NONE);
		return len;
	}
}

string ParaDialog::getExtraWidthPercent() const
{
	double val = strToDbl(extrapage->widthvalue->text()); 
	if (val > 100.0) 
		val = 100.0;
	if (val < 0.0)
		val = 0.0;
	return tostr(val);
}
