// -*- C++ -*-
/**
 * \file FileDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "lfuns.h"

#include <utility>
#include <string>


namespace lyx {
namespace support {

class FileFilterList;

} // namespace support
} // namespace lyx



/**
 * \class FileDialog
 * \brief GUI-I definition of file dialog interface
 */
class FileDialog
{
public:
	/// label, directory path button
	typedef std::pair<std::string, std::string> Button;

	/// result type
	enum ResultType {
		Later, /**< modeless chooser, no result */
		Chosen /**< string contains filename */
	};

	/// result return
	typedef std::pair<FileDialog::ResultType, std::string> Result;

	/**
	 * Constructs a file dialog with title \param title.
	 * If \param a is \const LFUN_SELECT_FILE_SYNC then a value
	 * will be returned immediately upon performing a open(),
	 * otherwise a callback Dispatch() will be invoked with the filename as
	 * argument, of action \param a.
	 *
	 * Up to two optional extra buttons are allowed for specifying
	 * additional directories in the navigation (an empty
	 * directory is interpreted as getcwd())
	 */
	FileDialog(std::string const & title,
		   kb_action a = LFUN_SELECT_FILE_SYNC,
		   Button b1 = Button(std::string(), std::string()),
		   Button b2 = Button(std::string(), std::string()));


	~FileDialog();

	/// Choose a file for opening, starting in directory \c path.
	Result const open(std::string const & path,
			  lyx::support::FileFilterList const & filters,
			  std::string const & suggested);

	/// Choose a directory, starting in directory \c path.
	Result const opendir(std::string const & path = std::string(),
			     std::string const & suggested = std::string());

	/// Choose a file for saving, starting in directory \c  path.
	Result const save(std::string const & path,
			  lyx::support::FileFilterList const & filters,
			  std::string const & suggested);

	/* This *has* to be public because there is no way to specify
	 * extern "C" functions as friends of Private implementation for
	 * the xforms implementation ... grr
	 */
	class Private;
	friend class Private;
	Private * private_;

private:
	/// the dialog title
	std::string title_;

	/// success action to perform if not synchronous
	kb_action success_;

};

#endif // FILEDIALOG_H
