/**
 * \file InsetParamsDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetParamsDialog.h"

#include "GuiBox.h"
#include "GuiBranch.h"
#include "GuiBibitem.h"
#include "GuiERT.h"
#include "GuiInfo.h"
#include "GuiHSpace.h"
#include "GuiTabular.h"
#include "GuiVSpace.h"
#include "FloatPlacement.h"

#include "InsetParamsWidget.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"

#include "support/debug.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////
//
// InsetParamsDialog::Private
//
/////////////////////////////////////////////////////////////////

struct InsetParamsDialog::Private
{
	///
	InsetParamsWidget * widget_;
};

/////////////////////////////////////////////////////////////////
//
// InsetParamsDialog
//
/////////////////////////////////////////////////////////////////

InsetParamsDialog::InsetParamsDialog(GuiView & lv, InsetParamsWidget * widget)
	: DialogView(lv, toqstr(insetName(widget->insetCode())),
	toqstr(insetName(widget->insetCode()))), d(new Private)
{
	setupUi(this);
	setInsetParamsWidget(widget);
	synchronizedViewCB->setChecked(false);
	on_synchronizedViewCB_stateChanged(false);
}

InsetParamsDialog::~InsetParamsDialog()
{
	delete d;
}


bool InsetParamsDialog::initialiseParams(std::string const &)
{
	on_restorePB_clicked();
	return true;
}

void InsetParamsDialog::setInsetParamsWidget(InsetParamsWidget * widget)
{
	d->widget_ = widget;
	stackedWidget->addWidget(widget);
	stackedWidget->setCurrentWidget(widget);
	connect(d->widget_, SIGNAL(changed()), this, SLOT(onWidget_changed()));
}


void InsetParamsDialog::on_restorePB_clicked()
{
	updateView(true);
	restorePB->setEnabled(false);
}


void InsetParamsDialog::on_okPB_clicked()
{
	Inset const * i = inset(d->widget_->insetCode());
	if (i)
		applyView();
	else
		newInset();
	hide();
}


void InsetParamsDialog::newInset()
{
	docstring const argument = d->widget_->dialogToParams();
	dispatch(FuncRequest(d->widget_->creationCode(), argument));
}


void InsetParamsDialog::on_newPB_clicked()
{
	newInset();
}


void InsetParamsDialog::on_applyPB_clicked()
{
	applyView();
}


void InsetParamsDialog::on_closePB_clicked()
{
	hide();
}


void InsetParamsDialog::on_synchronizedViewCB_stateChanged(int state)
{
	checkWidgets(state == Qt::Checked);
}


docstring InsetParamsDialog::checkWidgets(bool synchronized_view)
{
	bool const widget_ok = d->widget_->checkWidgets();
	Inset const * ins = inset(d->widget_->insetCode());
	docstring const argument = d->widget_->dialogToParams();
	bool valid_argument = !argument.empty();
	if (ins)
		valid_argument &= ins->validateModifyArgument(argument);
	FuncCode const code = synchronized_view
		? d->widget_->creationCode() : LFUN_INSET_MODIFY;
	bool const lfun_ok = lyx::getStatus(FuncRequest(code, argument)).enabled();
	bool const read_only = buffer().isReadonly();

	okPB->setEnabled(!synchronized_view && widget_ok && !read_only && valid_argument);
	restorePB->setEnabled(!synchronized_view && ins && !read_only);
	applyPB->setEnabled(lfun_ok && widget_ok && !read_only && valid_argument);
	d->widget_->setEnabled(!read_only);
	return argument;
}


void InsetParamsDialog::onWidget_changed()
{
	docstring const argument = checkWidgets(synchronizedViewCB->isChecked());
	if (synchronizedViewCB->isChecked())
		dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
}


void InsetParamsDialog::applyView()
{
	docstring const argument = checkWidgets(synchronizedViewCB->isChecked());
	dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
}


void InsetParamsDialog::updateView(bool update_widget)
{
	if (update_widget) {
		Inset const * i = inset(d->widget_->insetCode());
		if (i) {
			d->widget_->blockSignals(true);
			d->widget_->paramsToDialog(i);
			d->widget_->blockSignals(false);
		}
	}
	checkWidgets(synchronizedViewCB->isChecked());
}


void InsetParamsDialog::updateView()
{
	updateView(synchronizedViewCB->isChecked());
}


Dialog * createDialog(GuiView & lv, InsetCode code)
{
	InsetParamsWidget * widget;
	switch (code) {
	case ERT_CODE:
		widget = new GuiERT;
		break;
	case FLOAT_CODE:
		widget = new FloatPlacement(true);
		break;
	case BIBITEM_CODE:
		widget = new GuiBibitem;
		break;
	case BRANCH_CODE:
		widget = new GuiBranch;
		break;
	case BOX_CODE:
		widget = new GuiBox;
		break;
	case INFO_CODE:
		widget = new GuiInfo;
		break;
	case MATH_SPACE_CODE:
		widget = new GuiHSpace(true);
		break;
	case SPACE_CODE:
		widget = new GuiHSpace(false);
		break;
	case TABULAR_CODE:
		widget = new GuiTabular;
		break;
	case VSPACE_CODE:
		widget = new GuiVSpace;
		break;
	default: return 0;
	}
	InsetParamsDialog * dialog = new InsetParamsDialog(lv, widget);
	return dialog;
}


Dialog * createDialog(GuiView & lv, string const & name)
{
	return createDialog(lv, insetCode(name));
}

} // namespace frontend
} // namespace lyx

#include "moc_InsetParamsDialog.cpp"
