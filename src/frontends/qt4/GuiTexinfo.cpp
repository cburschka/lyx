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

#include "ControlTexinfo.h"
#include "qt_helpers.h"

#include "support/filetools.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>

#include <algorithm>

using std::string;
using std::vector;


namespace lyx {
namespace frontend {

GuiTexinfoDialog::GuiTexinfoDialog(LyXView & lv)
	: GuiDialog(lv, "texinfo")
{
	setupUi(this);
	setViewTitle(_("TeX Information"));
	setController(new ControlTexinfo(*this));

	warningPosted = false;
	activeStyle = ControlTexinfo::cls;

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(viewPB, SIGNAL(clicked()), this, SLOT(viewClicked()));
	connect(whatStyleCO, SIGNAL(activated(const QString &)),
		this, SLOT(enableViewPB()));
	connect(whatStyleCO, SIGNAL(activated(int)), this, SLOT(updateView()));
	connect(pathCB, SIGNAL(stateChanged(int)), this, SLOT(updateView()));
	connect(rescanPB, SIGNAL(clicked()), this, SLOT(enableViewPB()));
	connect(rescanPB, SIGNAL(clicked()), this, SLOT(rescanClicked()));
	connect(fileListLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(enableViewPB()));
	connect(fileListLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(enableViewPB()));

	updateStyles(ControlTexinfo::cls);

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


ControlTexinfo & GuiTexinfoDialog::controller()
{
	return static_cast<ControlTexinfo &>(GuiDialog::controller());
}


void GuiTexinfoDialog::change_adaptor()
{
	changed();
}


void GuiTexinfoDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiTexinfoDialog::rescanClicked()
{
	// build new *Files.lst
	rescanTexStyles();
	updateStyles();
	enableViewPB();
}


void GuiTexinfoDialog::viewClicked()
{
	size_t const fitem = fileListLW->currentRow();
	vector<string> const & data = texdata_[activeStyle];
	string file = data[fitem];
	if (!pathCB->isChecked())
		file = getTexFileFromList(data[fitem],
			controller().getFileType(activeStyle));
	controller().viewFile(file);
}


void GuiTexinfoDialog::updateView()
{
	switch (whatStyleCO->currentIndex()) {
		case 0:
			updateStyles(ControlTexinfo::cls);
			break;
		case 1:
			updateStyles(ControlTexinfo::sty);
			break;
		case 2:
			updateStyles(ControlTexinfo::bst);
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


void GuiTexinfoDialog::updateStyles(ControlTexinfo::texFileSuffix type)
{
	ContentsType & data = texdata_[type];

	string filename;
	switch (type) {
	case ControlTexinfo::bst:
		filename = "bstFiles.lst";
		break;
	case ControlTexinfo::cls:
		filename = "clsFiles.lst";
		break;
	case ControlTexinfo::sty:
		filename = "styFiles.lst";
		break;
	}
	getTexFileList(filename, data);
	if (data.empty()) {
		// build filelists of all availabe bst/cls/sty-files.
		// Done through kpsewhich and an external script,
		// saved in *Files.lst
		rescanTexStyles();
		getTexFileList(filename, data);
	}
	bool const withFullPath = pathCB->isChecked();
	if (withFullPath)
		return;
	vector<string>::iterator it1  = data.begin();
	vector<string>::iterator end1 = data.end();
	for (; it1 != end1; ++it1)
		*it1 = support::onlyFilename(*it1);

	// sort on filename only (no path)
	std::sort(data.begin(), data.end());

	fileListLW->clear();
	ContentsType::const_iterator it  = data.begin();
	ContentsType::const_iterator end = data.end();
	for (; it != end; ++it)
		fileListLW->addItem(toqstr(*it));

	activeStyle = type;
}


void GuiTexinfoDialog::updateStyles()
{
	updateStyles(activeStyle);
}

} // namespace frontend
} // namespace lyx


#include "GuiTexinfo_moc.cpp"
