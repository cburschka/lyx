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

#ifdef CXX_WORKING_NAMESPACES
using kde_helpers::setSizeHint;
#endif

using std::endl;

ParaDialog::ParaDialog(FormParagraph * form, QWidget * parent, char const * name, bool, WFlags)
	: ParaDialogData(parent,name), form_(form)
{
	setCaption(name);

	setUpdatesEnabled(false);

	generalpage = new ParaGeneralDialog(this, "generalpage");
	extrapage = new ParaExtraDialog(this, "extrapage");
	
	tabstack->addTabPage(generalpage, _("&General"));
	tabstack->addTabPage(extrapage, _("&Extra"));

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
	generalpage->justification->setEnabled(!readonly);
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
}


void ParaDialog::setLabelWidth(char const * text)
{
	// FIXME: should be cleverer here
	if (!compare(_("Senseless with this layout!"), text)) {
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
		case LYX_ALIGN_RIGHT: type = 3; break;
		case LYX_ALIGN_LEFT: type = 2; break;
		case LYX_ALIGN_CENTER: type = 1; break;
		default: type = 0; break;
	}

	generalpage->justification->setCurrentItem(type);
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
	generalpage->abovepage->spaceabove->setCurrentItem(item);

	switch (kindbelow) {
		case VSpace::NONE: item = 0; break;
		case VSpace::DEFSKIP: item = 1; break;
		case VSpace::SMALLSKIP: item = 2; break;
		case VSpace::MEDSKIP: item = 3; break;
		case VSpace::BIGSKIP: item = 4; break;
		case VSpace::VFILL: item = 5; break;
		case VSpace::LENGTH: item = 6; break;
	}
	generalpage->belowpage->spacebelow->setCurrentItem(item);
	
	generalpage->abovepage->spaceabovevalue->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceaboveplus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->abovepage->spaceaboveminus->setEnabled(kindabove == VSpace::LENGTH);
	generalpage->belowpage->spacebelowvalue->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowplus->setEnabled(kindbelow == VSpace::LENGTH);
	generalpage->belowpage->spacebelowminus->setEnabled(kindbelow == VSpace::LENGTH);

	generalpage->abovepage->keepabove->setChecked(keepabove);
	generalpage->belowpage->keepbelow->setChecked(keepbelow);
}


void ParaDialog::setAboveLength(float val, float plus, float minus,
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit == LyXLength::UNIT_NONE) {
		generalpage->abovepage->spaceabovevalue->setValue("");
		generalpage->abovepage->spaceaboveplus->setValue("");
		generalpage->abovepage->spaceaboveminus->setValue("");
	} else {
		generalpage->abovepage->spaceabovevalue->setValue(tostr(val));
		generalpage->abovepage->spaceaboveplus->setValue(tostr(plus));
		generalpage->abovepage->spaceaboveminus->setValue(tostr(minus));
	}

	generalpage->abovepage->spaceabovevalue->setUnits(vunit);
	generalpage->abovepage->spaceaboveplus->setUnits(punit);
	generalpage->abovepage->spaceaboveminus->setUnits(munit);
}


void ParaDialog::setBelowLength(float val, float plus, float minus,
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit == LyXLength::UNIT_NONE) {
		generalpage->belowpage->spacebelowvalue->setValue("");
		generalpage->belowpage->spacebelowplus->setValue("");
		generalpage->belowpage->spacebelowminus->setValue("");
	} else {
		generalpage->belowpage->spacebelowvalue->setValue(tostr(val));
		generalpage->belowpage->spacebelowplus->setValue(tostr(plus));
		generalpage->belowpage->spacebelowminus->setValue(tostr(minus));
	}

	generalpage->belowpage->spacebelowvalue->setUnits(vunit);
	generalpage->belowpage->spacebelowplus->setUnits(punit);
	generalpage->belowpage->spacebelowminus->setUnits(munit);
}


void ParaDialog::setExtra(float widthval, LyXLength::UNIT units, const string percent, int align,
	bool hfill, bool startminipage, LyXParagraph::PEXTRA_TYPE type)
{
	if (type!=LyXParagraph::PEXTRA_NONE) {
		lyxerr[Debug::GUI] << "percent : $" << percent << "$ widthval " << widthval << " unit " << long(units) << endl;
		if (percent != "") {
			extrapage->widthvalue->setText(percent.c_str());
			extrapage->widthvalueunits->setCurrentItem(11);
		} else {
			int unit = 0;
			extrapage->widthvalue->setText(tostr(widthval).c_str());
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
			extrapage->widthvalueunits->setCurrentItem(unit);
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


LyXGlueLength ParaDialog::getAboveLength() const
{
	LyXGlueLength len(
		generalpage->abovepage->spaceabovevalue->getValue(),
		generalpage->abovepage->spaceabovevalue->getUnits(),
		generalpage->abovepage->spaceaboveplus->getValue(),
		generalpage->abovepage->spaceaboveplus->getUnits(),
		generalpage->abovepage->spaceaboveminus->getValue(),
		generalpage->abovepage->spaceaboveminus->getUnits()
		);
	
	return len;
}


LyXGlueLength ParaDialog::getBelowLength() const
{
	LyXGlueLength len(
		generalpage->belowpage->spacebelowvalue->getValue(),
		generalpage->belowpage->spacebelowvalue->getUnits(),
		generalpage->belowpage->spacebelowplus->getValue(),
		generalpage->belowpage->spacebelowplus->getUnits(),
		generalpage->belowpage->spacebelowminus->getValue(),
		generalpage->belowpage->spacebelowminus->getUnits()
		);
	
	return len;
}


LyXLength ParaDialog::getExtraWidth() const
{

	if (extrapage->widthvalueunits->currentItem() != 11) {
		LyXLength::UNIT unit = LyXLength::CM;
		switch (extrapage->widthvalueunits->currentItem()) {
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
				lyxerr[Debug::GUI] << "Unknown unit " << extrapage->widthvalueunits->currentItem() << endl;
		}
		LyXLength len(strToDbl(extrapage->widthvalue->text()), unit);
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
