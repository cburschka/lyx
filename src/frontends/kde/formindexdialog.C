/*
 * formindexdialog.C
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

#include "formindexdialog.h"

FormIndexDialog::FormIndexDialog(FormIndex *form, QWidget *parent, const char *name, bool, WFlags)
	: QDialog(parent,name,false), form_(form)
{
	setCaption(name);

	// widgets

	index = new QLineEdit(this);
	index->setMinimumSize(index->sizeHint());

	labelindex = new QLabel(this);
	labelindex->setText(_("Keyword :"));
	labelindex->setMargin(5);
	labelindex->setMinimumSize(labelindex->sizeHint());
	labelindex->setMaximumSize(labelindex->sizeHint());

	buttonOk = new QPushButton(this);
	buttonOk->setMinimumSize(buttonOk->sizeHint());
	buttonOk->setMaximumSize(buttonOk->sizeHint());
	buttonOk->setText(_("&OK"));
	buttonOk->setDefault(true);

	buttonCancel = new QPushButton(this);
	buttonCancel->setMinimumSize(buttonCancel->sizeHint());
	buttonCancel->setMaximumSize(buttonCancel->sizeHint());
	buttonCancel->setText(_("&Cancel"));

	// tooltips

	QToolTip::add(labelindex,_("Index entry"));
	QToolTip::add(index,_("Index entry"));

	// layouts

	topLayout = new QHBoxLayout(this,10);

	layout = new QVBoxLayout();
	topLayout->addLayout(layout);
	layout->addSpacing(10);

	indexLayout = new QHBoxLayout();
	layout->addLayout(indexLayout);
	indexLayout->addWidget(labelindex, 0);
	indexLayout->addWidget(index, 1);

	layout->addStretch(1);

	buttonLayout = new QHBoxLayout();

	layout->addLayout(buttonLayout);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(buttonOk, 1);
	buttonLayout->addStretch(2);
	buttonLayout->addWidget(buttonCancel, 1);
	buttonLayout->addStretch(1);
 
	// connections

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(apply_adaptor()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close_adaptor()));
}

void FormIndexDialog::closeEvent(QCloseEvent *e)
{
	form_->close();
	e->accept();
}

FormIndexDialog::~FormIndexDialog()
{
}
