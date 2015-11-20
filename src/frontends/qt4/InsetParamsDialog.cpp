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
#include "GuiHSpace.h"
#include "GuiHyperlink.h"
#include "GuiInfo.h"
#include "GuiLabel.h"
#include "GuiLine.h"
#include "GuiNomenclature.h"
#include "GuiPrintNomencl.h"
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
	Private() : widget_(0), inset_(0), changed_(false) {}
	///
	InsetParamsWidget * widget_;
	/// The inset that was used at last Restore or Apply operation.
	Inset const * inset_;
	/// Set to true whenever the dialog is changed and set back to
	/// false when the dialog is applied or restored.
	bool changed_;
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
	immediateApplyCB->setChecked(false);
	synchronizedCB->setChecked(true);
	on_immediateApplyCB_stateChanged(false);
	setFocusProxy(widget);
}

InsetParamsDialog::~InsetParamsDialog()
{
	delete d;
}


bool InsetParamsDialog::initialiseParams(std::string const & data)
{
	if (!d->widget_->initialiseParams(data))
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
	d->changed_ = false;
	d->inset_ = inset(d->widget_->insetCode());
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


void InsetParamsDialog::on_immediateApplyCB_stateChanged(int state)
{
	checkWidgets(state == Qt::Checked);
}


void InsetParamsDialog::on_synchronizedCB_stateChanged(int)
{
	checkWidgets(false);
}


docstring InsetParamsDialog::checkWidgets(bool immediate)
{
	bool const widget_ok = d->widget_->checkWidgets();
	Inset const * ins = inset(d->widget_->insetCode());
	docstring const argument = d->widget_->dialogToParams();
	bool valid_argument = !argument.empty();
	if (ins)
		valid_argument &= ins->validateModifyArgument(argument);
	FuncCode const code = immediate
		? d->widget_->creationCode() : LFUN_INSET_MODIFY;
	bool const lfun_ok = lyx::getStatus(FuncRequest(code, argument)).enabled();
	bool const read_only = buffer().isReadonly();

	okPB->setEnabled(!immediate && widget_ok && !read_only && valid_argument);
	bool const can_be_restored = !immediate && !read_only
			&& ins && (ins != d->inset_ || d->changed_);
	restorePB->setEnabled(can_be_restored);
	applyPB->setEnabled(ins && !immediate && lfun_ok && widget_ok && !read_only && valid_argument);
	d->widget_->setEnabled(!read_only);
	synchronizedCB->setEnabled(!immediate);
	return argument;
}


void InsetParamsDialog::onWidget_changed()
{
	d->changed_ = true;
	docstring const argument = checkWidgets(immediateApplyCB->isChecked());
	if (immediateApplyCB->isChecked())
		dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
}


void InsetParamsDialog::applyView()
{
	docstring const argument = checkWidgets(immediateApplyCB->isChecked());
	dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
	d->changed_ = false;
	d->inset_ = inset(d->widget_->insetCode());
	updateView(true);
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
	checkWidgets(immediateApplyCB->isChecked());
}


void InsetParamsDialog::updateView()
{
	bool const update_widget =
		(synchronizedCB->isChecked() || immediateApplyCB->isChecked());
	updateView(update_widget);
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
	case HYPERLINK_CODE:
		widget = new GuiHyperlink;
		break;
	case INFO_CODE:
		widget = new GuiInfo;
		break;
	case LABEL_CODE:
		widget = new GuiLabel;
		break;
	case LINE_CODE:
		widget = new GuiLine;
		break;
	case MATH_SPACE_CODE:
		widget = new GuiHSpace(true);
		break;
	case NOMENCL_CODE:
		widget = new GuiNomenclature;
		break;
	case NOMENCL_PRINT_CODE:
		widget = new GuiPrintNomencl;
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
