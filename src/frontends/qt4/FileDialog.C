/**
 * \file qt4/FileDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/FileDialog.h"

#include "FileDialog_private.h"
#include "qt_helpers.h"

#include "debug.h"
#include "gettext.h"

#include "support/filefilterlist.h"

/** when this is defined, the code will use
 * QFileDialog::getOpenFileName and friends to create filedialogs.
 * Effects:
 * - the dialog does not use the quick directory buttons (Button
 *   parameters);
 * - with Qt/Mac or Qt/Win, the dialogs native to the environment are used.
 *
 * Therefore there is a tradeoff in enabling or disabling this (JMarc)
 */
#ifdef Q_WS_MACX
#define USE_NATIVE_FILEDIALOG 1
#endif

#ifdef USE_NATIVE_FILEDIALOG
#include <qapplication.h>
#include "support/filetools.h"
using lyx::support::makeAbsPath;
#endif

using lyx::support::FileFilterList;

using std::endl;
using std::string;


class FileDialog::Private {
public:
	Button b1;
	Button b2;
};


FileDialog::FileDialog(string const & t,
		       kb_action s, Button b1, Button b2)
	: private_(new FileDialog::Private), title_(t), success_(s)
{
	private_->b1 = b1;
	private_->b2 = b2;
}


FileDialog::~FileDialog()
{
	delete private_;
}


FileDialog::Result const FileDialog::save(string const & path,
					  FileFilterList const & filters,
					  string const & suggested)
{
	lyxerr[Debug::GUI] << "Select with path \"" << path
			   << "\", mask \"" << filters.as_string()
			   << "\", suggested \"" << suggested << '"' << endl;
	FileDialog::Result result;
	result.first = FileDialog::Chosen;

#ifdef USE_NATIVE_FILEDIALOG
	string const startsWith = makeAbsPath(suggested, path);
	result.second = fromqstr(QFileDialog::getSaveFileName(
		qApp->focusWidget(),
		title_.c_str(), toqstr(startsWith), toqstr(filters.as_string()) ));
#else
	LyXFileDialog dlg(title_, path, filters, private_->b1, private_->b2);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);

	if (!suggested.empty())
		dlg.selectFile(toqstr(suggested));

	lyxerr[Debug::GUI] << "Synchronous FileDialog: " << endl;
	int res = dlg.exec();
	lyxerr[Debug::GUI] << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = fromqstr(dlg.selectedFiles()[0]);
	dlg.hide();
#endif
	return result;
}


FileDialog::Result const FileDialog::open(string const & path,
					  FileFilterList const & filters,
					  string const & suggested)
{
	lyxerr[Debug::GUI] << "Select with path \"" << path
			   << "\", mask \"" << filters.as_string()
			   << "\", suggested \"" << suggested << '"' << endl;
	FileDialog::Result result;
	result.first = FileDialog::Chosen;

#ifdef USE_NATIVE_FILEDIALOG
	string const startsWith = makeAbsPath(suggested, path);
	result.second = fromqstr(QFileDialog::getOpenFileName(
		qApp->focusWidget(), 
		title_.c_str(), toqstr(startsWith), toqstr(filters.as_string()) ));
#else
	LyXFileDialog dlg(title_, path, filters, private_->b1, private_->b2);

	if (!suggested.empty())
		dlg.selectFile(toqstr(suggested));

	lyxerr[Debug::GUI] << "Synchronous FileDialog: " << endl;
	int res = dlg.exec();
	lyxerr[Debug::GUI] << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = fromqstr(dlg.selectedFiles()[0]);
	dlg.hide();
#endif
	return result;
}


FileDialog::Result const FileDialog::opendir(string const & path,
					    string const & suggested)
{
	lyxerr[Debug::GUI] << "Select with path \"" << path
			   << "\", suggested \"" << suggested << '"' << endl;
	FileDialog::Result result;
	result.first = FileDialog::Chosen;

#ifdef USE_NATIVE_FILEDIALOG
	string const startsWith = makeAbsPath(suggested, path);
	result.second = fromqstr(QFileDialog::getExistingDirectory(
		qApp->focusWidget(),
		title_.c_str(),toqstr(startsWith) ));
#else
	FileFilterList const filter(_("Directories"));

	LyXFileDialog dlg(title_, path, filter, private_->b1, private_->b2);

	dlg.setFileMode(QFileDialog::DirectoryOnly);

	if (!suggested.empty())
		dlg.selectFile(toqstr(suggested));

	lyxerr[Debug::GUI] << "Synchronous FileDialog: " << endl;
	int res = dlg.exec();
	lyxerr[Debug::GUI] << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = fromqstr(dlg.selectedFiles()[0]);
	dlg.hide();
#endif
	return result;
}
