/**
 * \file QMathMatrixDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QMathMatrixDialog.h"

#include "EmptyTable.h"
#include "qt_helpers.h"

#include "controllers/ControlMath.h"

#include <sstream>

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include "gettext.h"

using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {

namespace {

char h_align_str[80] = "c";
char v_align_c[] = "tcb";

} // namespace anon


typedef QController<ControlMath, QView<QMathMatrixDialog> > matrix_base;


QMathMatrix::QMathMatrix(Dialog & parent)
	: matrix_base(parent, _("Math Matrix"))
{}


void QMathMatrix::build_dialog()
{
	dialog_.reset(new QMathMatrixDialog(this));
}


QMathMatrixDialog::QMathMatrixDialog(QMathMatrix * form)
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


void QMathMatrixDialog::columnsChanged(int)
{
	int const nx = int(columnsSB->value());
	for (int i = 0; i < nx; ++i)
		h_align_str[i] = 'c';

	h_align_str[nx] = '\0';
	halignED->setText(h_align_str);
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


void QMathMatrixDialog::slotClose()
{
	close();
}

} // namespace frontend
} // namespace lyx

#include "QMathMatrixDialog_moc.cpp"
