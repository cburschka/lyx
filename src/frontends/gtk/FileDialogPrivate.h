// -*- C++ -*-
/**
 * \file FileDialogPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
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

private:
	void onButton1Clicked();
	void onButton2Clicked();
	Gtk::FileSelection fileSelection_;
	Gtk::Button button1_;
	Gtk::Button button2_;
	std::string dir1_;
	std::string dir2_;
	kb_action action_;
};

#endif
