/**
 * \file QMathDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QMATHDIALOG_H
#define QMATHDIALOG_H

#include <config.h>

#include "LString.h"
 
#include "ui/QMathDialogBase.h"

class QMath;
class IconPalette;

class QMathDialog : public QMathDialogBase
{
	Q_OBJECT

public:
	QMathDialog(QMath * form);

public slots:
	virtual void delimiterClicked();
	virtual void expandClicked();
	virtual void fracClicked();
	virtual void functionSelected(const QString &);
	virtual void matrixClicked();
	virtual void spaceClicked();
	virtual void sqrtClicked();
	virtual void styleClicked();
	virtual void subscriptClicked();
	virtual void superscriptClicked();
	void symbol_clicked(string str);
 
protected:
	//needed ? virtual void closeEvent(QCloseEvent * e);

private:
	/// make a symbol panel
	IconPalette * makePanel(QWidget * parent, char const ** entries);

	/// add a symbol panel
	void addPanel(char const ** entries);
 
	/// owning form
	QMath * form_;
};

#endif // QMATHDIALOG_H
