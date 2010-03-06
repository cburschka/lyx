/**
 * \file GuiMathMatrix.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiMathMatrix.h"

#include "EmptyTable.h"
#include "qt_helpers.h"

#include "FuncRequest.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

using namespace std;

namespace lyx {
namespace frontend {

GuiMathMatrix::GuiMathMatrix(GuiView & lv)
	: GuiDialog(lv, "mathmatrix", qt_("Math Matrix"))
{
	setupUi(this);

	table->setMinimumSize(100, 100);
	rowsSB->setValue(5);
	columnsSB->setValue(5);
	valignCO->setCurrentIndex(1);
	decorationCO->setCurrentIndex(0);

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
	connect(decorationCO, SIGNAL(activated(int)),
		this, SLOT(decorationChanged(int)));

	bc().setPolicy(ButtonPolicy::IgnorantPolicy);
}


void GuiMathMatrix::columnsChanged(int)
{
	int const nx = int(columnsSB->value());
	halignED->setText(QString(nx, 'c'));
}


void GuiMathMatrix::rowsChanged(int)
{
}


void GuiMathMatrix::decorationChanged(int deco)
{
	// a matrix with a decoration cannot have a vertical alignment
	if (deco != 0) {
		alignmentGB->setEnabled(false);
		valignCO->setCurrentIndex(1);
		halignED->clear();
	} else
		alignmentGB->setEnabled(true);
}


void GuiMathMatrix::change_adaptor()
{
	// FIXME: We need a filter for the halign input
}


void GuiMathMatrix::slotOK()
{
	int const nx = columnsSB->value();
	int const ny = rowsSB->value();
	// a matrix without a decoration is an array,
	// otherwise it is an AMS matrix that cannot have a vertical alignment
	if (decorationCO->currentIndex() == 0) {
		char v_align_c[] = "tcb";
		char const c = v_align_c[valignCO->currentIndex()];
		QString const sh = halignED->text();
		string const str = fromqstr(
			QString("%1 %2 %3 %4").arg(nx).arg(ny).arg(c).arg(sh));
		dispatch(FuncRequest(LFUN_MATH_MATRIX, str));
	} else {
		int const deco = decorationCO->currentIndex();
		QString deco_name;
		switch (deco) {
			case 1: deco_name = "bmatrix";
				break;
			case 2: deco_name = "pmatrix";
				break;
			case 3: deco_name = "Bmatrix";
				break;
			case 4: deco_name = "vmatrix";
				break;
			case 5: deco_name = "Vmatrix";
				break;
		}
		string const str_ams = fromqstr(
			QString("%1 %2 %3").arg(nx).arg(ny).arg(deco_name));
		dispatch(FuncRequest(LFUN_MATH_AMS_MATRIX, str_ams));
	}
	close();
}


void GuiMathMatrix::slotClose()
{
	close();
}


Dialog * createGuiMathMatrix(GuiView & lv) { return new GuiMathMatrix(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiMathMatrix.cpp"
