/**
 * \file xforms/FileDialog.C
 * Read the file COPYING
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormFiledialog.h"
#include "frontends/FileDialog.h"
// temp. hack until Allow/prohibitInput is not
// needed any more in src/ - for now it's simplest
// to leave it there
#include "frontends/LyXView.h"
#include "bufferview_funcs.h"
#include "gettext.h"
#include "commandtags.h"
#include "debug.h"
#include "support/lstrings.h"
#include <utility>


using std::make_pair;
using std::pair;
using std::endl;

FileDialog::FileDialog(LyXView *lv, string const &t, kb_action s, Button b1, Button b2)
	: private_(0), lv_(lv), title_(t), success_(s)
{
	private_ = new FileDialog::Private(lv->getDialogs());

	private_->SetButton(0, b1.first, b1.second);
	private_->SetButton(1, b2.first, b2.second);
}


FileDialog::~FileDialog()
{
	delete private_;
}


FileDialog::Result const FileDialog::Select(string const & path, string const & mask, string const & suggested)
{
	string filter = mask;

	if (mask.empty())
		filter = _("*");
	else {
		rsplit(mask, filter, '|');
		if (filter.empty())
			filter = mask;
	}

	lyxerr[Debug::GUI] << "Select with path \"" << path << "\", mask \"" << filter << "\", suggested \"" << suggested << "\"" << endl;

	// no support for asynchronous selection yet

	lv_->prohibitInput();

	FileDialog::Result result;

	result.first = FileDialog::Chosen;
	result.second = private_->Select(title_, path, filter, suggested);

	lv_->allowInput();

	return result;
}
