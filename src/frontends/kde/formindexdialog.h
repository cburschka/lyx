/*
 * formindexdialog.h
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

#ifndef FORMINDEXDIALOG_H
#define FORMINDEXDIALOG_H

#include <config.h>
#include <gettext.h>

// to connect apply() and hide()
#include "FormIndex.h"

#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class FormIndexDialog : public QDialog  {
   Q_OBJECT
public:
	FormIndexDialog(FormIndex *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~FormIndexDialog();

	// widgets
 
	QLabel *labelindex;
	QLineEdit *index;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;
 

protected:
	void closeEvent(QCloseEvent *e);
 
private:
	FormIndex *form_;

	// layouts
 
	QHBoxLayout *topLayout;
	QVBoxLayout *layout;
	QHBoxLayout *indexLayout;
	QHBoxLayout *buttonLayout;

private slots:
	/// adaptor to FormIndex::apply
	void apply_adaptor(void) {
		form_->apply();
		form_->close();
		hide();
	}

	/// adaptor to FormIndex::close
	void close_adaptor(void) {
		form_->close();
		hide();
	}
};

#endif
