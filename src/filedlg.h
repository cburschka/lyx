// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef FILEDLG_H
#define FILEDLG_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include <sigc++/signal_system.h>

#include "LString.h"
#include FORMS_H_LOCATION
#include "form1.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::Object;
using SigC::Connection;
#endif

/// LyXDirEntry internal structure definition
class LyXDirEntry {
public:
	///
	string pszName;
	///
	string pszDisplayed;
	///
	string pszLsEntry;
};


/// FileDlg class definition
class LyXFileDlg : public Object
{
public:
	///
	LyXFileDlg();
	///
	~LyXFileDlg();

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
	static FD_FileDlg * pFileDlgForm;
	///
	static LyXFileDlg * pCurrentDlg;
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
	typedef std::vector<LyXDirEntry> DirEntries;
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
	Connection r_;
};

#endif
