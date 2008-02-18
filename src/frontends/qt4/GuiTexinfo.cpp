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

#include "support/debug.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/lstrings.h"

#include "qt_helpers.h"

#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>

#include <fstream>
#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


static string texFileFromList(string const & file, string const & type)
{
	string file_ = file;
	// do we need to add the suffix?
	if (!(getExtension(file) == type))
		file_ += '.' + type;

	lyxerr << "Searching for file " << file_ << endl;

	string lstfile = type + "Files.lst";
	if (type == "cls")
		lstfile = "clsFiles.lst";
	else if (type == "sty")
		lstfile = "styFiles.lst";
	else if (type == "bst")
		lstfile = "bstFiles.lst";
	else if (type == "bib")
		lstfile = "bibFiles.lst";
	FileName const abslstfile = libFileSearch(string(), lstfile);
	if (abslstfile.empty()) {
		lyxerr << "File `'" << lstfile << "' not found." << endl;
		return string();
	}
	// FIXME UNICODE
	string const allClasses = to_utf8(abslstfile.fileContents("UTF-8"));
	int entries = 0;
	string classfile = token(allClasses, '\n', entries);
	int count = 0;
	while ((!contains(classfile, file) ||
		(onlyFilename(classfile) != file)) &&
		(++count < 1000)) {
		classfile = token(allClasses, '\n', ++entries);
	}

	// now we have filename with full path
	lyxerr << "with full path: " << classfile << endl;

	return classfile;
}


GuiTexInfo::GuiTexInfo(GuiView & lv)
	: GuiDialog(lv, "texinfo", qt_("TeX Information"))
{
	setupUi(this);

	warningPosted = false;
	activeStyle = ClsType;

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

	updateStyles(ClsType);

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
	size_t const fitem = fileListLW->currentRow();
	vector<string> const & data = texdata_[activeStyle];
	string file = data[fitem];
	if (!pathCB->isChecked())
		file = texFileFromList(data[fitem], fileType(activeStyle));
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
	ContentsType & data = texdata_[type];

	static string filenames[] = { "clsFile.lst", "styFiles.lst", "bstFiles.lst" };
	string filename = filenames[type];

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
		*it1 = onlyFilename(*it1);

	// sort on filename only (no path)
	sort(data.begin(), data.end());

	fileListLW->clear();
	ContentsType::const_iterator it  = data.begin();
	ContentsType::const_iterator end = data.end();
	for (; it != end; ++it)
		fileListLW->addItem(toqstr(*it));

	activeStyle = type;
}


void GuiTexInfo::updateStyles()
{
	updateStyles(activeStyle);
}


void GuiTexInfo::viewFile(string const & filename) const
{
	dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + filename));
}


/// get a class with full path from the list
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


string GuiTexInfo::fileType(TexFileType type) const
{
	// takes advantage of enum order
	static string const ext[] = { "cls", "sty", "bst" };
	return ext[type];
}


Dialog * createGuiTexInfo(GuiView & lv) { return new GuiTexInfo(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiTexinfo_moc.cpp"
