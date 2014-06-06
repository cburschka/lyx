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

#include "support/gettext.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

using namespace std;

namespace lyx {
namespace frontend {

static char const * const DecoChars[] = {
	N_("None"),
	N_("[x]"),
	N_("(x)"),
	N_("{x}"),
	N_("|x|"),
	N_("||x||"),
	""
};

static char const * const DecoNames[] = {
	N_("bmatrix"),
	N_("pmatrix"),
	N_("Bmatrix"),
	N_("vmatrix"),
	N_("Vmatrix"),
	""
};

static char const * const VertAligns[] = {
	N_("Top"),
	N_("Middle"),
	N_("Bottom"),
	""
};

static char const v_align_c[] = "tcb";


GuiMathMatrix::GuiMathMatrix(GuiView & lv)
	: GuiDialog(lv, "mathmatrix", qt_("Math Matrix"))
{
	setupUi(this);

	for (int i = 0; *VertAligns[i]; ++i)
		valignCO->addItem(qt_(VertAligns[i]));
	for (int i = 0; *DecoChars[i]; ++i)
		decorationCO->addItem(qt_(DecoChars[i]));

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
		this, SLOT(change_adaptor()));
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


void GuiMathMatrix::decorationChanged(int deco)
{
	// a matrix with a decoration cannot have a vertical alignment
	if (deco != 0) {
		valignCO->setEnabled(false);
		valignCO->setCurrentIndex(1);
	} else
		valignCO->setEnabled(true);
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
	// otherwise it is an AMS matrix
	// decorated matrices cannot have a vertical alignment
	
	char const c = v_align_c[valignCO->currentIndex()];
	QString const sh = halignED->text();
	string const str = fromqstr(
		QString("%1 %2 %3 %4").arg(nx).arg(ny).arg(c).arg(sh));

	if (decorationCO->currentIndex() != 0) {
		int const deco = decorationCO->currentIndex();
		QString deco_name = DecoNames[deco - 1];
		// only if a special alignment is set create a 1x1 AMS array in which
		// a normal array will be created, otherwise create just a normal AMS array
		if (sh.contains('l') || sh.contains('r')) {
			string const str_ams = fromqstr(
				QString("%1 %2 %3").arg(int(1)).arg(int(1)).arg(deco_name));
			dispatch(FuncRequest(LFUN_MATH_AMS_MATRIX, str_ams));
		} else {
			string const str_ams = fromqstr(
				QString("%1 %2 %3").arg(nx).arg(ny).arg(deco_name));
			dispatch(FuncRequest(LFUN_MATH_AMS_MATRIX, str_ams));
			close();
			return;
		}
	}
	// create the normal array
		dispatch(FuncRequest(LFUN_MATH_MATRIX, str));
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
