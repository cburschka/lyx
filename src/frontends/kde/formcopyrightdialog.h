/***************************************************************************
                          formcopyrightdialog.h  -  description
                             -------------------
    begin                : Fri Feb 4 2000
    copyright            : (C) 2000 by Jürgen Vigna
    email                : jug@sad.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORMCOPYRIGHTDIALOG_H
#define FORMCOPYRIGHTDIALOG_H

//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)

#include <qdialog.h>

/**
  *@author Jürgen Vigna
  */

class FormCopyrightDialog : public QDialog  {
   Q_OBJECT
public: 
	FormCopyrightDialog(QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~FormCopyrightDialog();

protected: 
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QLabel *labelWho;
	QLabel *labelGNU;
	QLabel *labelLyX;
	QPushButton *buttonOk;
	//Generated area. DO NOT EDIT!!!(end)

private: 
};

#endif
