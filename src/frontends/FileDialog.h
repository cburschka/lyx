// -*- C++ -*-
/**
 * \file FileDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FILEDIALOG_H
#define FILEDIALOG_H


#include <utility>

#include "lfuns.h"

#include "LString.h"

/**
 * \class FileDialog
 * \brief GUI-I definition of file dialog interface
 */
class FileDialog
{
public:
	/// label, directory path button
	typedef std::pair<string, string> Button;

	/// result type
	enum ResultType {
		Later, /**< modeless chooser, no result */
		Chosen /**< string contains filename */
	};

	/// result return
	typedef std::pair<FileDialog::ResultType, string> Result;

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
	FileDialog(string const & title,
		   kb_action a = LFUN_SELECT_FILE_SYNC,
		   Button b1 = Button(string(), string()),
		   Button b2 = Button(string(), string()));


	~FileDialog();

	/**
	 * Choose a file for opening, starting in directory \param
	 * path, with the file selection \param mask. The \param mask
	 * string is of the form :
	 *
	 * <glob to match> | <description>
	 *
	 * for example, "*.ps | PostScript files (*.ps)".
	 *
	 * FIXME: should support multiple lines of these for different file types.
	 */
	Result const open(string const & path = string(),
		string const & mask = string(),
		string const & suggested = string());

	/**
	 * Choose a directory, starting in directory \param
	 * path.
	 */
	Result const opendir(string const & path = string(),
		string const & suggested = string());

	/**
	 * Choose a file for saving, starting in directory \param
	 * path, with the file selection \param mask. The \param mask
	 * string is of the form :
	 *
	 * <glob to match> | <description>
	 *
	 * for example, "*.ps | PostScript files (*.ps)".
	 */
	Result const save(string const & path = string(),
		string const & mask = string(),
		string const & suggested = string());


	/* This *has* to be public because there is no way to specify extern "C" functions
	 * as friends of Private implementation for the xforms implementation ... grr
	 */
	class Private;
	friend class Private;
	Private * private_;

private:
	/// the dialog title
	string title_;

	/// success action to perform if not synchronous
	kb_action success_;

};

#endif // FILEDIALOG_H
