/**
 * \file QMinipage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "qt_helpers.h"
#include "support/tostr.h"
#include "support/lstrings.h"
#include "ControlMinipage.h"

#include "QMinipage.h"
#include "QMinipageDialog.h"
#include "Qt2BC.h"
#include "lengthcombo.h"

#include <qpushbutton.h>
#include <qlineedit.h>

using lyx::support::strToDbl;

using std::string;


typedef QController<ControlMinipage, QView<QMinipageDialog> > base_class;

QMinipage::QMinipage(Dialog & parent)
	: base_class(parent, _("LyX: Minipage Settings"))
{
}


void QMinipage::build_dialog()
{
	dialog_.reset(new QMinipageDialog(this));

	bcview().setRestore(dialog_->restorePB);
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->widthED);
	bcview().addReadOnly(dialog_->unitsLC);
	bcview().addReadOnly(dialog_->valignCO);
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
