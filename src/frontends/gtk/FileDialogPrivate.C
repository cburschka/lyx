/**
 * \file FileDialogPrivate.C
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


FileDialog::Private::Private(string const & title,
			     kb_action action,
			     FileDialog::Button b1,
			     FileDialog::Button b2) :
	action_(action)
{
	fileSelection_.set_title(title);
/*	fileSelection_.get_button_area()->children().push_back(
		Gtk::Box_Helpers::Element(button1_));
	fileSelection_.get_button_area()->children().push_back(
		Gtk::Box_Helpers::Element(button2_));
	button1_.signal_clicked().connect(
		SigC::slot(*this, &FileDialog::Private::onButton1Clicked));
	button2_.signal_clicked().connect(
	SigC::slot(*this, &FileDialog::Private::onButton2Clicked));
	if (!b1.first.empty() && !b1.second.empty()) {
		string::size_type pos = b1.first.find('|');
		button1_.set_label(
			Glib::locale_to_utf8(b1.first.substr(0, pos)));
		dir1_ = b1.second;
		button1_.show();
	}
	if (!b2.first.empty() && !b2.second.empty()) {
		string::size_type pos = b2.first.find('|');
		button2_.set_label(
			Glib::locale_to_utf8(b2.first.substr(0, pos)));
		dir2_ = b2.second;
		button2_.show();
	}*/
}


void FileDialog::Private::onButton1Clicked()
{
	fileSelection_.set_filename(dir1_);
}


void FileDialog::Private::onButton2Clicked()
{
	fileSelection_.set_filename(dir2_);
}


FileDialog::Result const FileDialog::Private::open(string const & path,
						   string const & /*mask*/,
						   string const & /*suggested*/)
{
	fileSelection_.set_filename(path);
	fileSelection_.get_file_list()->get_parent()->show();
	Result result;
	result.first = FileDialog::Chosen;
	if (fileSelection_.run() == Gtk::RESPONSE_OK)
		result.second = fileSelection_.get_filename();
	else
		result.second = string();
	return result;
}


FileDialog::Result const FileDialog::Private::opendir(string const & path,
						      string const & /*suggested*/)
{
	fileSelection_.set_filename(path);
	fileSelection_.get_file_list()->get_parent()->hide();
	Result result;
	result.first = FileDialog::Chosen;
	if (fileSelection_.run() == Gtk::RESPONSE_OK)
		result.second = fileSelection_.get_filename();
	else
		result.second = string();
	return result;
}


FileDialog::Result const FileDialog::Private::save(string const & path,
						   string const & mask,
						   string const & suggested)
{
	return open(path, mask, suggested);
}
