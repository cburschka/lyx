/**
 * \file QURLDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QURLDialog.h"
#include "UrlView.h"

#include <QLineEdit>
#include <QPushButton>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QURLDialog::QURLDialog(UrlView * form)
	: form_(form)
{
	setupUi(this);

	Q_CONNECT_1(QPushButton, okPB, clicked, bool,
				UrlView, form_, slotOK, void);
	Q_CONNECT_1(QPushButton, closePB, clicked, bool,
				UrlView, form_, slotClose, void);

    Q_CONNECT_1(QLineEdit, urlED, textChanged, const QString&, 
				QURLDialog, this, changed_adaptor, void);
    Q_CONNECT_1(QCheckBox, hyperlinkCB, clicked, bool, 
				QURLDialog, this, changed_adaptor, void);
    Q_CONNECT_1(QLineEdit, nameED, textChanged, const QString&, 
				QURLDialog, this, changed_adaptor, void);
}


QURLDialog::~QURLDialog()
{
}


void QURLDialog::show()
{
	QDialog::show();
	urlED->setFocus();
}


void QURLDialog::changed_adaptor()
{
	form_->changed();
}


void QURLDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QURLDialog_moc.cpp"
