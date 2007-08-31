/**
 * \file GuiTexinfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiTexinfo.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "support/filetools.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiTexinfoDialog
//
/////////////////////////////////////////////////////////////////////


GuiTexinfoDialog::GuiTexinfoDialog(GuiTexinfo * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(viewPB, SIGNAL(clicked()), this, SLOT(viewClicked()));
	connect(whatStyleCO, SIGNAL(activated(const QString &)),
		this, SLOT(enableViewPB()));
	connect(whatStyleCO, SIGNAL(activated(int)), this, SLOT(update()));
	connect(pathCB, SIGNAL(stateChanged(int)), this, SLOT(update()));
	connect(rescanPB, SIGNAL(clicked()), this, SLOT(enableViewPB()));
	connect(rescanPB, SIGNAL(clicked()), this, SLOT(rescanClicked()));
	connect(fileListLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT( enableViewPB() ) );
	connect(fileListLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(enableViewPB()));
}


void GuiTexinfoDialog::change_adaptor()
{
	form_->changed();
}


void GuiTexinfoDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiTexinfoDialog::rescanClicked()
{
	// build new *Files.lst
	rescanTexStyles();
	form_->updateStyles();
	enableViewPB();
}


void GuiTexinfoDialog::viewClicked()
{
	size_t const fitem = fileListLW->currentRow();
	vector<string> const & data = form_->texdata_[form_->activeStyle];
	string file = data[fitem];
	if (!pathCB->isChecked())
		file = getTexFileFromList(data[fitem],
			form_->controller().getFileType(form_->activeStyle));
	form_->controller().viewFile(file);
}


void GuiTexinfoDialog::update()
{
	switch (whatStyleCO->currentIndex()) {
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


void GuiTexinfoDialog::enableViewPB()
{
	viewPB->setEnabled(fileListLW->currentRow() > -1);
}


/////////////////////////////////////////////////////////////////////
//
// GuiTexinfo
//
/////////////////////////////////////////////////////////////////////


GuiTexinfo::GuiTexinfo(Dialog & parent)
	: GuiView<GuiTexinfoDialog>(parent, _("TeX Information")),
	  warningPosted(false), activeStyle(ControlTexinfo::cls)
{
}


void GuiTexinfo::build_dialog()
{
	dialog_.reset(new GuiTexinfoDialog(this));

	updateStyles(ControlTexinfo::cls);

	bcview().setCancel(dialog_->closePB);
}


void GuiTexinfo::updateStyles(ControlTexinfo::texFileSuffix whichStyle)
{
	ContentsType & data = texdata_[whichStyle];
	bool const withFullPath = dialog_->pathCB->isChecked();

	getTexFileList(whichStyle, data, withFullPath);

	dialog_->fileListLW->clear();
	ContentsType::const_iterator it  = data.begin();
	ContentsType::const_iterator end = data.end();
	for (; it != end; ++it)
		dialog_->fileListLW->addItem(toqstr(*it));

	activeStyle = whichStyle;
}


void GuiTexinfo::updateStyles()
{
	updateStyles(activeStyle);
}

} // namespace frontend
} // namespace lyx


#include "GuiTexinfo_moc.cpp"
