/**
 * \file tocdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "tocdlg.h"

#include "dlg/helpers.h"

#ifdef CXX_WORKING_NAMESPACES
using kde_helpers::setSizeHint;
#endif

TocDialog::TocDialog(FormToc * form, QWidget * parent, char const * name, bool, WFlags)
	: QDialog(parent,name,0), form_(form)
{
	setCaption(name);
	setMinimumWidth(350);

	// widgets

	menu = new QComboBox(this);
	menu->insertItem(_("Table of Contents"));
	menu->insertItem(_("List of Figures"));
	menu->insertItem(_("List of Tables"));
	menu->insertItem(_("List of Algorithms"));
	setSizeHint(menu);

	tree = new QListView(this);
	tree->setMinimumHeight(200);
	tree->setRootIsDecorated(true);
	tree->setSorting(-1);
	tree->addColumn("Table of Contents");

	buttonUpdate = new QPushButton(this);
	setSizeHint(buttonUpdate);
	buttonUpdate->setMaximumSize(buttonUpdate->sizeHint());
	buttonUpdate->setText(_("&Update"));

	buttonClose = new QPushButton(this);
	setSizeHint(buttonClose);
	buttonClose->setMaximumSize(buttonClose->sizeHint());
	buttonClose->setText(_("&Close"));
	buttonClose->setDefault(true);

	depth = new QSlider(0, 5, 1, 1, QSlider::Horizontal, this);
	setSizeHint(depth);
	depth->setTickInterval(1);
	depth->setTracking(true);

	depthlabel = new QLabel(this);
	depthlabel->setText(_("Depth"));
	setSizeHint(depthlabel);
	depthlabel->setMaximumSize(depthlabel->sizeHint()); 
 
	// layouts

	topLayout = new QHBoxLayout(this,10);

	layout = new QVBoxLayout();
	topLayout->addLayout(layout);
	layout->addSpacing(10);

	layout->addWidget(menu,0);
	layout->addWidget(tree,1);
	layout->addWidget(depthlabel,0,AlignLeft);
	layout->addWidget(depth,0);

	buttonLayout = new QHBoxLayout();

	layout->addLayout(buttonLayout);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(buttonUpdate, 1);
	buttonLayout->addStretch(2);
	buttonLayout->addWidget(buttonClose, 1);
	buttonLayout->addStretch(1);

	// connections

	connect(tree, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(select_adaptor(QListViewItem *)));
	connect(menu, SIGNAL(activated(int)), this, SLOT(activate_adaptor(int)));
	connect(buttonUpdate, SIGNAL(clicked()), this, SLOT(update_adaptor()));
	connect(buttonClose, SIGNAL(clicked()), this, SLOT(close_adaptor()));
	connect(depth, SIGNAL(valueChanged(int)), this, SLOT(depth_adaptor(int)));
	
	resize(sizeHint());
}


void TocDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}


TocDialog::~TocDialog()
{
}
