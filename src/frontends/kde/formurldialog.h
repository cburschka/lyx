/*
 * formurldialog.h
 * (C) 2000 John Levon
 * moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORMURLDIALOG_H
#define FORMURLDIALOG_H

#include <config.h>
#include <gettext.h>

// to connect apply() and hide()
#include "FormUrl.h"

#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class FormUrlDialog : public QDialog  {
   Q_OBJECT
public:
	FormUrlDialog(FormUrl *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~FormUrlDialog();

	// widgets
 
	QLabel *labelurl;
	QLabel *labelurlname;
	QLineEdit *url;
	QLineEdit *urlname;
	QCheckBox *htmlurl;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;

protected:
	void closeEvent(QCloseEvent *e);
 
private:
	FormUrl *form_;

	// layouts
 
	QHBoxLayout *topLayout;
	QVBoxLayout *layout;
	QHBoxLayout *urlLayout;
	QBoxLayout *urlnameLayout;
	QBoxLayout *htmlurlLayout;
	QHBoxLayout *buttonLayout;

private slots:
	/// gate to FormUrl::apply
	void apply_gate(void) {
		form_->apply();
		form_->close();
		hide();
	}

	/// gate to FormUrl::close
	void close_gate(void) {
		form_->close();
		hide();
	}
};

#endif
