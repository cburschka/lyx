// -*- C++ -*-
/**
 * \file QCharacterDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCHARACTERDIALOG_H
#define QCHARACTERDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "QCharacter.h"
#include "ui/QCharacterDialogBase.h"

#include <qevent.h>
#include <qcheckbox.h>
#include <qcombobox.h>

class LyXFont;

class QCharacterDialog : public QCharacterDialogBase
{ Q_OBJECT

public:
	QCharacterDialog(QCharacter * form);

protected:
	void closeEvent(QCloseEvent * e);

private:
	QCharacter * form_;

protected slots:
	void change_adaptor();
};

#endif // QCHARACTERDIALOG_H
