/**
 * \file QWrapDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QWrapDialog.h"
#include "QWrap.h"

#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {


QWrapDialog::QWrapDialog(QWrap * form)
	: form_(form)
{
	setupUi(this);

	Q_CONNECT_1(QPushButton, restorePB, clicked, bool,
				QWrap, form, slotRestore, void);
	Q_CONNECT_1(QPushButton, okPB, clicked, bool,
				QWrap, form, slotOK, void);
	Q_CONNECT_1(QPushButton, applyPB, clicked, bool,
				QWrap, form, slotApply, void);
	Q_CONNECT_1(QPushButton, closePB, clicked, bool,
				QWrap, form, slotClose, void);

    Q_CONNECT_1(QLineEdit, widthED, textChanged, const QString&, 
				QWrapDialog, this, change_adaptor, void);
    Q_CONNECT_1(LengthCombo, unitsLC, selectionChanged, LyXLength::UNIT, 
				QWrapDialog, this, change_adaptor, void);
    Q_CONNECT_1(QComboBox, valignCO, highlighted, const QString&, 
				QWrapDialog, this, change_adaptor, void);
}


void QWrapDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QWrapDialog::change_adaptor()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx

#include "QWrapDialog_moc.cpp"
