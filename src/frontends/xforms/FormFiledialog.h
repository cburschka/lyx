// -*- C++ -*-
/**
 * \file FormFiledialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMFILEDIALOG_H
#define FORMFILEDIALOG_H


#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "forms_fwd.h"

#include "frontends/FileDialog.h"

#include <boost/signals/connection.hpp>
#include <boost/signals/trackable.hpp>

#include <vector>

/// DirEntry internal structure definition
class DirEntry {
public:
	///
	string pszName;
	///
	string pszDisplayed;
	///
	string pszLsEntry;
};


//extern "C" {
//	void C_LyXFileDlg_FileDlgCB(FL_OBJECT * ob, long data);
//	void C_LyXFileDlg_DoubleClickCB(FL_OBJECT * ob, long data);
//	int C_LyXFileDlg_CancelCB(FL_FORM *fl, void *xev);
//}


class FD_filedialog;

class FileDialog::Private : public boost::signals::trackable {
public:
	///
	Private();
	///
	~Private();

	/// sets file selector user button action
	void SetButton(int iIndex, string const & pszName = string(),
		       string const & pszPath = string());
	/// gets last dialog directory
	string const GetDirectory() const;
	/// launches dialog and returns selected file
	string const Select(string const & pszTitle = string(),
		       string const & pszPath = string(),
		       string const & pszMask = string(),
		       string const & pszSuggested = string());
	/// XForms objects callback (static)
	static void FileDlgCB(FL_OBJECT *, long);
	/// Callback for double click in list
	static void DoubleClickCB(FL_OBJECT *, long);
	/// Handle Cancel CB from WM close
	static int CancelCB(FL_FORM *, void *);

private:
	/// data
	static FD_filedialog * pFileDlgForm;
	///
	static FileDialog::Private * pCurrentDlg;
	///
	string pszUserPath1;
	///
	string pszUserPath2;
	///
	string pszDirectory;
	///
	string pszMask;
	///
	string pszFileName;
	///
	int iDepth;
	///
	int iLastSel;
	///
	long lLastTime;
	///
	string pszInfoLine;
	///
	typedef std::vector<DirEntry> DirEntries;
	///
	DirEntries direntries;
	///
	bool force_cancel;
	///
	bool force_ok;

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped).
	*/
	void redraw();
	/// updates dialog list to match class directory
	void Reread();
	/// sets dialog current directory
	void SetDirectory(string const & pszPath);
	/// sets dialog file mask
	void SetMask(string const & pszNewMask);
	/// sets dialog information line
	void SetInfoLine(string const & pszLine);
	/// handle dialog during file selection
	bool RunDialog();
	/// Handle callback from list
	void HandleListHit();
	/// Handle double click from list
	bool HandleDoubleClick();
	/// Handle OK button call
	bool HandleOK();
	/// Simulates a click on OK/Cancel
	void Force(bool);
	/// Redraw connection.
	boost::signals::connection r_;
};

#endif // FORMFILEDIALOG_H
