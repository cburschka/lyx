/**
 * \file urldlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

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

class UrlDialog : public QDialog {
   Q_OBJECT
public:
	UrlDialog(FormUrl * form, QWidget * parent=0, char const * name=0,
			    bool modal=false, WFlags f=0);
	~UrlDialog();

	// widgets
 
	QLabel * labelurl;
	QLabel * labelurlname;
	QLineEdit * url;
	QLineEdit * urlname;
	QCheckBox * htmlurl;
	QPushButton * buttonOk;
	QPushButton * buttonCancel;

protected slots:
	virtual void closeEvent(QCloseEvent * e);
 
private:
	FormUrl * form_;

	// layouts
 
	QHBoxLayout * topLayout;
	QVBoxLayout * layout;
	QHBoxLayout * urlLayout;
	QBoxLayout * urlnameLayout;
	QBoxLayout * htmlurlLayout;
	QHBoxLayout * buttonLayout;

private slots:
	/// adaptor to FormUrl::apply
	void apply_adaptor(void) {
		form_->OKButton();
	}

	/// adaptor to FormUrl::close
	void close_adaptor(void) {
		form_->CancelButton(); 
	}
};

#endif // URLDLG_H
