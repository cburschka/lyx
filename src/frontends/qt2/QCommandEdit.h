// -*- C++ -*-
/**
 * \file QCommandEdit.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCOMMANDEDIT_H
#define QCOMMANDEDIT_H

#include <qlineedit.h>

class QCommandEdit : public QLineEdit {
	Q_OBJECT
public:

	QCommandEdit(QWidget * parent);

signals:
	/// cancel
	void escapePressed();

	/// up history
	void upPressed();

	/// down history
	void downPressed();

	/// complete
	void rightPressed();

protected:
	virtual void keyPressEvent(QKeyEvent * e);
};

#endif // QCOMMANDEDIT_H
