// -*- C++ -*-
/**
 * \file FileDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon
 */

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include <utility>

#include "commandtags.h"

#include "LString.h"

class LyXView;

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
	 * Constructs a file dialog attached to LyXView \param lv, with
	 * title \param title. If param \a is \const LFUN_SELECT_FILE_SYNC
	 * then a value will be returned immediately upon performing a Select(),
	 * otherwise a callback Dispatch() will be invoked with the filename as
	 * argument, of action \param a.
	 *
	 * Up to two optional extra buttons are allowed for specifying addtional
	 * directories in the navigation.
	 */
	FileDialog(LyXView * lv, string const & title, kb_action a = LFUN_SELECT_FILE_SYNC,
		Button b1 = Button(string(), string()),
		Button b2 = Button(string(), string()));
			

	~FileDialog();

	/**
	 * Choose a file for selection, starting in directory \param path, with the file
	 * selection \param mask. The \param mask string is of the form :
	 *
	 * <regular expression to match> | <description>
	 *
	 * for example, "*.ps | PostScript files (*.ps)".
	 *
	 * FIXME: should support multiple lines of these for different file types.
	 */
	Result const Select(string const & path = string(),
		string const & mask = string(),
		string const & suggested = string());


	/* This *has* to be public because there is no way to specify extern "C" functions
	 * as friends of Private implementation for the xforms implementation ... grr
	 */
	class Private;
	friend class Private;
	Private * private_;

private:
	/// our LyXView
	LyXView * lv_;

	/// the dialog title
	string title_;

	/// success action to perform if not synchronous
	kb_action success_;

};

#endif // FILEDIALOG_H
