/*
 * formtocdialog.C
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

#include "formtocdialog.h"

FormTocDialog::FormTocDialog(FormToc *form, QWidget *parent, const char *name, bool, WFlags)
	: QDialog(parent,name,false), form_(form)
{
	setCaption(name);
	setMinimumWidth(350);

	// widgets

	menu = new QComboBox(this);
	menu->insertItem(_("Table of Contents"));
	menu->insertItem(_("List of Figures"));
	menu->insertItem(_("List of Tables"));
	menu->insertItem(_("List of Algorithms"));
	menu->setMinimumSize(menu->sizeHint()); 
 
	tree = new KTreeList(this,"tree");
	tree->setMinimumHeight(200);

        buttonUpdate = new QPushButton(this);
        buttonUpdate->setMinimumSize(buttonUpdate->sizeHint());
        buttonUpdate->setMaximumSize(buttonUpdate->sizeHint());
        buttonUpdate->setText(_("&Update"));

        buttonClose = new QPushButton(this);
        buttonClose->setMinimumSize(buttonClose->sizeHint());
        buttonClose->setMaximumSize(buttonClose->sizeHint());
        buttonClose->setText(_("&Close"));
        buttonClose->setDefault(true);

	// layouts
 
        topLayout = new QHBoxLayout(this,10);

        layout = new QVBoxLayout();
        topLayout->addLayout(layout);
        layout->addSpacing(10);

	layout->addWidget(menu,0);
	layout->addWidget(tree,1);

        buttonLayout = new QHBoxLayout();

        layout->addLayout(buttonLayout);
        buttonLayout->addStretch(1);
	buttonLayout->addWidget(buttonUpdate, 1);
	buttonLayout->addStretch(2);
        buttonLayout->addWidget(buttonClose, 1);
        buttonLayout->addStretch(1);

	// connections

	connect(tree, SIGNAL(highlighted(int)), this, SLOT(highlight_adaptor(int)));
	connect(menu, SIGNAL(activated(int)), this, SLOT(activate_adaptor(int))); 
	connect(buttonUpdate, SIGNAL(clicked()), this, SLOT(update_adaptor())); 
	connect(buttonClose, SIGNAL(clicked()), this, SLOT(close_adaptor()));
}

void FormTocDialog::closeEvent(QCloseEvent *e)
{
	form_->close();
	e->accept();
}

FormTocDialog::~FormTocDialog()
{
}
