/*
 * formcitationdialog.h
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

#ifndef FORMCITATIONDIALOG_H
#define FORMCITATIONDIALOG_H

#include <config.h>
#include <gettext.h>

// to connect apply() and hide()
#include "FormCitation.h"

#include <qdialog.h>
#include <qlistbox.h> 
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <qmultilinedit.h> 
#include <qpushbutton.h>

class FormCitationDialog : public QDialog  {
   Q_OBJECT
public:
	FormCitationDialog(FormCitation *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~FormCitationDialog();

	// widgets
 
	QLabel *labelchosen;
	QListBox *chosen;
	QLabel *labelkeys;
	QListBox *keys;
	QLabel *labelentry;
	QMultiLineEdit *entry;
	QLabel *labelafter;
	QLineEdit *after;
	QPushButton *add;
	QPushButton *up;
	QPushButton *down;
	QPushButton *remove;
	QPushButton *buttonOk;
	QPushButton *buttonCancel;
 
protected:
	void closeEvent(QCloseEvent *e);
 
private:
	FormCitation *form_;

	// layouts
 
	QHBoxLayout *topLayout;
	QVBoxLayout *layout;
	QHBoxLayout *browserLayout;
	QVBoxLayout *chosenLayout;
	QVBoxLayout *iconLayout;
	QVBoxLayout *keysLayout; 
	QVBoxLayout *entryLayout; 
	QHBoxLayout *afterLayout;
	QHBoxLayout *buttonLayout;

private slots:
	void apply_adaptor(void) {
		form_->apply();
		form_->close();
		hide();
	}

	void close_adaptor(void) {
		form_->close();
		hide();
	}

	void add_adaptor(void) {
		form_->add();
	}

	void up_adaptor(void) {
		form_->up();
	}

	void down_adaptor(void) {
		form_->down();
	}

	void remove_adaptor(void) {
		form_->remove();
	}

	void select_key_adaptor(const char *key) {
		form_->select_key(key);
	}
 
	void highlight_key_adaptor(const char *key) {
		form_->highlight_key(key);
	}

	void highlight_chosen_adaptor(const char *key) {
		form_->highlight_chosen(key);
	}
 
};

#endif
