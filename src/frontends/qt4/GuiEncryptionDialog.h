// -*- C++ -*-
/**
 * \file GuiEncryptionDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_ENCRYPTION_DIALOG_H
#define GUI_ENCRYPTION_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>


namespace lyx {
namespace frontend {


class GuiEncryptionDialog : public QDialog
{
	Q_OBJECT

public:
	GuiEncryptionDialog(QWidget *parent = 0);
	 
	void clearPassword();
	QString password() const;

private:
	QLabel * pwd_label_;
	QLineEdit * pwd_edit_;
	QDialogButtonBox * button_box_;
};


} // namespace frontend
} // namespace lyx

#endif

