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


#include "debug.h"
#include "qt_helpers.h"
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

typedef QController<ControlMinipage, QView<QMinipageDialog> > base_class;


QMinipage::QMinipage(Dialog & parent)
	: base_class(parent, qt_("LyX: Minipage Settings"))
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
	double const value = strToDbl(fromqstr(dialog_->widthED->text()));
	LyXLength::UNIT unit = dialog_->unitsLC->currentLengthItem();
	if (dialog_->widthED->text().isEmpty())
		unit = LyXLength::UNIT_NONE;

	InsetMinipage::Params & params = controller().params();

	params.width = LyXLength(value, unit);

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
	InsetMinipage::Params const & params = controller().params();

	LyXLength len(params.width);
	dialog_->widthED->setText(toqstr(numtostr(len.value())));
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
