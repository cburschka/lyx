/**
 * \file QWrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QWrap.h"
#include "QWrapDialog.h"
#include "Qt2BC.h"

#include "lengthcombo.h"
#include "qt_helpers.h"

#include "controllers/ControlWrap.h"

#include "insets/insetwrap.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <qlineedit.h>
#include <qpushbutton.h>

using lyx::support::strToDbl;

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlWrap, QView<QWrapDialog> > base_class;

QWrap::QWrap(Dialog & parent)
	: base_class(parent, _("LyX: Text Wrap Settings"))
{
}


void QWrap::build_dialog()
{
	dialog_.reset(new QWrapDialog(this));

	bcview().setRestore(dialog_->restorePB);
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->widthED);
	bcview().addReadOnly(dialog_->unitsLC);
	bcview().addReadOnly(dialog_->valignCO);
}


void QWrap::apply()
{
	double const value = strToDbl(fromqstr(dialog_->widthED->text()));
	LyXLength::UNIT unit = dialog_->unitsLC->currentLengthItem();
	if (dialog_->widthED->text().isEmpty())
		unit = LyXLength::UNIT_NONE;

	InsetWrapParams & params = controller().params();

	params.width = LyXLength(value, unit);

	switch (dialog_->valignCO->currentItem()) {
	case 0:
		params.placement.erase();
		break;
	case 1:
		params.placement = "l";
		break;
	case 2:
		params.placement = "r";
		break;
	case 3:
		params.placement = "p";
		break;
	}
}


namespace {

string const numtostr(double val) {
	string a(convert<string>(val));
#ifdef WITH_WARNINGS
#warning Will this test ever trigger? (Lgb)
#endif
	if (a == "0")
		a.erase();
	return a;
}

} // namespace anon


void QWrap::update_contents()
{
	InsetWrapParams & params = controller().params();

	LyXLength len(params.width);
	dialog_->widthED->setText(toqstr(numtostr(len.value())));
	dialog_->unitsLC->setCurrentItem(len.unit());

	int item = 0;
	if (params.placement == "l")
		item = 1;
	else if (params.placement == "r")
		item = 2;
	else if (params.placement == "p")
		item = 3;

	dialog_->valignCO->setCurrentItem(item);
}

} // namespace frontend
} // namespace lyx
