// -*- C++ -*-
/**
 * \file FileDialogPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FILE_DIALOG_PRIVATE_H
#define FILE_DIALOG_PRIVATE_H

#include <gtkmm.h>
#include "frontends/FileDialog.h"

class FileDialog::Private : public SigC::Object
{
public:
	Private(string const & title,
		kb_action action,
		FileDialog::Button b1, FileDialog::Button b2);
	FileDialog::Result const open(string const & path,
				      string const & mask,
				      string const & suggested);
	FileDialog::Result const opendir(string const & path,
					 string const & suggested);

	FileDialog::Result const save(string const & path,
				      string const & mask,
				      string const & suggested);

private:
	void onButton1Clicked();
	void onButton2Clicked();
	Gtk::FileSelection fileSelection_;
	Gtk::Button button1_;
	Gtk::Button button2_;
	string dir1_;
	string dir2_;
	kb_action action_;
};

#endif
