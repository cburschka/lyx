// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 1996 Matthias Ettrich
*           and the LyX Team.
*
*======================================================*/

#ifndef __FILEDLG_H__
#define __FILEDLG_H__

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
	UserCache(LString const & pszName = LString(), uid_t ID = 0, 
		  UserCache *pRoot = NULL);
	///
	~UserCache();
	// interface
	/// seeks user name from user ID
	LString Find(uid_t ID);
private:
	///
	uid_t ID;
	///
	LString pszName;
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
	GroupCache(LString const & pszName = LString(), gid_t ID = 0,
		   GroupCache *pRoot = NULL);
	///
	~GroupCache();
	// interface
	/// seeks group name from group ID
	LString Find(gid_t ID);

private:
	///
	gid_t ID;
	///
	LString pszName;
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
	LString pszName;
	LString pszDisplayed;
	LString pszLsEntry;
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
	void SetButton(int iIndex, LString const & pszName = LString(), 
		       LString const & pszPath = LString());
	/// gets last dialog directory
	LString GetDirectory();
	/// launches dialog and returns selected file
	LString Select(LString const & pszTitle = LString(),
		       LString const & pszPath = LString(),
		       LString const & pszMask = LString(), 
		       LString const & pszSuggested = LString());
	/// XForms objects callback (static)
	static void FileDlgCB(FL_OBJECT *, long);
	/// Callback for double click in list
	static void DoubleClickCB(FL_OBJECT *, long);
private:
	// data
	static FD_FileDlg *pFileDlgForm;
	static LyXFileDlg *pCurrentDlg;
	LString pszUserPath1;
	LString pszUserPath2;
	LString pszDirectory;
	LString pszMask;
	LString pszFileName;
	int iDepth;
	int iLastSel;
	long lLastTime;
	LString pszInfoLine;
	LyXDirEntry *pCurrentNames;
	int iNumNames;

	bool force_cancel;
	bool force_ok;

	// internal functions
	/// updates dialog list to match class directory
	void Reread();
	/// sets dialog current directory
	void SetDirectory(LString const & pszPath);
	/// sets dialog file mask
	void SetMask(LString const & pszNewMask);
	/// sets dialog information line
	void SetInfoLine(LString const & pszLine);
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
