// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#ifndef FILEDLG_H
#define FILEDLG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

// necessary for xForms related stuff
#include FORMS_H_LOCATION
#include "form1.h"


/// User cache class definition
class UserCache
{
public:
	///
	UserCache(string const & pszName = string(), uid_t ID = 0, 
		  UserCache *pRoot = 0);
	///
	~UserCache();
	// interface
	/// seeks user name from user ID
	string Find(uid_t ID);
private:
	///
	uid_t ID;
	///
	string pszName;
	///
	UserCache *pNext, *pRoot;
	// internal methods
	/// creates a new user entry
	UserCache *Add(uid_t ID);
};
extern UserCache lyxUserCache;


/// Group cache class definition
class GroupCache
{
public:
	///
	GroupCache(string const & pszName = string(), gid_t ID = 0,
		   GroupCache *pRoot = 0);
	///
	~GroupCache();
	// interface
	/// seeks group name from group ID
	string Find(gid_t ID);

private:
	///
	gid_t ID;
	///
	string pszName;
	///
	GroupCache *pNext, *pRoot;
	// internal methods
	/// creates a new group entry
	GroupCache *Add(gid_t ID);
};
extern GroupCache lyxGroupCache;


/// LyXDirEntry internal structure definition
class LyXDirEntry
{
private:
	friend class LyXFileDlg;    
	string pszName;
	string pszDisplayed;
	string pszLsEntry;
	/// compares two LyXDirEntry objects content (used by qsort)
	static int ldeCompProc(const LyXDirEntry *r1, const LyXDirEntry *r2);
	LyXDirEntry() {};
};


/// FileDlg class definition
class LyXFileDlg 
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
	string GetDirectory();
	/// launches dialog and returns selected file
	string Select(string const & pszTitle = string(),
		       string const & pszPath = string(),
		       string const & pszMask = string(), 
		       string const & pszSuggested = string());
	/// XForms objects callback (static)
	static void FileDlgCB(FL_OBJECT *, long);
	/// Callback for double click in list
	static void DoubleClickCB(FL_OBJECT *, long);
private:
	// data
	static FD_FileDlg *pFileDlgForm;
	static LyXFileDlg *pCurrentDlg;
	string pszUserPath1;
	string pszUserPath2;
	string pszDirectory;
	string pszMask;
	string pszFileName;
	int iDepth;
	int iLastSel;
	long lLastTime;
	string pszInfoLine;
	LyXDirEntry *pCurrentNames;
	int iNumNames;

	bool force_cancel;
	bool force_ok;

	// internal functions
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
	/// Handle Cancel CB from WM close
	static int CancelCB(FL_FORM *, void *);
	/// Simulates a click on OK/Cancel
	void Force(bool);
};

#endif
