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

/*
 * This is the top-level dialog which contains the buttons, and the tab bar for adding
 * the qtarch-designed child widget dialogs.
 *
 * FIXME: QTabDialog is not good for three reasons - 
 * 1) OK/Apply don't emit different signals. Why didn't the Qt people consider we might
 *    care about things other than visibility of the dialog *sigh*
 * 2) the default button placement goes against the style used in the other dialogs
 * 3) we don't seem to be able to disable the OK/Apply buttons
 *
 * So this must go
 *
 * In fact I'm not at all sure that this design is a good one from the user's point of view, but I
 * don't really have a better solution at the moment :/
 */

ParaDialog::ParaDialog(FormParagraph *form, QWidget *parent, const char *name, bool, WFlags)
	: QTabDialog(parent,name,false), form_(form)
{
	setCaption(name);

	generalpage = new ParaGeneralDialog(this, "generalpage");
	extrapage = new ParaExtraDialog(this, "extrapage");
	
	addTab(generalpage, _("&General"));
	addTab(extrapage, _("&Extra"));

	setOKButton(_("&OK"));
	setApplyButton(_("&Apply"));
	setDefaultButton(_("&Restore"));
	setCancelButton(_("&Cancel")); 

	connect(this, SIGNAL(applyButtonPressed()), SLOT(apply_adaptor()));
	connect(this, SIGNAL(defaultButtonPressed()), SLOT(restore()));
	connect(this, SIGNAL(cancelButtonPressed()), SLOT(close_adaptor()));
}

ParaDialog::~ParaDialog()
{
}

void ParaDialog::restore()
{
	// this will restore to the settings of the paragraph
	form_->update();
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
	generalpage->pagebreakabove->setEnabled(!readonly);
	generalpage->pagebreakbelow->setEnabled(!readonly);
	generalpage->noindent->setEnabled(!readonly);
	generalpage->spaceabove->setEnabled(!readonly);
	generalpage->spacebelow->setEnabled(!readonly);
	generalpage->spaceabovevalue->setEnabled(!readonly);
	generalpage->spacebelowvalue->setEnabled(!readonly);
	generalpage->spaceabovevalueunits->setEnabled(!readonly);
	generalpage->spacebelowvalueunits->setEnabled(!readonly);
	generalpage->spaceaboveplus->setEnabled(!readonly);
	generalpage->spacebelowplus->setEnabled(!readonly);
	generalpage->spaceaboveplusunits->setEnabled(!readonly);
	generalpage->spacebelowplusunits->setEnabled(!readonly);
	generalpage->spaceaboveminus->setEnabled(!readonly);
	generalpage->spacebelowminus->setEnabled(!readonly);
	generalpage->spaceaboveminusunits->setEnabled(!readonly);
	generalpage->spacebelowminusunits->setEnabled(!readonly);
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
	// FIXME: can't set buttons readonly
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
	generalpage->pagebreakabove->setChecked(pabove); 
	generalpage->pagebreakbelow->setChecked(pbelow); 
	generalpage->noindent->setChecked(noindent); 
}

void ParaDialog::setSpace(VSpace::vspace_kind kindabove, VSpace::vspace_kind kindbelow, bool keepabove, bool keepbelow)
{
	switch (kindabove) {
		case VSpace::NONE:
			generalpage->spaceabove->setCurrentItem(0);
			break;
		case VSpace::DEFSKIP:
			generalpage->spaceabove->setCurrentItem(1);
			break;
		case VSpace::SMALLSKIP:
			generalpage->spaceabove->setCurrentItem(2);
			break;
		case VSpace::MEDSKIP:
			generalpage->spaceabove->setCurrentItem(3);
			break;
		case VSpace::BIGSKIP:
			generalpage->spaceabove->setCurrentItem(4);
			break;
		case VSpace::VFILL:
			generalpage->spaceabove->setCurrentItem(5);
			break;
		case VSpace::LENGTH:
			generalpage->spaceabove->setCurrentItem(6);
			break;
	}
	switch (kindbelow) {
		case VSpace::NONE:
			generalpage->spacebelow->setCurrentItem(0);
			break;
		case VSpace::DEFSKIP:
			generalpage->spacebelow->setCurrentItem(1);
			break;
		case VSpace::SMALLSKIP:
			generalpage->spacebelow->setCurrentItem(2);
			break;
		case VSpace::MEDSKIP:
			generalpage->spacebelow->setCurrentItem(3);
			break;
		case VSpace::BIGSKIP:
			generalpage->spacebelow->setCurrentItem(4);
			break;
		case VSpace::VFILL:
			generalpage->spacebelow->setCurrentItem(5);
			break;
		case VSpace::LENGTH:
			generalpage->spacebelow->setCurrentItem(6);
			break;
	}

	generalpage->spaceabovevalue->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->spaceabovevalueunits->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->spaceaboveplus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->spaceaboveplusunits->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->spaceaboveminus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->spaceaboveminusunits->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->spacebelowvalue->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->spacebelowvalueunits->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->spacebelowplus->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->spacebelowplusunits->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->spacebelowminus->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->spacebelowminusunits->setEnabled(kindbelow == VSpace::LENGTH);

	// FIXME: I admit I don't know what keep does, or what is best to do with it ...
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
		case LyXLength::MU: box->setCurrentItem(11); break;
		case LyXLength::UNIT_NONE: box->setCurrentItem(0); break;
		default:
			lyxerr[Debug::GUI] << "Unknown unit " << long(unit) << endl;
	}
}

void ParaDialog::setAboveLength(float val, float plus, float minus, 
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit==LyXLength::UNIT_NONE) {
		generalpage->spaceabovevalue->setText(""); 
		generalpage->spaceaboveplus->setText(""); 
		generalpage->spaceaboveminus->setText(""); 
		setUnits(generalpage->spaceabovevalueunits, LyXLength::CM);
		setUnits(generalpage->spaceaboveplusunits, LyXLength::CM);
		setUnits(generalpage->spaceaboveminusunits, LyXLength::CM);
		return;
	}

	generalpage->spaceabovevalue->setText(tostr(val).c_str());
	generalpage->spaceaboveplus->setText(tostr(plus).c_str());
	generalpage->spaceaboveminus->setText(tostr(minus).c_str());
	setUnits(generalpage->spaceabovevalueunits, vunit);
	setUnits(generalpage->spaceaboveplusunits, punit);
	setUnits(generalpage->spaceaboveminusunits, munit);
}

void ParaDialog::setBelowLength(float val, float plus, float minus, 
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit==LyXLength::UNIT_NONE) {
		generalpage->spacebelowvalue->setText(""); 
		generalpage->spacebelowplus->setText(""); 
		generalpage->spacebelowminus->setText(""); 
		setUnits(generalpage->spacebelowvalueunits, LyXLength::CM);
		setUnits(generalpage->spacebelowplusunits, LyXLength::CM);
		setUnits(generalpage->spacebelowminusunits, LyXLength::CM);
		return;
	}

	generalpage->spacebelowvalue->setText(tostr(val).c_str());
	generalpage->spacebelowplus->setText(tostr(plus).c_str());
	generalpage->spacebelowminus->setText(tostr(minus).c_str());
	setUnits(generalpage->spacebelowvalueunits, vunit); 
	setUnits(generalpage->spacebelowplusunits, punit); 
	setUnits(generalpage->spacebelowminusunits, munit); 
}

void ParaDialog::setExtra(float widthval, LyXLength::UNIT units, const string percent, int align, 
	bool hfill, bool startminipage, LyXParagraph::PEXTRA_TYPE type)
{

	if (type!=LyXParagraph::PEXTRA_NONE) {
		lyxerr[Debug::GUI] << "percent : $" << percent << "$ widthval " << widthval << " unit " << long(units) << endl;
		if (percent!="") {
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
		case 11: return LyXLength::MU;
		default:
			lyxerr[Debug::GUI] << "Unknown combo choice " << box->currentItem() << endl;
	}
	return LyXLength::UNIT_NONE;
}

LyXGlueLength ParaDialog::getAboveLength() const
{
	LyXGlueLength len(
		strToDbl(generalpage->spaceabovevalue->text()),
		getUnits(generalpage->spaceabovevalueunits),
		strToDbl(generalpage->spaceaboveplus->text()),
		getUnits(generalpage->spaceaboveplusunits),
		strToDbl(generalpage->spaceaboveminus->text()),
		getUnits(generalpage->spaceaboveminusunits)
		);
	
	return len;
}

LyXGlueLength ParaDialog::getBelowLength() const
{
	LyXGlueLength len(
	 	strToDbl(generalpage->spacebelowvalue->text()),
		getUnits(generalpage->spacebelowvalueunits),
		strToDbl(generalpage->spacebelowplus->text()),
		getUnits(generalpage->spacebelowplusunits),
		strToDbl(generalpage->spacebelowminus->text()),
		getUnits(generalpage->spacebelowminusunits)
		);
	
	return len;
}

LyXLength ParaDialog::getExtraWidth() const
{

	if (extrapage->widthvalueunits->currentItem()!=12) {
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
