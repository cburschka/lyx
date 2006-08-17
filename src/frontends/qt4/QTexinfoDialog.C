/**
 * \file QTexinfoDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTexinfoDialog.h"
#include "QTexinfo.h"

#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QCloseEvent>

using std::vector;
using std::string;

namespace lyx {
namespace frontend {


QTexinfoDialog::QTexinfoDialog(QTexinfo * form)
	:form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( viewPB, SIGNAL( clicked() ), this, SLOT( viewClicked() ) );
    connect( whatStyle, SIGNAL( activated(const QString&) ), this, SLOT( enableViewPB() ) );
    connect( whatStyle, SIGNAL( activated(int) ), this, SLOT( update() ) );
    connect( path, SIGNAL( stateChanged(int) ), this, SLOT( update() ) );
    connect( rescanPB, SIGNAL( clicked() ), this, SLOT( enableViewPB() ) );
    connect( rescanPB, SIGNAL( clicked() ), this, SLOT( rescanClicked() ) );
    connect( fileList, SIGNAL( itemClicked(QListWidgetItem*) ), this, SLOT( enableViewPB() ) );
}


void QTexinfoDialog::change_adaptor()
{
	form_->changed();
}


void QTexinfoDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QTexinfoDialog::rescanClicked()
{
	// build new *Files.lst
	rescanTexStyles();
	form_->updateStyles();
	enableViewPB();
}


void QTexinfoDialog::viewClicked()
{
	vector<string>::size_type const fitem = fileList->currentRow();
	vector<string> const & data = form_->texdata_[form_->activeStyle];
	string file = data[fitem];
	if (!path->isChecked())
		file = getTexFileFromList(data[fitem],
			form_->controller().getFileType(form_->activeStyle));
	form_->controller().viewFile(file);
}


void QTexinfoDialog::update()
{
	switch (whatStyle->currentIndex()) {
	case 0:
		form_->updateStyles(ControlTexinfo::cls);
		break;
	case 1:
		form_->updateStyles(ControlTexinfo::sty);
		break;
	case 2:
		form_->updateStyles(ControlTexinfo::bst);
		break;
	default:
		break;
	}

	enableViewPB();
}


void QTexinfoDialog::enableViewPB()
{
	viewPB->setEnabled(fileList->currentRow() > -1);
}

} // namespace frontend
} // namespace lyx

#include "QTexinfoDialog_moc.cpp"
