/**
 * \file QCharacterDialog.h
 * Copyright 2001 the LyX Team
 * see the file COPYING
 *
 * \author Edwin Leuven
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QCHARACTERDIALOG_H
#define QCHARACTERDIALOG_H

#include <config.h>

#include "ui/QCharacterDialogBase.h"
#include "QCharacter.h"

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
