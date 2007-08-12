/**
 * \file QURLDialog.cpp
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

	connect(okPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));
	connect(urlED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(hyperlinkCB, SIGNAL(clicked()),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));

	setFocusProxy(urlED);
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
