/**
 * \file QViewSourceDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QViewSourceDialog.h"
#include "QViewSource.h"

#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

QViewSourceDialog::QViewSourceDialog(QViewSource * form)
	: form_(form)
{
	setupUi(this);

	Q_CONNECT_1(QPushButton, closePB, clicked, bool,
				QViewSource, form, slotClose, void);
	Q_CONNECT_1(QCheckBox, viewFullSourceCB, toggled, bool,
				QViewSourceDialog, this, slotUpdate, void);
	Q_CONNECT_1(QCheckBox, autoUpdateCB, toggled, bool,
				QPushButton, updatePB, setDisabled, bool);
	Q_CONNECT_1(QPushButton, updatePB, clicked, bool,
				QViewSourceDialog, this, slotUpdate, void);
}


void QViewSourceDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QViewSourceDialog::slotUpdate()
{
	form_->update_source();
}

} // namespace frontend
} // namespace lyx

#include "QViewSourceDialog_moc.cpp"
