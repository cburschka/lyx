/**
 * \file QCommandEdit.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
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
