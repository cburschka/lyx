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
#include "support/docstring.h"

#include <string>


namespace lyx {

namespace support { class FileFilterList; }


/**
 * \class FileDialog
 * \brief GUI-I definition of file dialog interface
 */
class FileDialog
{
public:
	/// label, directory path button
	typedef std::pair<docstring, docstring> Button;

	/// result type
	enum ResultType {
		Later, /**< modeless chooser, no result */
		Chosen /**< string contains filename */
	};

	/// result return
	typedef std::pair<FileDialog::ResultType, docstring> Result;

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
	FileDialog(docstring const & title, kb_action a = LFUN_SELECT_FILE_SYNC);

	~FileDialog();

	void setButton1(docstring const & label, docstring const & dir);
	void setButton2(docstring const & label, docstring const & dir);

	/// Choose a file for opening, starting in directory \c path.
	Result const open(docstring const & path,
			  support::FileFilterList const & filters,
			  docstring const & suggested);

	/// Choose a directory, starting in directory \c path.
	Result const opendir(docstring const & path = docstring(),
			     docstring const & suggested = docstring());

	/// Choose a file for saving, starting in directory \c  path.
	Result const save(docstring const & path,
			  support::FileFilterList const & filters,
			  docstring const & suggested);

private:
	class Private;
	friend class Private;
	Private * private_;

	/// the dialog title
	docstring title_;

	/// success action to perform if not synchronous
	kb_action success_;
};

} // namespace lyx

#endif // FILEDIALOG_H
