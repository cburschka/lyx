/**
 * \file refdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
 
#include "refdlg.h"
#include "dlg/helpers.h"
#include "insets/insetref.h"

#ifdef CXX_WORKING_NAMESPACES
using kde_helpers::setSizeHint;
#endif

RefDialog::RefDialog(FormRef * form, QWidget * parent, char const * name, bool, WFlags)
	: QDialog(parent,name,0), form_(form)
{
	setCaption(name);

	// widgets

	refs = new QListBox(this);
	refs->setMinimumSize(200,200);
 
	labelrefs = new QLabel(this);
	labelrefs->setText(_("Available References"));
	labelrefs->setMargin(5);
	setSizeHint(labelrefs);
	labelrefs->setMaximumSize(labelrefs->sizeHint());

	refname = new QLineEdit(this);
	setSizeHint(refname);
	// FIXME: should user be able to edit this ? what's it for ? - jbl 
	refname->setFocusPolicy(QWidget::NoFocus);
 
	reference = new QLineEdit(this); 
	setSizeHint(reference);
	reference->setFocusPolicy(QWidget::NoFocus);

	labelrefname = new QLabel(this);
	labelrefname->setText(_("Name :"));
	labelrefname->setMargin(5);
	setSizeHint(labelrefname);
	labelrefname->setMaximumSize(labelrefname->sizeHint());

	labelreference = new QLabel(this);
	labelreference->setText(_("Reference :"));
	labelreference->setMargin(5);
	setSizeHint(labelreference);
	labelreference->setMaximumSize(labelreference->sizeHint());

	sort = new QCheckBox(this); 
	sort->setText(_("Sort"));
	setSizeHint(sort);
	sort->setMaximumSize(sort->sizeHint());
 
	type = new QComboBox(this);
	for (int i = 0; !InsetRef::types[i].latex_name.empty(); ++i)
		type->insertItem(_(InsetRef::types[i].gui_name.c_str()));
	setSizeHint(type);
	 
	labeltype = new QLabel(this); 
	labeltype->setText(_("Reference Type"));
	labeltype->setMargin(5);
	setSizeHint(labeltype);
	labeltype->setMaximumSize(labeltype->sizeHint());
 
	buttonGoto = new QPushButton(this);
	buttonGoto->setText(_("&Goto reference"));
	setSizeHint(buttonGoto);
	buttonGoto->setMaximumSize(buttonGoto->sizeHint());

	buttonUpdate = new QPushButton(this);
	buttonUpdate->setText(_("&Update"));
	setSizeHint(buttonUpdate);
	buttonUpdate->setMaximumSize(buttonUpdate->sizeHint());

	buttonOk = new QPushButton(this);
	buttonOk->setText(_("&OK"));
	setSizeHint(buttonOk);
	buttonOk->setMaximumSize(buttonOk->sizeHint());
	buttonOk->setDefault(true);

	buttonCancel = new QPushButton(this);
	buttonCancel->setText(_("&Cancel"));
	setSizeHint(buttonCancel);
	buttonCancel->setMaximumSize(buttonCancel->sizeHint());

	// tooltips

	QToolTip::add(type,_("Reference as it appears in output"));
	QToolTip::add(sort,_("Sort references in alphabetical order ?"));
	QToolTip::add(buttonUpdate,_("Update list of references shown"));
	QToolTip::add(buttonGoto,_("Jump to reference in document"));
	QToolTip::add(refname,_("FIXME please !")); 

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
	connect(refs, SIGNAL(selected(const char *)), this, SLOT(select_adaptor(const char *)));
	connect(sort, SIGNAL(toggled(bool)), this, SLOT(sort_adaptor(bool))); 
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(apply_adaptor()));
	connect(buttonUpdate, SIGNAL(clicked()), this, SLOT(update_adaptor()));
	connect(buttonGoto, SIGNAL(clicked()), this, SLOT(goto_adaptor())); 
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(close_adaptor()));
	
	resize(sizeHint());
}


RefDialog::~RefDialog()
{
}


void RefDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}
