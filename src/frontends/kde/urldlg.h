/*
 * urldlg.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef URLDLG_H
#define URLDLG_H

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

class UrlDialog : public QDialog  {
   Q_OBJECT
public:
	UrlDialog(FormUrl *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~UrlDialog();

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
	/// adaptor to FormUrl::apply
	void apply_adaptor(void) {
		form_->apply();
		form_->close();
		hide();
	}

	/// adaptor to FormUrl::close
	void close_adaptor(void) {
		form_->close();
		hide();
	}
};

#endif
