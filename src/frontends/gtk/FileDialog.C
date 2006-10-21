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

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "FileDialogPrivate.h"


namespace lyx {

using lyx::docstring;

using std::string;


FileDialog::FileDialog(docstring const & title,
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


FileDialog::Result const
FileDialog::open(docstring const & path,
		 lyx::support::FileFilterList const & filters,
		 docstring const & suggested)
{
	return private_->open(path, filters, suggested);
}


FileDialog::Result const
FileDialog::opendir(docstring const & path,
		    docstring const & suggested)
{
	return private_->opendir(path, suggested);
}


FileDialog::Result const
FileDialog::save(docstring const & path,
		 lyx::support::FileFilterList const & filters,
		 docstring const & suggested)
{
	return private_->save(path, filters, suggested);
}


} // namespace lyx
