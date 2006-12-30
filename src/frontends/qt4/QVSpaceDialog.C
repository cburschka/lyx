/**
 * \file QVSpaceDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QVSpaceDialog.h"
#include "QVSpace.h"
//Added by qt3to4:
#include <QCloseEvent>

#include "lengthcombo.h"
#include "validators.h"
#include "qt_helpers.h"

#include <QLineEdit>
#include <QPushButton>
#include <QValidator>


namespace lyx {
namespace frontend {


QVSpaceDialog::QVSpaceDialog(QVSpace * form)
	: form_(form)
{
	setupUi(this);

	Q_CONNECT_1(QPushButton, okPB, clicked, bool,
				QVSpace, form_, slotOK, void);
	Q_CONNECT_1(QPushButton, applyPB, clicked, bool,
				QVSpace, form_, slotApply, void);
	Q_CONNECT_1(QPushButton, closePB, clicked, bool,
				QVSpace, form_, slotClose, void);

    Q_CONNECT_1(QComboBox, spacingCO, highlighted, const QString&, 
				QVSpaceDialog, this, change_adaptor, void);
    Q_CONNECT_1(QLineEdit, valueLE, textChanged, const QString&,
				QVSpaceDialog, this, change_adaptor, void);
    Q_CONNECT_1(QComboBox, spacingCO, activated, int,
				QVSpaceDialog, this, enableCustom, int);
    Q_CONNECT_1(QCheckBox, keepCB, clicked, bool,
				QVSpaceDialog, this, change_adaptor, void);
    Q_CONNECT_1(LengthCombo, unitCO, selectionChanged, lyx::LyXLength::UNIT,
				QVSpaceDialog, this, change_adaptor, void);

	valueLE->setValidator(unsignedLengthValidator(valueLE));
}


void QVSpaceDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QVSpaceDialog::change_adaptor()
{
	form_->changed();
}


void QVSpaceDialog::enableCustom(int selection)
{
	bool const enable = selection == 5;
	valueLE->setEnabled(enable);
	unitCO->setEnabled(enable);
}

} // namespace frontend
} // namespace lyx

#include "QVSpaceDialog_moc.cpp"
