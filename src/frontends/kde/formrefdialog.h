/*
 * formrefdialog.h
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

#ifndef FORMREFDIALOG_H
#define FORMREFDIALOG_H

#include <config.h>
#include <gettext.h>

#include "FormRef.h"

#include <qdialog.h>
#include <qlayout.h>
#include <qcombobox.h> 
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class FormRefDialog : public QDialog  {
   Q_OBJECT
public:
	FormRefDialog(FormRef *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~FormRefDialog();

	// widgets
 
	QListBox *refs;
	QLabel *labelrefs;
	QLineEdit *refname;
	QLineEdit *reference;
	QLabel *labelrefname;
	QLabel *labelreference;
	QCheckBox *sort;
	QComboBox *type; 
	QLabel *labeltype; 
	QPushButton *buttonGoto;
	QPushButton *buttonUpdate;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;
 
protected:
	void closeEvent(QCloseEvent *e);
 
private:
	FormRef *form_;

	// layouts
 
	QHBoxLayout *topLayout;
	QVBoxLayout *layout;
	QHBoxLayout *upperLayout;
	QVBoxLayout *browserLayout;
	QVBoxLayout *rightLayout;
	QHBoxLayout *nameLayout;
	QHBoxLayout *referenceLayout; 
	QHBoxLayout *buttonLayout;

private slots:
	/// adaptor to FormRef::select
	void highlight_adaptor(const char *sel) {
		form_->select(sel); 
	}

	/// adaptor to FormRef::set_sort
	void sort_adaptor(bool sort) {
		form_->set_sort(sort);
	} 

	/// adaptor to FormRef::gotoRef
	void goto_adaptor(void) {
		form_->goto_ref();
	}
 
	/// adaptor to FormRef::update
	void update_adaptor(void) {
		form_->update();
	}
 
	/// adaptor to FormRef::apply
	void apply_adaptor(void) {
		form_->apply();
		form_->close();
		hide();
	}

	/// adaptor to FormRef::close
	void close_adaptor(void) {
		form_->close();
		hide();
	}
};

#endif
