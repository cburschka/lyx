/**
 * \file urldlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "urldlg.h"

#include "dlg/helpers.h"

using kde_helpers::setSizeHint;

UrlDialog::UrlDialog(FormUrl *form, QWidget *parent, char const * name, bool, WFlags)
	: QDialog(parent,name,0), form_(form)
{
	setCaption(name);

	// widgets

	url = new QLineEdit(this);
	setSizeHint(url);

	urlname = new QLineEdit(this);
	setSizeHint(urlname);

	labelurl = new QLabel(this);
	labelurl->setText(_("Url :"));
	labelurl->setMargin(5);
	setSizeHint(labelurl);
	labelurl->setMaximumSize(labelurl->sizeHint());

	labelurlname = new QLabel(this);
	labelurlname->setText(_("Name :"));
	labelurlname->setMargin(5);
	setSizeHint(labelurlname);
	labelurlname->setMaximumSize(labelurlname->sizeHint());

	htmlurl = new QCheckBox(this);
	htmlurl->setText(_("Generate hyperlink"));
	setSizeHint(htmlurl);
	htmlurl->setMaximumSize(htmlurl->sizeHint());

	buttonOk = new QPushButton(this);
	setSizeHint(buttonOk);
	buttonOk->setMaximumSize(buttonOk->sizeHint());
	buttonOk->setText(_("&OK"));
	buttonOk->setDefault(true);

	buttonCancel = new QPushButton(this);
	setSizeHint(buttonCancel);
	buttonCancel->setMaximumSize(buttonCancel->sizeHint());
	buttonCancel->setText(_("&Cancel"));

	// tooltips

	QToolTip::add(labelurl,_("URL"));
	QToolTip::add(url,_("URL"));
	QToolTip::add(labelurlname,_("Name associated with the URL"));
	QToolTip::add(urlname,_("Name associated with the URL"));
	QToolTip::add(htmlurl,_("Output as a hyperlink ?"));

	// layouts

	topLayout = new QHBoxLayout(this,10);

	layout = new QVBoxLayout();
	topLayout->addLayout(layout);
	layout->addSpacing(10);
	
	urlLayout = new QHBoxLayout();
	layout->addLayout(urlLayout);
	urlLayout->addWidget(labelurl, 0);
	urlLayout->addWidget(url, 1);

	urlnameLayout = new QHBoxLayout();
	layout->addLayout(urlnameLayout);
	urlnameLayout->addWidget(labelurlname, 0);
	urlnameLayout->addWidget(urlname, 1);

	htmlurlLayout = new QHBoxLayout();
	layout->addStretch(1);
	layout->addLayout(htmlurlLayout);
	layout->addStretch(1);
	htmlurlLayout->addWidget(htmlurl);
	htmlurlLayout->addStretch(1);

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
	
	resize(sizeHint());
}


UrlDialog::~UrlDialog()
{
}


void UrlDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}
