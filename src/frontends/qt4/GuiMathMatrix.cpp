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
#include "gettext.h"

#include <sstream>

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {

GuiMathMatrix::GuiMathMatrix(GuiDialog & parent)
	: GuiView<GuiMathMatrixDialog>(parent, _("Math Matrix"))
{}


void GuiMathMatrix::build_dialog()
{
	dialog_.reset(new GuiMathMatrixDialog(this));
}


GuiMathMatrixDialog::GuiMathMatrixDialog(GuiMathMatrix * form)
	: form_(form)
{
	setupUi(this);

	setWindowTitle(qt_("LyX: Insert Matrix"));

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
	connect(valignCO, SIGNAL(highlighted(const QString&)),
		this, SLOT(change_adaptor()));
	connect(halignED, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
}


void GuiMathMatrixDialog::columnsChanged(int)
{
	char h_align_str[80] = "c";
	int const nx = int(columnsSB->value());
	for (int i = 0; i < nx; ++i)
		h_align_str[i] = 'c';

	h_align_str[nx] = '\0';
	halignED->setText(h_align_str);
}


void GuiMathMatrixDialog::rowsChanged(int)
{
}


void GuiMathMatrixDialog::change_adaptor()
{
	// FIXME: We need a filter for the halign input
}


void GuiMathMatrixDialog::slotOK()
{
	char v_align_c[] = "tcb";
	char const c = v_align_c[valignCO->currentIndex()];
	string const sh = fromqstr(halignED->text());
	int const nx = int(columnsSB->value());
	int const ny = int(rowsSB->value());

	ostringstream os;
	os << nx << ' ' << ny << ' ' << c << ' ' << sh;
	form_->controller().dispatchMatrix(os.str().c_str());

	// close the dialog
	close();
}


void GuiMathMatrixDialog::slotClose()
{
	close();
}

} // namespace frontend
} // namespace lyx

#include "GuiMathMatrix_moc.cpp"
