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

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"

#include "support/lstrings.h"

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


QMathMatrixDialog::QMathMatrixDialog(QMath * form)
	: QMathMatrixDialogBase(0, 0, false, 0),
	form_(form)
{
	setCaption(_("LyX: Insert matrix"));

	table->setMinimumSize(100,100);
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
	char const * sh = halignED->text().latin1();
	int const nx = int(rowsSB->value());
	int const ny = int(columnsSB->value());

	ostringstream os;
	os << nx << ' ' << ny << ' ' << c << ' ' << sh;
	form_->insertMatrix(os.str().c_str());
	
	// close the dialog
	close();
}


void QMathMatrixDialog::slotClose()
{
	close();
}
