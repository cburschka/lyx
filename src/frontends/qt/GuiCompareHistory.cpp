/**
 * \file GuiCompareHistory.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCompareHistory.h"

#include "Buffer.h"
#include "BufferView.h"
#include "FuncRequest.h"
#include "GuiView.h"
#include "LyXVC.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lstrings.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <limits>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiCompareHistory::GuiCompareHistory(GuiView & lv)
	: GuiDialog(lv, "comparehistory", qt_("Compare different revisions"))

{
	setupUi(this);
	setModal(Qt::WindowModal);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

	connect(revbackRB, SIGNAL(clicked()), this, SLOT(selectRevback()));
	connect(betweenrevRB, SIGNAL(clicked()), this, SLOT(selectBetweenrev()));
}

bool GuiCompareHistory::initialiseParams(std::string const &)
{
	string revstring = lyxview().currentBufferView()->buffer().lyxvc().revisionInfo(LyXVC::File);
	int rev = 0;

	string tmp;
	bool enableBetween = true;
	// GIT case, hash is long
	if (revstring.length() > 20) {
		enableBetween = false;
		rev = numeric_limits<int>::max();
	} else {
		// RCS case
		if (!isStrInt(revstring))
			revstring = rsplit(revstring, tmp , '.' );
		// both SVN & RCS cases
		if (isStrInt(revstring))
			rev = convert<int>(revstring);
	}

	// later we can provide comparison between two hashes
	betweenrevRB->setEnabled(enableBetween);

	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(rev);
	rev1SB->setMaximum(rev);
	rev2SB->setMaximum(rev);
	revbackSB->setMaximum(rev-1);
	rev2SB->setValue(rev);
	rev1SB->setValue(rev-1);

	//bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	//bc().setOK(okPB);
	//bc().setCancel(cancelPB);
	enableControls();
	return true;
}


void GuiCompareHistory::updateContents()
{
	enableControls();
}


void GuiCompareHistory::selectRevback()
{
	betweenrevRB->setChecked(false);
	enableControls();
}


void GuiCompareHistory::selectBetweenrev()
{
	revbackRB->setChecked(false);
	enableControls();
}


void GuiCompareHistory::enableControls()
{
	bool rb = revbackRB->isChecked();
	oldL->setEnabled(!rb);
	newL->setEnabled(!rb);
	rev1SB->setEnabled(!rb);
	rev2SB->setEnabled(!rb);
	rev2SB->setEnabled(!rb);
	betweenrevRB->setChecked(!rb);
	revbackSB->setEnabled(rb);
}


void GuiCompareHistory::slotOK()
{
	string param;
	if (revbackRB->isChecked())
		param = "-" + convert<string>(revbackSB->value());
	else
		param = convert<string>(rev1SB->value()) +
			+ " " + convert<string>(rev2SB->value());

	GuiDialog::slotClose();
	dispatch(FuncRequest(LFUN_VC_COMPARE, param));
}


void GuiCompareHistory::slotCancel()
{
	GuiDialog::slotClose();
}


void GuiCompareHistory::slotButtonBox(QAbstractButton * button)
{
	switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Ok:
		slotOK();
		break;
	case QDialogButtonBox::Cancel:
		slotCancel();
		break;
	default:
		break;
	}
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiCompareHistory.cpp"
