/**
 * \file QMathMatrixDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "qt_helpers.h"

#include "support/lstrings.h"
#include "Lsstream.h"
#include "ControlMath2.h"

#include "QMath.h"
#include "QMathMatrixDialog.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include "emptytable.h"


static char h_align_str[80] = "c";
static char v_align_c[] = "tcb";


QMathMatrixDialog::QMathMatrixDialog(QMathMatrix * form)
	: QMathMatrixDialogBase(0, 0, false, 0),
	form_(form)
{
	setCaption(qt_("LyX: Insert Matrix"));

	table->setMinimumSize(100, 100);
	rowsSB->setValue(2);
	columnsSB->setValue(2);
	valignCO->setCurrentItem(1);

	connect(okPB, SIGNAL(clicked()),
		this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
}


void QMathMatrixDialog::columnsChanged(int)
{
	int const nx = int(columnsSB->value());
	for (int i = 0; i < nx; ++i)
		h_align_str[i] = 'c';

	h_align_str[nx] = '\0';
	halignED->setText(h_align_str);

	return;
}


void QMathMatrixDialog::rowsChanged(int)
{
}


void QMathMatrixDialog::change_adaptor()
{
	// FIXME: We need a filter for the halign input
}


void QMathMatrixDialog::slotOK()
{
	char const c = v_align_c[valignCO->currentItem()];
	string const sh = fromqstr(halignED->text());
	int const nx = int(columnsSB->value());
	int const ny = int(rowsSB->value());

	ostringstream os;
	os << nx << ' ' << ny << ' ' << c << ' ' << sh;
	form_->controller().dispatchMatrix(os.str().c_str());

	// close the dialog
	close();
}


void QMathMatrixDialog::slotClose()
{
	close();
}
