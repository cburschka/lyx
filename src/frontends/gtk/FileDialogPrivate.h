// -*- C++ -*-
/**
 * \file FileDialogPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILE_DIALOG_PRIVATE_H
#define FILE_DIALOG_PRIVATE_H

#include "frontends/FileDialog.h"

#include <gtkmm.h>


namespace lyx {

class FileDialog::Private : public sigc::trackable {
public:
	Private(lyx::docstring const & title,
		kb_action action,
		FileDialog::Button b1, FileDialog::Button b2);
	FileDialog::Result const open(lyx::docstring const & path,
		lyx::support::FileFilterList const & filters,
		lyx::docstring const & suggested);
	FileDialog::Result const opendir(lyx::docstring const & path,
		lyx::docstring const & suggested);
	FileDialog::Result const save(lyx::docstring const & path,
		lyx::support::FileFilterList const & filters,
		lyx::docstring const & suggested);

	FileDialog::Result const showChooser(lyx::docstring const & path,
		lyx::support::FileFilterList const & filters,
		lyx::docstring const & suggested);

private:
	kb_action action_;
	Gtk::FileChooserDialog fileChooser_;
};


} // namespace lyx

#endif
