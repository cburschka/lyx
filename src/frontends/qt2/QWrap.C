/**
 * \file QWrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"
#include "gettext.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "ControlWrap.h"

#include "QWrap.h"
#include "QWrapDialog.h"
#include "Qt2BC.h"
#include "lengthcombo.h"
 
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>

typedef Qt2CB<ControlWrap, Qt2DB<QWrapDialog> > base_class;

QWrap::QWrap()
	: base_class(_("Wrap Options"))
{
}


void QWrap::build_dialog()
{
	dialog_.reset(new QWrapDialog(this));

	bc().setRestore(dialog_->restorePB);
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);

	bc().addReadOnly(dialog_->widthED);
	bc().addReadOnly(dialog_->unitsLC);
	bc().addReadOnly(dialog_->valignCO);
}


void QWrap::apply()
{
	double const value = strToDbl(dialog_->widthED->text().latin1());
	LyXLength::UNIT unit = dialog_->unitsLC->currentLengthItem();
	if (string(dialog_->widthED->text().latin1()).empty())
		unit = LyXLength::UNIT_NONE;

	controller().params().pageWidth = LyXLength(value, unit);

	switch (dialog_->valignCO->currentItem()) {
	case 0:
		controller().params().placement.erase();
		break;
	case 1:
		controller().params().placement = "l";
		break;
	case 2:
		controller().params().placement = "r";
		break;
	case 3:
		controller().params().placement = "p";
		break;
	}
}


namespace {
	string const numtostr(double val) {
		string a(tostr(val));
		if (a == "0")
			a = "";
		return a;
	}
} // namespace anon


void QWrap::update_contents()
{
	LyXLength len(controller().params().pageWidth);
	dialog_->widthED->setText(numtostr(len.value()).c_str());
	dialog_->unitsLC->setCurrentItem(len.unit());

	int item = 0;
	if (controller().params().placement == "l")
		item = 1;
	else if (controller().params().placement == "r")
		item = 2;
	else if (controller().params().placement == "p")
		item = 3;

	dialog_->valignCO->setCurrentItem(item);
}
