/**
 * \file QMinipage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
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
#include "ControlMinipage.h"

#include "QMinipage.h"
#include "QMinipageDialog.h"
#include "Qt2BC.h"
#include "lengthcombo.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>

typedef Qt2CB<ControlMinipage, Qt2DB<QMinipageDialog> > base_class;


QMinipage::QMinipage()
	: base_class(_("Minipage"))
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
	double const value = strToDbl(dialog_->widthED->text().latin1());
	LyXLength::UNIT unit = dialog_->unitsLC->currentLengthItem();
	if (string(dialog_->widthED->text().latin1()).empty())
		unit = LyXLength::UNIT_NONE;

	MinipageParams & params = controller().params();

	params.pageWidth = LyXLength(value, unit);

	switch (dialog_->valignCO->currentItem()) {
	case 0:
		params.pos = InsetMinipage::top;
		break;
	case 1:
		params.pos = InsetMinipage::center;
		break;
	case 2:
		params.pos = InsetMinipage::bottom;
		break;
	}
}


namespace {

string const numtostr(double val)
{
	string a(tostr(val));
	if (a == "0")
		a.erase();
	return a;
}

} // namespace anon


void QMinipage::update_contents()
{
	MinipageParams const & params = controller().params();

	LyXLength len(params.pageWidth);
	dialog_->widthED->setText(numtostr(len.value()).c_str());
	dialog_->unitsLC->setCurrentItem(len.unit());
	lyxerr << "width " << numtostr(len.value())
	       << " units " << len.unit() << std::endl;

	int item = 0;
	switch (params.pos) {
	case InsetMinipage::top:
		item = 0;
		break;
	case InsetMinipage::center:
		item = 1;
		break;
	case InsetMinipage::bottom:
		item = 2;
		break;
	}
	dialog_->valignCO->setCurrentItem(item);
}
