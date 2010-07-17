// -*- C++ -*-
/**
 * \file GuiEncryptionDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiEncryptionDialog.h"

#include "qt_i18n.h"
#include "support/qstring_helpers.h"

#include <QHBoxLayout>
#include <QGridLayout>


namespace lyx {
namespace frontend {


GuiEncryptionDialog::GuiEncryptionDialog(QWidget *parent) : QDialog(parent)
{
	pwd_label_ = new QLabel(qt_("Password:"));
	pwd_edit_ = new QLineEdit;
	pwd_edit_->setEchoMode(QLineEdit::Password);
	pwd_label_->setBuddy(pwd_edit_);

	button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(pwd_label_);
	hLayout->addWidget(pwd_edit_);

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addLayout(hLayout, 0, 0);
	mainLayout->addWidget(button_box_, 1, 0);
	setLayout(mainLayout);
}

QString GuiEncryptionDialog::password() const
{
	return pwd_edit_->text();
}

void GuiEncryptionDialog::clearPassword()
{
	pwd_edit_->setText("");
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiEncryptionDialog.cpp"
