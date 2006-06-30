// -*- C++ -*-
/**
 * \file QCharacterDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCHARACTERDIALOG_H
#define QCHARACTERDIALOG_H

#include "ui/QCharacterUi.h"
#include <QCloseEvent>
#include <QDialog>
#include "QCharacter.h"

class LyXFont;



namespace lyx {
namespace frontend {

class QCharacterDialog : public QDialog, public Ui::QCharacterUi {
	Q_OBJECT
public:
	QCharacterDialog(QCharacter * form);
protected:
	void closeEvent(QCloseEvent * e);
private:
	QCharacter * form_;
protected Q_SLOTS:
	void change_adaptor();
};

} // namespace frontend
} // namespace lyx

#endif // QCHARACTERDIALOG_H
