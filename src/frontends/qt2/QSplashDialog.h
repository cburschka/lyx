/**
 * \file QSplashDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef FORMSPLASHBASE_H
#define FORMSPLASHBASE_H

class QSplashDialog : public QFrame
{
public:
	QSplashDialog(QWidget * parent,  const char * name,
		WFlags fl = QWidget::WStyle_NoBorder | QWidget::WStyle_Customize, 
		bool modal = false);
   
	~QSplashDialog();
   
protected:
	virtual void mousePressEvent(QMouseEvent *);
};

#endif // QSPLASHDIALOG_H
