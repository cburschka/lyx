/**
 * \file citationdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "dlg/helpers.h"
#include "citationdlg.h"

#ifdef CXX_WORKING_NAMESPACES
using kde_helpers::setSizeHint;
#endif

CitationDialog::CitationDialog(FormCitation *form, QWidget *parent, char const * name, bool, WFlags)
	: QDialog(parent,name,0), form_(form)
{
	setCaption(name);
	setMinimumWidth(500);

	// widgets

	labelchosen = new QLabel(this);
	labelchosen->setText(_("Selected keys"));
	setSizeHint(labelchosen);
	labelchosen->setMaximumSize(labelchosen->sizeHint());

	chosen = new QListBox(this);

	labelkeys = new QLabel(this);
	labelkeys->setText(_("Available keys"));
	setSizeHint(labelkeys);
	labelkeys->setMaximumSize(labelkeys->sizeHint());

	keys = new QListBox(this);

	labelentry = new QLabel(this);
	labelentry->setText(_("Reference entry"));
	setSizeHint(labelentry);
	labelentry->setMaximumSize(labelentry->sizeHint());

	after = new QLineEdit(this);
	setSizeHint(after);

	entry = new QMultiLineEdit(this);
	entry->setReadOnly(true);
	entry->setFixedVisibleLines(2); 

	labelafter = new QLabel(this);
	labelafter->setText(_("Text after"));
	labelafter->setMargin(5);
	setSizeHint(labelafter);
	labelafter->setMaximumSize(labelafter->sizeHint());

	add = new QPushButton(this); 
	add->setText(_("&Add"));
	setSizeHint(add); 
	add->setMaximumSize(add->sizeHint());
 
	up = new QPushButton(this); 
	up->setText(_("&Up"));
	setSizeHint(up); 
	up->setMaximumSize(up->sizeHint());
 
	down = new QPushButton(this); 
	down->setText(_("&Down"));
	setSizeHint(down); 
	down->setMaximumSize(down->sizeHint());
 
	remove = new QPushButton(this); 
	remove->setText(_("&Remove"));
	setSizeHint(remove); 
	remove->setMaximumSize(remove->sizeHint());

	buttonOk = new QPushButton(this);
	buttonOk->setText(_("&OK"));
	buttonOk->setDefault(true);
	setSizeHint(buttonOk); 
	buttonOk->setMaximumSize(buttonOk->sizeHint());

	buttonCancel = new QPushButton(this);
	buttonCancel->setText(_("&Cancel"));
	setSizeHint(buttonCancel); 
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
	
	resize(sizeHint());
}


CitationDialog::~CitationDialog()
{
}


void CitationDialog::closeEvent(QCloseEvent * e)
{
	form_->CancelButton();
	e->accept();
}
