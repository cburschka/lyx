/**
 * \file paragraphdlgimpl.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include "paragraphdlg.h"

#include <config.h>
#include <gettext.h>
#include <string>

#include "paragraphdlgimpl.h"
#include "FormParagraph.h"
#include "support/lstrings.h" 
#include "debug.h"

#include "qlineedit.h"
#include "qcombobox.h"
#include "qcheckbox.h"
#include "qpushbutton.h"
#include "qtabwidget.h"
#include "qlabel.h"
#include "qgroupbox.h"

using std::endl;

ParagraphDlgImpl::ParagraphDlgImpl(FormParagraph *form, QWidget* parent,  const char* name, bool modal, WFlags fl )
	: ParagraphDlg( parent, name, modal, fl ), form_(form)
{
	setCaption(name);
}


ParagraphDlgImpl::~ParagraphDlgImpl()
{
}


void ParagraphDlgImpl::setReadOnly(bool readonly)
{
	TabsParagraph->setEnabled(!readonly);
	okPB->setEnabled(!readonly);
	applyPB->setEnabled(!readonly);
	defaultsPB->setEnabled(!readonly);
	cancelPB->setText(readonly ? _("&Close") : _("&Cancel"));
}


void ParagraphDlgImpl::setLabelWidth(const char *text)
{
	// FIXME: should be cleverer here
	if (!strcmp(_("Senseless with this layout!"),text)) {
		listOptions->setEnabled(false);
		labelWidth->setText("");
	} else {
		listOptions->setEnabled(true);
		labelWidth->setText(text);
	}
}


void ParagraphDlgImpl::setAlign(int type)
{
	int item=0;
	switch (type) {
		case LYX_ALIGN_BLOCK : item=0; break;
		case LYX_ALIGN_LEFT : item=1; break;
		case LYX_ALIGN_RIGHT : item=2; break;
		case LYX_ALIGN_CENTER : item=3; break;
		default:
			item=0;
	}
	alignment->setCurrentItem(item);
}


void ParagraphDlgImpl::setChecks(bool labove, bool lbelow, bool pabove, bool pbelow, bool noindent)
{
	lineAbove->setChecked(labove);
	lineBelow->setChecked(lbelow);
	pagebreakAbove->setChecked(pabove);
	pagebreakBelow->setChecked(pbelow);
	noIndent->setChecked(noindent);
}


void ParagraphDlgImpl::setSpace(VSpace::vspace_kind kindabove, VSpace::vspace_kind kindbelow, bool keepabove, bool keepbelow)
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
	spacingAbove->setCurrentItem(item);
	
	switch (kindbelow) {
		case VSpace::NONE: item = 0; break;
		case VSpace::DEFSKIP: item = 1; break;
		case VSpace::SMALLSKIP: item = 2; break;
		case VSpace::MEDSKIP: item = 3; break;
		case VSpace::BIGSKIP: item = 4; break;
		case VSpace::VFILL: item = 5; break;
		case VSpace::LENGTH: item = 6; break;
	}
	spacingBelow->setCurrentItem(item);
	
	spacingAboveValue->setEnabled(kindabove == VSpace::LENGTH);
	spacingAbovePlus->setEnabled(kindabove == VSpace::LENGTH);
	spacingAboveMinus->setEnabled(kindabove == VSpace::LENGTH);
	spacingBelowValue->setEnabled(kindbelow == VSpace::LENGTH);
	spacingBelowPlus->setEnabled(kindbelow == VSpace::LENGTH);
	spacingBelowMinus->setEnabled(kindbelow == VSpace::LENGTH);
	
	spacingAboveKeep->setChecked(keepabove);
	spacingBelowKeep->setChecked(keepbelow);
}


void ParagraphDlgImpl::setAboveLength(float val, float plus, float minus, 
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit==LyXLength::UNIT_NONE) {
		spacingAboveValue->setText("");
		spacingAbovePlus->setText("");
		spacingAboveMinus->setText("");
	} else {
		spacingAboveValue->setText(tostr(val).c_str());
		spacingAbovePlus->setText(tostr(plus).c_str());
		spacingAboveMinus->setText(tostr(minus).c_str());
	}
	
	spacingAboveValueUnit->setCurrentItem(getItem(vunit));
	spacingAbovePlusUnit->setCurrentItem(getItem(punit));
	spacingAboveMinusUnit->setCurrentItem(getItem(munit));

}


void ParagraphDlgImpl::setBelowLength(float val, float plus, float minus, 
	LyXLength::UNIT vunit, LyXLength::UNIT punit, LyXLength::UNIT munit)
{
	if (vunit==LyXLength::UNIT_NONE) {
		spacingBelowValue->setText("");
		spacingBelowPlus->setText("");
		spacingBelowMinus->setText("");
	} else {
		spacingBelowValue->setText(tostr(val).c_str());
		spacingBelowPlus->setText(tostr(plus).c_str());
		spacingBelowMinus->setText(tostr(minus).c_str());
	}
	
	spacingBelowValueUnit->setCurrentItem(getItem(vunit));
	spacingBelowPlusUnit->setCurrentItem(getItem(punit));
	spacingBelowMinusUnit->setCurrentItem(getItem(munit));
}


void ParagraphDlgImpl::setExtra(float widthval, LyXLength::UNIT units, const string percent, int align, 
	bool hfill, bool startminipage, LyXParagraph::PEXTRA_TYPE type)
{
	if (type!=LyXParagraph::PEXTRA_NONE) {
		lyxerr[Debug::GUI] << "percent : $" << percent << "$ widthval " << widthval << " unit " << long(units) << endl;
		if (percent != "") {
			extraWidth->setText(percent.c_str());
			extraUnit->setCurrentItem(11);
		} else {
			extraWidth->setText(tostr(widthval).c_str());
			extraUnit->setCurrentItem(getItem(units));
		}
	} else
		extraWidth->setText("");
	
	switch (type) {
		case LyXParagraph::PEXTRA_NONE: 
			extraType->setCurrentItem(0); 
			break;
		case LyXParagraph::PEXTRA_MINIPAGE: 
			extraType->setCurrentItem(1); 
			enable_extraOptions(1); 
			enable_minipageOptions(1); 
			break;
		case LyXParagraph::PEXTRA_FLOATFLT: 
			extraType->setCurrentItem(2); 
			enable_extraOptions(2); 
			break;
		case LyXParagraph::PEXTRA_INDENT: 
			extraType->setCurrentItem(3); 
			enable_extraOptions(3); 
			break;
	}
	
	switch (align) {
		case LyXParagraph::MINIPAGE_ALIGN_TOP: 
			minipageValign->setCurrentItem(0); 
			break;
		case LyXParagraph::MINIPAGE_ALIGN_MIDDLE:	
			minipageValign->setCurrentItem(1); 
			break;
		case LyXParagraph::MINIPAGE_ALIGN_BOTTOM:	
			minipageValign->setCurrentItem(2); 
			break;
	}
	
	minipageHfill->setChecked(hfill);
	minipageStart->setChecked(startminipage);
}


LyXGlueLength ParagraphDlgImpl::getAboveLength() const
{
	LyXGlueLength len(
		spacingAboveValue->text().toFloat(),
		getLyXLength( spacingAboveValueUnit->currentItem() ),
		spacingAbovePlus->text().toFloat(),
		getLyXLength( spacingAbovePlusUnit->currentItem() ),
		spacingAboveMinus->text().toFloat(),
		getLyXLength( spacingAboveMinusUnit->currentItem() )
		);
	return len;
}

 
LyXGlueLength ParagraphDlgImpl::getBelowLength() const
{
	LyXGlueLength len(
		spacingBelowValue->text().toFloat(),
		getLyXLength( spacingBelowValueUnit->currentItem() ),
		spacingBelowPlus->text().toFloat(),
		getLyXLength( spacingBelowPlusUnit->currentItem() ),
		spacingBelowMinus->text().toFloat(),
		getLyXLength( spacingBelowMinusUnit->currentItem() )
		);
	return len;
}


LyXLength ParagraphDlgImpl::getExtraWidth() const
{
	if (extraUnit->currentItem()!=11) {
		LyXLength len( 
			 (extraWidth->text()).toDouble(), 
			 getLyXLength(extraUnit->currentItem())
			 );
		return len;
	} else {
		LyXLength len(0.0, LyXLength::UNIT_NONE);
		return len;
	}
}

 
string ParagraphDlgImpl::getExtraWidthPercent() const
{
	double val = (extraWidth->text()).toDouble();
	if (val > 100.0)
		val = 100.0;
	if (val < 0.0)
		val = 0.0;
	return tostr(val);
}


const char * ParagraphDlgImpl::getLabelWidth() const { 
	return labelWidth->text(); 
}


LyXAlignment ParagraphDlgImpl::getAlign() const {
	switch (alignment->currentItem()) {
		case 0: return LYX_ALIGN_BLOCK;
		case 1: return LYX_ALIGN_LEFT;
		case 2: return LYX_ALIGN_RIGHT;
		case 3: return LYX_ALIGN_CENTER;
		default: return LYX_ALIGN_CENTER;
	}
}
	
	
bool ParagraphDlgImpl::getAboveKeep() const { 
	return spacingAboveKeep->isChecked(); 
}

	
bool ParagraphDlgImpl::getBelowKeep() const { 
	return spacingBelowKeep->isChecked(); 
}

	
bool ParagraphDlgImpl::getLineAbove() const { 
	return lineAbove->isChecked(); 
}

	
bool ParagraphDlgImpl::getLineBelow() const { 
	return lineBelow->isChecked(); 
}

	
bool ParagraphDlgImpl::getPagebreakAbove() const { 
	return pagebreakAbove->isChecked(); 
}

	
bool ParagraphDlgImpl::getPagebreakBelow() const { 
	return pagebreakBelow->isChecked(); 
}

	
bool ParagraphDlgImpl::getNoIndent() const { 
	return noIndent->isChecked(); 
}

	
VSpace::vspace_kind ParagraphDlgImpl::getSpaceAboveKind() const {
	return getSpaceKind(spacingAbove->currentItem());
}

	
VSpace::vspace_kind ParagraphDlgImpl::getSpaceBelowKind() const {
	return getSpaceKind(spacingBelow->currentItem());
}


LyXParagraph::PEXTRA_TYPE ParagraphDlgImpl::getExtraType() const {
	switch (extraType->currentItem()) {
		case 0: return LyXParagraph::PEXTRA_NONE;
		case 1: return LyXParagraph::PEXTRA_MINIPAGE;
		case 2: return LyXParagraph::PEXTRA_FLOATFLT;
		case 3: return LyXParagraph::PEXTRA_INDENT;
	}
	return LyXParagraph::PEXTRA_NONE;
}

	
LyXParagraph::MINIPAGE_ALIGNMENT ParagraphDlgImpl::getExtraAlign() const {
	switch (minipageValign->currentItem()) {
		case 0: return LyXParagraph::MINIPAGE_ALIGN_TOP;
		case 1: return LyXParagraph::MINIPAGE_ALIGN_MIDDLE;
		case 2: return LyXParagraph::MINIPAGE_ALIGN_BOTTOM;
		default: return LyXParagraph::MINIPAGE_ALIGN_BOTTOM;
	}
}

	
bool ParagraphDlgImpl::getHfillBetween() const {
	return minipageHfill->isChecked();
}


bool ParagraphDlgImpl::getStartNewMinipage() const {
	return minipageStart->isChecked();
}


VSpace::vspace_kind ParagraphDlgImpl::getSpaceKind(int val) const {
	switch (val) {
		case 0: return VSpace::NONE;
		case 1: return VSpace::DEFSKIP;
		case 2: return VSpace::SMALLSKIP;
		case 3: return VSpace::MEDSKIP;
		case 4: return VSpace::BIGSKIP;
		case 5: return VSpace::VFILL;
		case 6: return VSpace::LENGTH;
		default:
			lyxerr[Debug::GUI] << "Unknown kind combo entry " << val << std::endl;
		}
	return VSpace::NONE;
}


LyXLength::UNIT ParagraphDlgImpl::getLyXLength(int val) const {
	switch (val) {
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
			lyxerr[Debug::GUI] << "Unknown kind combo entry " << val << std::endl;
	}
	return LyXLength::UNIT_NONE;
}

	
int ParagraphDlgImpl::getItem(LyXLength::UNIT unit) const {
	int item=0;
	switch (unit) {
		case LyXLength::CM: item = 0; break;
		case LyXLength::IN: item = 1; break;
		case LyXLength::PT: item = 2; break;
		case LyXLength::MM: item = 3; break;
		case LyXLength::PC: item = 4; break;
		case LyXLength::EX: item = 5; break;
		case LyXLength::EM: item = 6; break;
		case LyXLength::SP: item = 7; break;
		case LyXLength::BP: item = 8; break;
		case LyXLength::DD: item = 9; break;
		case LyXLength::CC: item = 10; break;
		case LyXLength::MU: item = 0; break;
		case LyXLength::UNIT_NONE: item = 0; break;
		default:
			lyxerr[Debug::GUI] << "Unknown unit " << long(unit) << endl;
	}
	return item;
}


void ParagraphDlgImpl::enable_extraOptions(int item)
{
	bool enable = (item!=0);
	extraWidth->setEnabled(enable);
	extraWidthL->setEnabled(enable);
	extraUnit->setEnabled(enable);
	extraUnitL->setEnabled(enable);
}


void ParagraphDlgImpl::enable_minipageOptions(int item)
{
	minipageOptions->setEnabled( item==1 );
}


void ParagraphDlgImpl::enable_spacingAbove(int item)
{
	bool enable = (item==6);
	spacingAboveValue->setEnabled(enable);
	spacingAbovePlus->setEnabled(enable);
	spacingAboveMinus->setEnabled(enable);
	spacingAboveValueUnit->setEnabled(enable);
	spacingAbovePlusUnit->setEnabled(enable);
	spacingAboveMinusUnit->setEnabled(enable);
	spacingAboveUnitsL->setEnabled(enable);
	bool enablel = ( (item==6) || (item!=6 && spacingBelow->currentItem()==6) );
	spacingValueL->setEnabled(enablel);
	spacingPlusL->setEnabled(enablel);
	spacingMinusL->setEnabled(enablel);
}


void ParagraphDlgImpl::enable_spacingBelow(int item)
{
	bool enable = (item==6);
	spacingBelowValue->setEnabled(enable);
	spacingBelowPlus->setEnabled(enable);
	spacingBelowMinus->setEnabled(enable);
	spacingBelowValueUnit->setEnabled(enable);
	spacingBelowPlusUnit->setEnabled(enable);
	spacingBelowMinusUnit->setEnabled(enable);
	spacingBelowUnitsL->setEnabled(enable);
	bool enablel = ( (item==6) || (item!=6 && spacingAbove->currentItem()==6) );
	spacingValueL->setEnabled(enablel);
	spacingPlusL->setEnabled(enablel);
	spacingMinusL->setEnabled(enablel);
}


void ParagraphDlgImpl::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}


void ParagraphDlgImpl::apply_adaptor()
{
	form_->apply();
}


void ParagraphDlgImpl::cancel_adaptor()  
{
	form_->close();
	hide();
}


void ParagraphDlgImpl::ok_adaptor()
{
	apply_adaptor();
	form_->close();
	hide();
}


void ParagraphDlgImpl::restore_adaptor()
{
	form_->update();
}
