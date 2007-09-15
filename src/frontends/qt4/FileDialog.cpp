/**
 * \file qt4/FileDialog.cpp
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

#include "LyXFileDialog.h"
#include "qt_helpers.h"

#include "debug.h"
#include "gettext.h"

#include "support/FileFilterList.h"
#include "support/os.h"

/** when this is defined, the code will use
 * QFileDialog::getOpenFileName and friends to create filedialogs.
 * Effects:
 * - the dialog does not use the quick directory buttons (Button
 *   parameters);
 * - with Qt/Mac or Qt/Win, the dialogs native to the environment are used.
 * - with Qt/Win and Qt <= 4.3.0, there was a number of bugs with our own
 *   file dialog (http://bugzilla.lyx.org/show_bug.cgi?id=3907).
 *
 * Therefore there is a tradeoff in enabling or disabling this (JMarc)
 */
#if defined(Q_WS_MACX) || (defined(Q_WS_WIN) && !defined(Q_CYGWIN_WIN))
#define USE_NATIVE_FILEDIALOG 1
#endif

#ifdef USE_NATIVE_FILEDIALOG
#include <QApplication>
#include "support/filetools.h"

using lyx::support::makeAbsPath;
#endif

namespace lyx {

using support::FileFilterList;
using support::os::internal_path;
using std::endl;


class FileDialog::Private {
public:
	Button b1;
	Button b2;
};


FileDialog::FileDialog(docstring const & t,
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


FileDialog::Result const FileDialog::save(docstring const & path,
					  FileFilterList const & filters,
					  docstring const & suggested)
{
	LYXERR(Debug::GUI) << "Select with path \"" << to_utf8(path)
			   << "\", mask \"" << to_utf8(filters.as_string())
			   << "\", suggested \"" << to_utf8(suggested) << '"' << endl;
	FileDialog::Result result;
	result.first = FileDialog::Chosen;

#ifdef USE_NATIVE_FILEDIALOG
	docstring const startsWith = from_utf8(
		makeAbsPath(to_utf8(suggested), to_utf8(path)).absFilename());
	QString const name = 
		QFileDialog::getSaveFileName(qApp->focusWidget(),
					     toqstr(title_), 
					     toqstr(startsWith), 
					     toqstr(filters.as_string()),
					     0, 
					     QFileDialog::DontConfirmOverwrite);
	result.second = from_utf8(internal_path(fromqstr(name)));
#else
	LyXFileDialog dlg(title_, path, filters, private_->b1, private_->b2);
#if QT_VERSION != 0x040203
	dlg.setFileMode(QFileDialog::AnyFile);
#endif
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setConfirmOverwrite(false);

	if (!suggested.empty())
		dlg.selectFile(toqstr(suggested));

	LYXERR(Debug::GUI) << "Synchronous FileDialog: " << endl;
	int res = dlg.exec();
	LYXERR(Debug::GUI) << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = from_utf8(internal_path(
					fromqstr(dlg.selectedFiles()[0])));
	dlg.hide();
#endif
	return result;
}


FileDialog::Result const FileDialog::open(docstring const & path,
					  FileFilterList const & filters,
					  docstring const & suggested)
{
	LYXERR(Debug::GUI) << "Select with path \"" << to_utf8(path)
			   << "\", mask \"" << to_utf8(filters.as_string())
			   << "\", suggested \"" << to_utf8(suggested) << '"' << endl;
	FileDialog::Result result;
	result.first = FileDialog::Chosen;

#ifdef USE_NATIVE_FILEDIALOG
	docstring const startsWith = from_utf8(
		makeAbsPath(to_utf8(suggested), to_utf8(path)).absFilename());
	result.second = from_utf8(internal_path(fromqstr(
		QFileDialog::getOpenFileName(qApp->focusWidget(),
		toqstr(title_), toqstr(startsWith), toqstr(filters.as_string()) ))));
#else
	LyXFileDialog dlg(title_, path, filters, private_->b1, private_->b2);

	if (!suggested.empty())
		dlg.selectFile(toqstr(suggested));

	LYXERR(Debug::GUI) << "Synchronous FileDialog: " << endl;
	int res = dlg.exec();
	LYXERR(Debug::GUI) << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = from_utf8(internal_path(
					fromqstr(dlg.selectedFiles()[0])));
	dlg.hide();
#endif
	return result;
}


FileDialog::Result const FileDialog::opendir(docstring const & path,
					    docstring const & suggested)
{
	LYXERR(Debug::GUI) << "Select with path \"" << to_utf8(path)
			   << "\", suggested \"" << to_utf8(suggested) << '"' << endl;
	FileDialog::Result result;
	result.first = FileDialog::Chosen;

#ifdef USE_NATIVE_FILEDIALOG
	docstring const startsWith = from_utf8(
		makeAbsPath(to_utf8(suggested), to_utf8(path)).absFilename());
	result.second = from_utf8(internal_path(fromqstr(
		QFileDialog::getExistingDirectory(qApp->focusWidget(),
		toqstr(title_),toqstr(startsWith)))));
#else
	FileFilterList const filter(_("Directories"));

	LyXFileDialog dlg(title_, path, filter, private_->b1, private_->b2);

	dlg.setFileMode(QFileDialog::DirectoryOnly);

	if (!suggested.empty())
		dlg.selectFile(toqstr(suggested));

	LYXERR(Debug::GUI) << "Synchronous FileDialog: " << endl;
	int res = dlg.exec();
	LYXERR(Debug::GUI) << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = from_utf8(internal_path(
					fromqstr(dlg.selectedFiles()[0])));
	dlg.hide();
#endif
	return result;
}


} // namespace lyx
