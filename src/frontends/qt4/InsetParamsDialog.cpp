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
#include "GuiERT.h"
#include "GuiInfo.h"
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


void InsetParamsDialog::setInsetParamsWidget(InsetParamsWidget * widget)
{
	d->widget_ = widget;
	stackedWidget->addWidget(widget);
	stackedWidget->setCurrentWidget(widget);
}


void InsetParamsDialog::on_restorePB_clicked()
{
	updateView();
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


void InsetParamsDialog::on_applyPB_clicked()
{
	if (synchronizedViewCB->isChecked())
		newInset();
	else
		applyView();
}


void InsetParamsDialog::on_closePB_clicked()
{
	hide();
}


void InsetParamsDialog::on_synchronizedViewCB_stateChanged(int state)
{
	bool const sync = (state == Qt::Checked);
	QString const label = sync ? qt_("&New") :  qt_("&Apply");
	applyPB->setText(label);
	okPB->setEnabled(!sync && d->widget_->checkWidgets());
	restorePB->setEnabled(!sync);
	if (sync)
		connect(d->widget_, SIGNAL(changed()), this, SLOT(applyView()));
	else {
		applyPB->setEnabled(d->widget_->checkWidgets());
		d->widget_->setEnabled(!buffer().isReadonly());
		QObject::disconnect(d->widget_, SIGNAL(changed()), this, SLOT(applyView()));
	}
}


void InsetParamsDialog::applyView()
{
	if (!d->widget_->checkWidgets())
		return;

	Inset const * i = inset(d->widget_->insetCode());
	if (!i)
		return;
	
	docstring const argument = d->widget_->dialogToParams();

	if (argument.empty() || !i->validateModifyArgument(argument)) {
		applyPB->setEnabled(false);
		return;
	}
	applyPB->setEnabled(true);
	dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
}


void InsetParamsDialog::updateView()
{
	Inset const * i = inset(d->widget_->insetCode());
	if (i)
		d->widget_->paramsToDialog(i);
	else
		d->widget_->setEnabled(false);

	docstring const argument = d->widget_->dialogToParams();
	FuncCode const code = synchronizedViewCB->isChecked()
		? d->widget_->creationCode() : LFUN_INSET_MODIFY;
	applyPB->setEnabled(
		lyx::getStatus(FuncRequest(code, argument)).enabled());
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
	case BOX_CODE:
		widget = new GuiBox;
		break;
	case INFO_CODE:
		widget = new GuiInfo;
		break;
	//FIXME: not ready yet.
	/*
	case TABULAR_CODE:
		widget = new GuiTabular;
		break;
	*/
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
