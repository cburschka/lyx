/*
 * tocdlg.h
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

#ifndef TOCDLG_H
#define TOCDLG_H

#include <config.h>
#include <gettext.h>

#include <debug.h>
#include <qlabel.h> 
#include <qdialog.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qslider.h>

#include "FormToc.h"

class TocDialog : public QWidget {
   Q_OBJECT
public:
	TocDialog(FormToc *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~TocDialog();

	// widgets

	QComboBox *menu;
	QListView *tree;
	QPushButton *buttonUpdate;
	QPushButton *buttonClose;
	QSlider *depth;
	QLabel *depthlabel; 

protected:
	void closeEvent(QCloseEvent *e);

private:
	FormToc *form_;

	// layouts 

	QHBoxLayout *topLayout;
	QVBoxLayout *layout;
	QHBoxLayout *buttonLayout;

private slots:
	/// adaptor to FormToc::select
	void select_adaptor(QListViewItem *item) {
		form_->select(item->text(0));
	}

	/// adaptor to FormToc::set_depth
	void depth_adaptor(int depth) {
		form_->set_depth(depth);
	}

	/// adaptor to FormToc::update
	void update_adaptor(void) {
		form_->update();
	}

	// adaptors to FormToc::set_type
	void activate_adaptor(int index) {
		switch (index) {
			case 0:
				form_->set_type(Buffer::TOC_TOC);
				break;
			case 1:
				form_->set_type(Buffer::TOC_LOF);
				break;
			case 2:
				form_->set_type(Buffer::TOC_LOT);
				break;
			case 3:
				form_->set_type(Buffer::TOC_LOA);
				break;
			default:
				lyxerr[Debug::GUI] << "Unknown TOC combo selection." << std::endl;
				break;
		}
	}

	/// adaptor to FormToc::close
	void close_adaptor(void) {
		form_->close();
		hide();
	}
};

#endif
