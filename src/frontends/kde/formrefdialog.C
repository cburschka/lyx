/*
 * formrefdialog.C
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

#include "formrefdialog.h"

FormRefDialog::FormRefDialog(FormRef *form, QWidget *parent, const char *name, bool, WFlags)
	: QDialog(parent,name,false), form_(form)
{
	setCaption(name);

	// widgets

	refs = new QListBox(this);
	refs->setMinimumSize(200,200);
 
	labelrefs = new QLabel(this);
	labelrefs->setText(_("Available References"));
	labelrefs->setMargin(5);
	labelrefs->setMinimumSize(labelrefs->sizeHint());
	labelrefs->setMaximumSize(labelrefs->sizeHint());

	refname = new QLineEdit(this);
	refname->setMinimumSize(refname->sizeHint());
	// FIXME: should user be able to edit this ? what's it for ? - jbl 
	refname->setFocusPolicy(QWidget::NoFocus);
 
	reference = new QLineEdit(this); 
	reference->setMinimumSize(reference->sizeHint());
	reference->setFocusPolicy(QWidget::NoFocus);

	labelrefname = new QLabel(this);
	labelrefname->setText(_("Name :"));
	labelrefname->setMargin(5);
	labelrefname->setMinimumSize(labelrefname->sizeHint());
	labelrefname->setMaximumSize(labelrefname->sizeHint());

	labelreference = new QLabel(this);
	labelreference->setText(_("Reference :"));
	labelreference->setMargin(5);
	labelreference->setMinimumSize(labelreference->sizeHint());
	labelreference->setMaximumSize(labelreference->sizeHint());

	sort = new QCheckBox(this); 
	sort->setText(_("Sort"));
	sort->setMinimumSize(sort->sizeHint());
	sort->setMaximumSize(sort->sizeHint());
 
	type = new QComboBox(this);
	type->insertItem(_("Reference"));
	type->insertItem(_("Page number"));
	type->insertItem(_("Ref on page xxx"));
	type->insertItem(_("on page xxx"));
	type->insertItem(_("Pretty reference"));
	type->setMinimumSize(type->sizeHint());
	 
	labeltype = new QLabel(this); 
	labeltype->setText(_("Reference Type"));
	labeltype->setMargin(5);
	labeltype->setMinimumSize(labeltype->sizeHint());
	labeltype->setMaximumSize(labeltype->sizeHint());
 
	buttonGoto = new QPushButton(this);
	buttonGoto->setText(_("&Goto reference"));
	buttonGoto->setMinimumSize(buttonGoto->sizeHint());
	buttonGoto->setMaximumSize(buttonGoto->sizeHint());

	buttonUpdate = new QPushButton(this);
	buttonUpdate->setText(_("&Update"));
	buttonUpdate->setMinimumSize(buttonUpdate->sizeHint());
	buttonUpdate->setMaximumSize(buttonUpdate->sizeHint());

	buttonOk = new QPushButton(this);
	buttonOk->setText(_("&OK"));
	buttonOk->setMinimumSize(buttonOk->sizeHint());
	buttonOk->setMaximumSize(buttonOk->sizeHint());
	buttonOk->setDefault(true);

	buttonCancel = new QPushButton(this);
	buttonCancel->setText(_("&Cancel"));
	buttonCancel->setMinimumSize(buttonCancel->sizeHint());
	buttonCancel->setMaximumSize(buttonCancel->sizeHint());

	// tooltips

	QToolTip::add(type,_("Reference as it appears in output"));
	QToolTip::add(sort,_("Sort references in alphabetical order ?"));

	// layouts

	topLayout = new QHBoxLayout(this,10);

	layout = new QVBoxLayout();
	topLayout->addLayout(layout);
	layout->addSpacing(10);

	upperLayout = new QHBoxLayout();
	layout->addLayout(upperLayout, 1);

	browserLayout = new QVBoxLayout();
	upperLayout->addLayout(browserLayout, 1);

	browserLayout->addWidget(labelrefs, 0);
	browserLayout->addWidget(refs, 1);

	rightLayout = new QVBoxLayout();
	upperLayout->addLayout(rightLayout, 1);

	nameLayout = new QHBoxLayout();
	rightLayout->addLayout(nameLayout, 0);

	nameLayout->addWidget(labelrefname, 0);
	nameLayout->addWidget(refname, 1);

	rightLayout->addStretch(1);
 
	referenceLayout = new QHBoxLayout();
	rightLayout->addLayout(referenceLayout, 0);

	referenceLayout->addWidget(labelreference, 0);
	referenceLayout->addWidget(reference, 1);
 
	rightLayout->addStretch(1);
 
	rightLayout->addWidget(labeltype, 0);
	rightLayout->addWidget(type, 0);
 
	rightLayout->addStretch(1);
 
	rightLayout->addWidget(buttonGoto, 1);

	rightLayout->addStretch(1);
 
	buttonLayout = new QHBoxLayout(); 
	layout->addLayout(buttonLayout);
	buttonLayout->addWidget(sort, 1);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(buttonUpdate, 1);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(buttonOk, 1);
	buttonLayout->addStretch(2);
	buttonLayout->addWidget(buttonCancel, 1);
	buttonLayout->addStretch(1);
 
	// connections

	connect(refs, SIGNAL(highlighted(const char *)), this, SLOT(highlight_adaptor(const char *))); 
	connect(sort, SIGNAL(toggled(bool)), this, SLOT(sort_adaptor(bool))); 
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(apply_adaptor()));
	connect(buttonUpdate, SIGNAL(clicked()), this, SLOT(update_adaptor()));
	connect(buttonGoto, SIGNAL(clicked()), this, SLOT(goto_adaptor())); 
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close_adaptor()));
}

void FormRefDialog::closeEvent(QCloseEvent *e)
{
	form_->close();
	e->accept();
}

FormRefDialog::~FormRefDialog()
{
}
