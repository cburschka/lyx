/**
 * \file FileDialogPrivate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "FileDialogPrivate.h"

#include "support/filefilterlist.h"
#include "debug.h"

using std::string;

FileDialog::Private::Private(string const & title,
			     kb_action action,
			     FileDialog::Button /*b1*/,
			     FileDialog::Button /*b2*/) :
	action_(action),
	fileChooser_("You shouldn't see this", Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	fileChooser_.set_title(title);
}


FileDialog::Result const
FileDialog::Private::open(string const & path,
			  lyx::support::FileFilterList const & filters,
			  string const & suggested)
{
	fileChooser_.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
	fileChooser_.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	fileChooser_.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	return showChooser(path, filters, suggested);
}


FileDialog::Result const FileDialog::Private::opendir(string const & path,
						      string const & suggested)
{
	fileChooser_.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	fileChooser_.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	fileChooser_.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	return showChooser(path, lyx::support::FileFilterList(), suggested);
}


FileDialog::Result const FileDialog::Private::save(string const & path,
						   lyx::support::FileFilterList const & filters,
						   string const & suggested)
{
	fileChooser_.set_action(Gtk::FILE_CHOOSER_ACTION_SAVE);
	fileChooser_.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	fileChooser_.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

	return showChooser(path, filters, suggested);
}


FileDialog::Result const FileDialog::Private::showChooser(string const & path,
			  lyx::support::FileFilterList const & filters,
			  string const & suggested)
{
	lyxerr[Debug::GUI] << "File Dialog with path \"" << path
			   << "\", mask \"" << filters.as_string()
			   << "\", suggested \"" << suggested << "\"\n";

	for (lyx::support::FileFilterList::size_type i = 0; i < filters.size(); ++i) {
		typedef lyx::support::FileFilterList::Filter::glob_iterator glob_iterator;
		glob_iterator it = filters[i].begin();
		glob_iterator const end = filters[i].end();
		if (it == end)
			continue;

		Gtk::FileFilter filter;
		filter.set_name(filters[i].description());
		for (; it != end; ++it)
			filter.add_pattern(*it);

		fileChooser_.add_filter(filter);
	}

	if (!path.empty())
		fileChooser_.set_current_folder(path);
	if (!suggested.empty())
		fileChooser_.set_current_name(suggested);		
		
	fileChooser_.set_default_response(Gtk::RESPONSE_OK);
	Result result;
	result.first = FileDialog::Chosen;
	if (fileChooser_.run() == Gtk::RESPONSE_OK)
		result.second = fileChooser_.get_filename();
	else
		result.second = string();
	return result;
}
