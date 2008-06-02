/**
 * \file GuiMathMatrix.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiMathMatrix.h"

#include "EmptyTable.h"
#include "qt_helpers.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

using namespace std;

namespace lyx {
namespace frontend {

GuiMathMatrix::GuiMathMatrix(GuiView & lv)
	: GuiMath(lv, "mathmatrix", qt_("Math Matrix"))
{
	setupUi(this);

	table->setMinimumSize(100, 100);
	rowsSB->setValue(5);
	columnsSB->setValue(5);
	valignCO->setCurrentIndex(1);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(table, SIGNAL(rowsChanged(int)),
		rowsSB, SLOT(setValue(int)));
	connect(table, SIGNAL(colsChanged(int)),
		columnsSB, SLOT(setValue(int)));
	connect(rowsSB, SIGNAL(valueChanged(int)),
		table, SLOT(setNumberRows(int)));
	connect(columnsSB, SIGNAL(valueChanged(int)),
		table, SLOT(setNumberColumns(int)));
	connect(rowsSB, SIGNAL(valueChanged(int)),
		this, SLOT(rowsChanged(int)));
	connect(columnsSB, SIGNAL(valueChanged(int)),
		this, SLOT(columnsChanged(int)) );
	connect(valignCO, SIGNAL(highlighted(QString)),
		this, SLOT(change_adaptor()));
	connect(halignED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::IgnorantPolicy);
}


void GuiMathMatrix::columnsChanged(int)
{
	char h_align_str[80] = "c";
	int const nx = int(columnsSB->value());
	for (int i = 0; i < nx; ++i)
		h_align_str[i] = 'c';

	h_align_str[nx] = '\0';
	halignED->setText(h_align_str);
}


void GuiMathMatrix::rowsChanged(int)
{
}


void GuiMathMatrix::change_adaptor()
{
	// FIXME: We need a filter for the halign input
}


void GuiMathMatrix::slotOK()
{
	char v_align_c[] = "tcb";
	char const c = v_align_c[valignCO->currentIndex()];
	QString const sh = halignED->text();
	int const nx = columnsSB->value();
	int const ny = rowsSB->value();
	string const str = fromqstr(
		QString("%1 %2 %3 %4").arg(nx).arg(ny).arg(c).arg(sh));
	dispatchFunc(LFUN_MATH_MATRIX, str);
	close();
}


void GuiMathMatrix::slotClose()
{
	close();
}


Dialog * createGuiMathMatrix(GuiView & lv) { return new GuiMathMatrix(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiMathMatrix_moc.cpp"
