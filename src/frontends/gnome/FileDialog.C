/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= 
 *
 * \author Baruch Even 
 **/

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "FileDialog.h"
#include "debug.h"
#include "support/lstrings.h"
#include <gtk--/fileselection.h>
#include <gnome--/main.h>
#include <gtk/gtkbutton.h>

#include <sigc++/signal_system.h>

#include "LyXView.h" // This is only needed while we have the xforms part!
#include "bufferview_funcs.h"
// FileDialog::Private

class FileDialog::Private : public SigC::Object {
public:
	Private(string const & title);

	void set_modal(bool modal) { modal_ = modal; }
	void set_complete(const string & pattern) { sel_.complete(pattern); }
	void set_filename(const string & filename) { sel_.set_filename(filename);}

	string const exec();

	void button_clicked(bool canceled);
	void ok_clicked()     { button_clicked(false); }
	void cancel_clicked() { button_clicked(true); }
	
private:
	Gtk::FileSelection sel_;
	bool modal_;
	bool canceled_;
};

FileDialog::Private::Private(string const & title)
	: sel_(title), modal_(false) 
{
	sel_.get_ok_button()->clicked.connect(slot(this, 
			&FileDialog::Private::ok_clicked));
	sel_.get_cancel_button()->clicked.connect(slot(this, 
			&FileDialog::Private::cancel_clicked));
}

string const FileDialog::Private::exec()
{
	canceled_ = false;
	sel_.set_modal(modal_);
	sel_.show();
	Gnome::Main::run();
	// Find if its canceled or oked and return as needed.
	
	if (canceled_)
		return string();
	else
		return sel_.get_filename();
}

void FileDialog::Private::button_clicked(bool canceled)
{
	canceled_ = canceled;
	sel_.hide();
	Gnome::Main::quit();
}

// FileDialog

FileDialog::FileDialog(LyXView * lv, string const & title, kb_action a, 
		Button /*b1*/, Button /*b2*/)
	: private_(new Private(title))
	, lv_(lv), title_(title), success_(a)
{
		private_->set_modal(LFUN_SELECT_FILE_SYNC == a);
}


FileDialog::~FileDialog()
{
	delete private_;
}


FileDialog::Result const 
FileDialog::Select(string const & path, string const & mask, 
		string const & suggested)
{
	// For some reason we need to ignore the asynchronous method...
#if 0
	if (LFUN_SELECT_FILE_SYNC != success_) {
		lyxerr << "Asynchronous file dialog." << std::endl;
		private_->show();

		return FileDialog::Result(Later, string());
	}
#endif
	lyxerr << "Synchronous file dialog." << std::endl;
	
	lyxerr << "Path: " << path << "\nMask: " << mask << "\nSuggested: " << suggested << std::endl;
	
	string filter = mask;
	rsplit(mask, filter, '|');
	private_->set_complete(mask);
	private_->set_filename(path+suggested);
	
	ProhibitInput(lv_->view());
	string const filename = private_->exec();
	AllowInput(lv_->view());

	// Collect the info and return it for synchronous dialog.
	return FileDialog::Result(Chosen, filename);
}
