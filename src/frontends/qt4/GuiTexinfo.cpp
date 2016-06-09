/**
 * \file GuiTexinfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiTexinfo.h"

#include "FuncRequest.h"

#include "support/filetools.h"
#include "support/FileName.h"

#include "qt_helpers.h"

#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

static QString texFileFromList(QString const & file, QString const & type)
{
	QString lstfile = type + "Files.lst";
	FileName const abslstfile = libFileSearch(QString(), lstfile);
	if (abslstfile.empty())
		return QString();
	QString cs = toqstr(abslstfile.fileContents("UTF-8"));
	cs.replace("\r", "");
	QStringList const result = cs.split("\n").filter(file);
	if (result.empty())
		return QString();
	return result.at(0);
}


GuiTexInfo::GuiTexInfo(GuiView & lv)
	: GuiDialog(lv, "texinfo", qt_("TeX Information"))
{
	setupUi(this);

	warningPosted_ = false;
	activeStyle_ = ClsType;

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(viewPB, SIGNAL(clicked()), this, SLOT(viewClicked()));
	connect(whatStyleCO, SIGNAL(activated(QString)),
		this, SLOT(enableViewPB()));
	connect(whatStyleCO, SIGNAL(activated(int)), this, SLOT(updateView()));
	connect(pathCB, SIGNAL(stateChanged(int)), this, SLOT(updateView()));
	connect(rescanPB, SIGNAL(clicked()), this, SLOT(enableViewPB()));
	connect(rescanPB, SIGNAL(clicked()), this, SLOT(rescanClicked()));
	connect(fileListLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(enableViewPB()));
	connect(fileListLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(enableViewPB()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


void GuiTexInfo::change_adaptor()
{
	changed();
}


void GuiTexInfo::rescanClicked()
{
	// build new *Files.lst
	rescanTexStyles();
	updateStyles();
	enableViewPB();
}


void GuiTexInfo::viewClicked()
{
	// takes advantage of enum order
	static QString const ext[] = { "cls", "sty", "bst", "bib" };
	int const fitem = fileListLW->currentRow();
	QStringList const & data = texdata_[activeStyle_];
	QString file = data[fitem];
	if (!pathCB->isChecked())
		file = texFileFromList(data[fitem], ext[activeStyle_]);
	viewFile(file);
}


void GuiTexInfo::updateView()
{
	// takes advantage of enum order
	updateStyles(static_cast<TexFileType>(whatStyleCO->currentIndex()));
	enableViewPB();
}


void GuiTexInfo::enableViewPB()
{
	viewPB->setEnabled(fileListLW->currentRow() > -1);
}


void GuiTexInfo::updateStyles(TexFileType type)
{
	static QString const filenames[] = {
		"clsFiles.lst", "styFiles.lst", "bstFiles.lst", "bibFiles.lst"
	};

	QString const filename = filenames[type];

	QStringList data = texFileList(filename);
	if (data.empty()) {
		// build filelists of all availabe bst/cls/sty-files.
		// Done through kpsewhich and an external script,
		// saved in *Files.lst
		rescanTexStyles();
		data = texFileList(filename);
	}

	if (!pathCB->isChecked()) {
		for (int i = 0; i != data.size(); ++i)
			data[i] = onlyFileName(data[i]);
	}
	// sort on filename only (no path)
	data.sort();

	fileListLW->clear();
	for(QString const & item : data)
		fileListLW->addItem(item);

	activeStyle_ = type;
	texdata_[type] = data;
}


void GuiTexInfo::updateStyles()
{
	updateStyles(activeStyle_);
}


void GuiTexInfo::viewFile(QString const & filename) const
{
	dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + fromqstr(filename)));
}


/// get a class with full path from the list
/*
string GuiTexInfo::classOptions(string const & classname) const
{
	FileName const filename(texFileFromList(classname, "cls"));
	if (filename.empty())
		return string();
	string optionList;
	ifstream is(filename.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		is >> s;
		if (contains(s, "DeclareOption")) {
			s = s.substr(s.find("DeclareOption"));
			s = split(s, '{');		// cut front
			s = token(s, '}', 0);		// cut end
			optionList += (s + '\n');
		}
	}
	return optionList;
}
*/


Dialog * createGuiTexInfo(GuiView & lv) { return new GuiTexInfo(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiTexinfo.cpp"
