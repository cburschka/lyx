/**
 * \file QMinipage.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include "lengthcombo.h"
 
#include "QMinipageDialog.h"
#include "QMinipage.h"
#include "Qt2BC.h"
#include "gettext.h"
#include "support/lstrings.h"
 
#include "QtLyXView.h"
#include "ControlMinipage.h"

#include "debug.h"
typedef Qt2CB<ControlMinipage, Qt2DB<QMinipageDialog> > base_class;

QMinipage::QMinipage(ControlMinipage & c)
	: base_class(c, _("Minipage"))
{
}


void QMinipage::build_dialog()
{
	dialog_.reset(new QMinipageDialog(this));

	bc().setRestore(dialog_->restorePB);
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);

	bc().addReadOnly(dialog_->widthED);
	bc().addReadOnly(dialog_->unitsLC);
	bc().addReadOnly(dialog_->valignCO); 
}


void QMinipage::apply()
{
	double value = strToDbl(dialog_->widthED->text().latin1());
	LyXLength::UNIT unit = dialog_->unitsLC->currentLengthItem();
	if (string(dialog_->widthED->text().latin1()).empty())
		unit = LyXLength::UNIT_NONE;

	LyXLength len(value, unit);
 
	controller().params().pageWidth = len.asString();

	switch (dialog_->valignCO->currentItem()) {
	case 0:
		controller().params().pos = InsetMinipage::top;
		break;
	case 1:
		controller().params().pos = InsetMinipage::center;
		break;
	case 2:
		controller().params().pos = InsetMinipage::bottom;
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
 
 
void QMinipage::update_contents()
{
	LyXLength len(controller().params().pageWidth.c_str());
	dialog_->widthED->setText(numtostr(len.value()).c_str());
	dialog_->unitsLC->setCurrentItem(len.unit());
	lyxerr << "width " << numtostr(len.value()).c_str() << " units " << len.unit() << std::endl;
 
	int item = 0;
	switch (controller().params().pos) {
		case InsetMinipage::center:
			item = 1;
			break;
		case InsetMinipage::bottom:
			item = 2;
			break;
	}
	dialog_->valignCO->setCurrentItem(item); 
}
