/**
 * \file QParagraphDialog.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include "QParagraphDialog.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qvalidator.h>

#include "helper_funcs.h"

using std::vector;

QParagraphDialog::QParagraphDialog(QParagraph * form)
	: QParagraphDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
	

	linespacingValue->setValidator(new QDoubleValidator(linespacingValue));
	valueAbove->setValidator(new QDoubleValidator(valueAbove));
	valueBelow->setValidator(new QDoubleValidator(valueBelow));
}

void QParagraphDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QParagraphDialog::change_adaptor()
{
	form_->changed();
}

void QParagraphDialog::enableAbove(int)
{
	bool const enable = spacingAbove->currentItem()==6;
	valueAbove->setEnabled(enable);
	unitAbove->setEnabled(enable);
	
}

void QParagraphDialog::enableBelow(int)
{
	bool const enable = spacingBelow->currentItem()==6;
	valueBelow->setEnabled(enable);
	unitBelow->setEnabled(enable);
}

void QParagraphDialog::enableLinespacingValue(int)
{
	bool const enable = linespacing->currentItem()==4;
	linespacingValue->setEnabled(enable);
}
