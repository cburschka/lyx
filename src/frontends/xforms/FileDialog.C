/**
 * \file xforms/FileDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/FileDialog.h"
#include "FormFiledialog.h"

#include "debug.h"
#include "gettext.h"

#include "support/lstrings.h"

using namespace lyx::support;

using std::endl;
using std::make_pair;
using std::pair;


FileDialog::FileDialog(string const &t, kb_action s, Button b1, Button b2)
	: private_(0), title_(t), success_(s)
{
	private_ = new FileDialog::Private();

	private_->SetButton(0, b1.first, b1.second);
	private_->SetButton(1, b2.first, b2.second);
}


FileDialog::~FileDialog()
{
	delete private_;
}


FileDialog::Result const FileDialog::save(string const & path, string const & mask, string const & suggested)
{
	return open(path, mask, suggested);
}


FileDialog::Result const FileDialog::opendir(string const & path, string const & suggested)
{
	lyxerr[Debug::GUI] << "filedialog open  with path \"" << path << "\", suggested \""
		<< suggested << '"' << endl;

	// no support for asynchronous selection yet

	FileDialog::Result result;

	result.first = FileDialog::Chosen;
	result.second = private_->SelectDir(title_, path, suggested);

	return result;
}


FileDialog::Result const FileDialog::open(string const & path, string const & mask, string const & suggested)
{
	string filter = mask;

	if (mask.empty())
		filter = _("*");
	else {
		rsplit(mask, filter, '|');
		if (filter.empty())
			filter = mask;
	}

	lyxerr[Debug::GUI] << "filedialog open  with path \"" << path << "\", mask \""
		<< filter << "\", suggested \"" << suggested << '"' << endl;

	// no support for asynchronous selection yet

	FileDialog::Result result;

	result.first = FileDialog::Chosen;
	result.second = private_->Select(title_, path, filter, suggested);

	return result;
}
