/**
 * \file gtk/FileDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include "FileDialogPrivate.h"

using std::string;


FileDialog::FileDialog(string const & title,
		       kb_action action,
		       Button b1,
		       Button b2)
{
	private_ = new FileDialog::Private(title, action, b1, b2);
}


FileDialog::~FileDialog()
{
	delete private_;
}


FileDialog::Result const FileDialog::open(string const & path,
					  lyx::support::FileFilterList const & filters,
					  string const & suggested)
{
	return private_->open(path, filters, suggested);
}


FileDialog::Result const FileDialog::opendir(string const & path,
					     string const & suggested)
{
	return private_->opendir(path, suggested);
}


FileDialog::Result const FileDialog::save(string const & path,
					  lyx::support::FileFilterList const & filters,
					  string const & suggested)
{
	return private_->save(path, filters, suggested);
}
