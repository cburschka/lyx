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

class FileDialog::Private : public sigc::trackable {
public:
	Private(std::string const & title,
		kb_action action,
		FileDialog::Button b1, FileDialog::Button b2);
	FileDialog::Result const open(std::string const & path,
		lyx::support::FileFilterList const & filters,
		std::string const & suggested);
	FileDialog::Result const opendir(std::string const & path,
		std::string const & suggested);
	FileDialog::Result const save(std::string const & path,
		lyx::support::FileFilterList const & filters,
		std::string const & suggested);

	FileDialog::Result const showChooser(std::string const & path,
		lyx::support::FileFilterList const & filters,
		std::string const & suggested);

private:
	Gtk::FileChooserDialog fileChooser_;
	kb_action action_;
};

#endif
