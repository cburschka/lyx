/**
 * \file QTocDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <debug.h>
#include "QTocDialog.h"
#include "Dialogs.h"
#include "QToc.h"

#include <qlistview.h>

QTocDialog::QTocDialog(QToc * form, QWidget * parent, const char * name, bool modal, WFlags fl)
	: QTocDialogBase(parent, name, modal, fl),
	form_(form)
{
}
 

QTocDialog::~QTocDialog()
{
}
 

void QTocDialog::activate_adaptor(int index)
{
	switch (index) {
	case 0:
		//form_->set_type(Buffer::TOC_TOC);
		break;
	case 1:
		//form_->set_type(Buffer::TOC_LOF);
		break;
	case 2:
		//form_->set_type(Buffer::TOC_LOT);
		break;
	case 3:
		//form_->set_type(Buffer::TOC_LOA);
		break;
	default:
		lyxerr[Debug::GUI] << "Unknown TOC combo selection." << std::endl;
		break;
	}
}
 
void QTocDialog::close_adaptor()
{
	form_->close();
	hide();
}


void QTocDialog::depth_adaptor(int depth)
{
	//form_->set_depth(depth);
}


void QTocDialog::select_adaptor(QListViewItem * item)
{
	//form_->select(item->text(0));
}


void QTocDialog::update_adaptor()
{
	form_->update();
}

 
void QTocDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}
