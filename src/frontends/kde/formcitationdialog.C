/*
 * formcitationdialog.C
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

#include "formcitationdialog.h"

FormCitationDialog::FormCitationDialog(FormCitation *form, QWidget *parent, const char *name, bool, WFlags)
	: QDialog(parent,name,false), form_(form)
{
	setCaption(name);
	setMinimumWidth(500);

	// widgets

	labelchosen = new QLabel(this);
	labelchosen->setText(_("Selected keys"));
	labelchosen->setMinimumSize(labelchosen->sizeHint());
	labelchosen->setMaximumSize(labelchosen->sizeHint());

	chosen = new QListBox(this);

	labelkeys = new QLabel(this);
	labelkeys->setText(_("Available keys"));
	labelkeys->setMinimumSize(labelkeys->sizeHint());
	labelkeys->setMaximumSize(labelkeys->sizeHint());

	keys = new QListBox(this);

	labelentry = new QLabel(this);
	labelentry->setText(_("Reference entry"));
	labelentry->setMinimumSize(labelentry->sizeHint());
	labelentry->setMaximumSize(labelentry->sizeHint());

	after = new QLineEdit(this);
	after->setMinimumSize(after->sizeHint());

	entry = new QMultiLineEdit(this);
	entry->setReadOnly(true);
	entry->setFixedVisibleLines(2); 

	labelafter = new QLabel(this);
	labelafter->setText(_("Text after"));
	labelafter->setMargin(5);
	labelafter->setMinimumSize(labelafter->sizeHint());
	labelafter->setMaximumSize(labelafter->sizeHint());

	/* FIXME: icons */
	add = new QPushButton(this); 
	add->setText(_("&Add"));
	add->setMinimumSize(add->sizeHint());
	add->setMaximumSize(add->sizeHint());
 
	up = new QPushButton(this); 
	up->setText(_("&Up"));
	up->setMinimumSize(up->sizeHint());
	up->setMaximumSize(up->sizeHint());
 
	down = new QPushButton(this); 
	down->setText(_("&Down"));
	down->setMinimumSize(down->sizeHint());
	down->setMaximumSize(down->sizeHint());
 
	remove = new QPushButton(this); 
	remove->setText(_("&Remove"));
	remove->setMinimumSize(remove->sizeHint());
	remove->setMaximumSize(remove->sizeHint());

	buttonOk = new QPushButton(this);
	buttonOk->setText(_("&OK"));
	buttonOk->setDefault(true);
	buttonOk->setMinimumSize(buttonOk->sizeHint());
	buttonOk->setMaximumSize(buttonOk->sizeHint());

	buttonCancel = new QPushButton(this);
	buttonCancel->setText(_("&Cancel"));
	buttonCancel->setMinimumSize(buttonCancel->sizeHint());
	buttonCancel->setMaximumSize(buttonCancel->sizeHint());

	// tooltips

	QToolTip::add(chosen,_("Keys currently selected"));
	QToolTip::add(keys,_("Reference keys available"));
	QToolTip::add(entry,_("Reference entry text"));
	QToolTip::add(after,_("Text to place after citation"));

	// layouts

	topLayout = new QHBoxLayout(this,10);

	layout = new QVBoxLayout();
	topLayout->addLayout(layout);
	layout->addSpacing(10);

	browserLayout = new QHBoxLayout();
	layout->addLayout(browserLayout,1);

	chosenLayout = new QVBoxLayout();
	browserLayout->addLayout(chosenLayout,1); 
	iconLayout = new QVBoxLayout();
	browserLayout->addLayout(iconLayout,0);
	keysLayout = new QVBoxLayout();
	browserLayout->addLayout(keysLayout,1);
 
	chosenLayout->addWidget(labelchosen, 0, AlignLeft);
	chosenLayout->addWidget(chosen, 1);
 
	iconLayout->addStretch(1);
	iconLayout->addWidget(add,1);
	iconLayout->addStretch(1);
	iconLayout->addWidget(up,1);
	iconLayout->addStretch(1);
	iconLayout->addWidget(down,1);
	iconLayout->addStretch(1);
	iconLayout->addWidget(remove,1);
	iconLayout->addStretch(1);
 
	keysLayout->addWidget(labelkeys, 0, AlignLeft);
	keysLayout->addWidget(keys, 1);

	entryLayout = new QVBoxLayout();
	layout->addLayout(entryLayout);
 
	entryLayout->addWidget(labelentry, 0, AlignLeft);
	entryLayout->addWidget(entry, 0);

	afterLayout = new QHBoxLayout();
	layout->addLayout(afterLayout);

	afterLayout->addWidget(labelafter, 0, AlignLeft);
	afterLayout->addWidget(after, 1);
 
	buttonLayout = new QHBoxLayout();
	layout->addLayout(buttonLayout);
 
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(buttonOk, 1);
	buttonLayout->addStretch(2);
	buttonLayout->addWidget(buttonCancel, 1);
	buttonLayout->addStretch(1);
 
	// connections

	connect(keys, SIGNAL(selected(const char *)), this, SLOT(select_key_adaptor(const char *)));
	connect(keys, SIGNAL(highlighted(const char *)), this, SLOT(highlight_key_adaptor(const char *)));
	connect(chosen, SIGNAL(highlighted(const char *)), this, SLOT(highlight_chosen_adaptor(const char *)));
	connect(add, SIGNAL(clicked()), this, SLOT(add_adaptor()));
	connect(up, SIGNAL(clicked()), this, SLOT(up_adaptor()));
	connect(down, SIGNAL(clicked()), this, SLOT(down_adaptor()));	
	connect(remove, SIGNAL(clicked()), this, SLOT(remove_adaptor()));
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(apply_adaptor()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close_adaptor()));
}

void FormCitationDialog::closeEvent(QCloseEvent *e)
{
	form_->close();
	e->accept();
}

FormCitationDialog::~FormCitationDialog()
{
}
