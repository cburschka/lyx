/**
 * \file refdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef REFDLG_H
#define REFDLG_H

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

class RefDialog : public QDialog {
	Q_OBJECT
public:
	RefDialog(FormRef * form, QWidget * parent=0, char const * name=0,
			    bool modal=false, WFlags f=0);
	~RefDialog();

	// widgets
 
	QListBox * refs;
	QLabel * labelrefs;
	QLineEdit * refname;
	QLineEdit * reference;
	QLabel * labelrefname;
	QLabel * labelreference;
	QCheckBox * sort;
	QComboBox * type; 
	QLabel * labeltype; 
	QPushButton * buttonGoto;
	QPushButton * buttonUpdate;
	QPushButton * buttonOk;
	QPushButton * buttonCancel;
 
protected:
	void closeEvent(QCloseEvent * e);
 
private:
	FormRef * form_;

	// layouts
 
	QHBoxLayout * topLayout;
	QVBoxLayout * layout;
	QHBoxLayout * upperLayout;
	QVBoxLayout * browserLayout;
	QVBoxLayout * rightLayout;
	QHBoxLayout * nameLayout;
	QHBoxLayout * referenceLayout; 
	QHBoxLayout * buttonLayout;

private slots:
	/// adaptor to FormRef::select
	void select_adaptor(const char * sel) {
		form_->select(sel);
	}

	/// adaptor to FormRef::highlight
	void highlight_adaptor(const char * sel) {
		form_->highlight(sel); 
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
		form_->do_ref_update();
	}
 
	/// adaptor to FormRef::apply
	void apply_adaptor(void) {
		form_->ApplyButton(); 
	}

	/// adaptor to FormRef::close
	void close_adaptor(void) {
		form_->CancelButton();
	}
};

#endif // REFDLG_H
