// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#include <config.h>

// 	$Id: filedlg.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: filedlg.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $";
#endif /* lint */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>

#include "lyx_gui_misc.h" // CancelCloseCB
#include "FileInfo.h"
#include "gettext.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef BROKEN_HEADERS
extern "C" int gettimeofday(struct timeval *,struct timezone *);
#define remove(a) unlink(a)      
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include "filetools.h"
#include "filedlg.h"
#include "definitions.h"

static const long SIX_MONTH_SEC = 6L * 30L * 24L * 60L * 60L; // six months, in seconds
static const long ONE_HOUR_SEC = 60L * 60L;

// *** User cache class implementation

// global instance (user cache root)
UserCache lyxUserCache = UserCache(LString(),0,0);


// Add: creates a new user entry
UserCache *UserCache::Add(uid_t ID)
{
	LString pszNewName;
	struct passwd *pEntry;

	// gets user name
	if ((pEntry = getpwuid(ID)))
		pszNewName = pEntry->pw_name;
	else {
		pszNewName = LString() + int(ID); // We don't have int cast to LString
	}

	// adds new node
	return new UserCache(pszNewName, ID, pRoot);
}


UserCache::UserCache(LString const & pszName, uid_t ID, UserCache *pRoot)
{
	// links node
	if (pRoot) {
		this->pRoot = pRoot;
		pNext = pRoot->pNext;
		pRoot->pNext = this;
	} else {
		this->pRoot = this;
		pNext = NULL;
	}

	// stores data
	this->pszName = pszName;
	this->ID = ID;
}


UserCache::~UserCache()
{
	if (pNext) delete pNext;
}


// Find: seeks user name from user ID
LString UserCache::Find(uid_t ID)
{
	if ((!pszName.empty()) && (this->ID == ID)) return pszName; 
	if (pNext) return pNext->Find(ID);

	return pRoot->Add(ID)->pszName;
}


// *** Group cache class implementation

// global instance (group cache root)
GroupCache lyxGroupCache = GroupCache(LString(),0,0);

// Add: creates a new group entry
GroupCache *GroupCache::Add(gid_t ID)
{
	LString pszNewName;
	struct group *pEntry;

	// gets user name
	if ((pEntry = getgrgid(ID))) pszNewName = pEntry->gr_name;
	else {
		pszNewName = LString() + int(ID); // We don't have int cast to LString
	}

	// adds new node
	return new GroupCache(pszNewName, ID, pRoot);
}


GroupCache::GroupCache(LString const & pszName, gid_t ID, GroupCache *pRoot)
{
	// links node
	if (pRoot) {
		this->pRoot = pRoot;
		pNext = pRoot->pNext;
		pRoot->pNext = this;
	} else {
		this->pRoot = this;
		pNext = NULL;
	}

	// stores data
	this->pszName = pszName;
	this->ID = ID;
}


GroupCache::~GroupCache()
{
	if (pNext) delete pNext;
}


// Find: seeks group name from group ID
LString GroupCache::Find(gid_t ID)
{
	if ((!pszName.empty()) && (this->ID == ID)) return pszName; 
	if (pNext) return pNext->Find(ID);

	return pRoot->Add(ID)->pszName;
}

// *** LyXDirEntry internal structure implementation

// ldeCompProc: compares two LyXDirEntry objects content (used for qsort)
int LyXDirEntry::ldeCompProc(const LyXDirEntry *r1, 
			     const LyXDirEntry *r2)
{
	bool r1d = r1->pszName.suffixIs('/'); 
	bool r2d = r2->pszName.suffixIs('/');
	if (r1d && !r2d) return -1;
	if (!r1d && r2d) return 1;
	return strcmp(r1->pszName.c_str(), r2->pszName.c_str());
}

// *** LyXFileDlg class implementation

// static members
FD_FileDlg *LyXFileDlg::pFileDlgForm = NULL;
LyXFileDlg *LyXFileDlg::pCurrentDlg = NULL;


// Reread: updates dialog list to match class directory
void LyXFileDlg::Reread()
{
	int i;
	DIR *pDirectory;
	struct dirent *pDirEntry;
	LString File, Buffer;	
	char szMode[15], szTime[40];
	FileInfo fileInfo;
	
	// Opens directory
	pDirectory = opendir(pszDirectory.c_str());
	if (!pDirectory) {
		WriteFSAlert(_("Warning! Couldn't open directory."), 
			     pszDirectory);
		pszDirectory = GetCWD();
		pDirectory = opendir(pszDirectory.c_str());
	}

	// Clear the present namelist
	if (pCurrentNames) {
		delete [] pCurrentNames;
		pCurrentNames = 0;
	}

	// Updates display
	fl_hide_object(pFileDlgForm->List);
	fl_clear_browser(pFileDlgForm->List);
	fl_set_input(pFileDlgForm->DirBox, pszDirectory.c_str());

	// Splits complete directory name into directories and compute depth
	iDepth = 0;
	LString line, Temp;
	File = pszDirectory;
	if (File != "/") {
		File.split(Temp, '/');
	}
	while (!File.empty() || !Temp.empty()) {
		LString dline = "@b"+line + Temp + '/';		
		fl_add_browser_line(pFileDlgForm->List, dline.c_str());
		File.split(Temp, '/');
		line += ' ';
		iDepth++;
	}

	// Allocate names array
	iNumNames = 0;
	rewinddir(pDirectory);
	while ((readdir(pDirectory))) ++iNumNames;
	pCurrentNames = new LyXDirEntry[iNumNames];

	// Parses all entries of the given subdirectory
	iNumNames = 0;
	time_t curTime = time(NULL);
	rewinddir(pDirectory);
	while ((pDirEntry = readdir(pDirectory))) {

		bool isLink = false, isDir = false;

		// If the pattern doesn't start with a dot, skip hidden files
		if (!pszMask.empty() && pszMask[0] != '.' && 
		    pDirEntry->d_name[0] == '.')
                        continue;

		// Gets filename
		LString fname = pDirEntry->d_name;

		// Under all circumstances, "." and ".." are not wanted
		if (fname == "." || fname == "..")
			continue;

		// gets file status
		File = AddName(pszDirectory, fname);

		fileInfo.newFile(File, true);

		fileInfo.modeString(szMode);
		unsigned int nlink = fileInfo.getNumberOfLinks();
		LString user =	lyxUserCache.Find(fileInfo.getUid());
		LString group = lyxGroupCache.Find(fileInfo.getGid());

		time_t modtime = fileInfo.getModificationTime();
		strcpy(szTime, ctime(&modtime));
		
		if (curTime > fileInfo.getModificationTime() + SIX_MONTH_SEC
		    || curTime < fileInfo.getModificationTime()
		    + ONE_HOUR_SEC) {
			// The file is fairly old or in the future. POSIX says
			// the cutoff is 6 months old. Allow a 1 hour slop
			// factor for what is considered "the future", to
			// allow for NFS server/client clock disagreement.
			// Show the year instead of the time of day.
			strcpy(szTime+10, szTime+19);
			szTime[15] = 0;
		} else
			szTime[16] = 0;
		
		char szHeadBuf[128];		  
		sprintf(szHeadBuf, "%s %u %s %s %s ", szMode, 
			nlink,
			user.c_str(),
			group.c_str(),
			szTime + 4);		
		Buffer = szHeadBuf;

		Buffer += pDirEntry->d_name;
		Buffer += fileInfo.typeIndicator();

		if ((isLink = fileInfo.isLink())) {
		  LString Link;

		  if (LyXReadLink(File,Link)) {
		       Buffer += " -> ";
		       Buffer += Link;

		       // This gives the FileType of the file that
		       // is really pointed too after resolving all
		       // symlinks. This is not necessarily the same
		       // as the type of Link (which could again be a
		       // link). Is that intended?
		       //                              JV 199902
		       fileInfo.newFile(File);
		       Buffer += fileInfo.typeIndicator();
		  }
		}

		// filters files according to pattern and type
		if (fileInfo.isRegular()
		    || fileInfo.isChar()
		    || fileInfo.isBlock()
		    || fileInfo.isFifo()) {
			if (!fname.regexMatch(pszMask))
				continue;
		} else if (!(isDir = fileInfo.isDir()))
			continue;

		// Note pszLsEntry is an LString!
		pCurrentNames[iNumNames].pszLsEntry = Buffer;

		// creates used name
		LString temp = fname;
		if (isDir) temp += '/';
		pCurrentNames[iNumNames].pszName = temp;

		// creates displayed name
		temp = pDirEntry->d_name;
		if (isLink)
			temp += '@';
		else
			temp += fileInfo.typeIndicator();
		
		pCurrentNames[iNumNames++].pszDisplayed = temp;
	}

	closedir(pDirectory);

	// Sort the names
	qsort(pCurrentNames, iNumNames, sizeof(LyXDirEntry), 
	      (int (*)(const void *, const void *))LyXDirEntry::ldeCompProc);

	// Add them to directory box
	for (i = 0; i < iNumNames; ++i) {
		LString temp = line + pCurrentNames[i].pszDisplayed;
		fl_add_browser_line(pFileDlgForm->List, temp.c_str());
	}
	fl_set_browser_topline(pFileDlgForm->List,iDepth);
	fl_show_object(pFileDlgForm->List);
	iLastSel = -1;
}


// SetDirectory: sets dialog current directory
void LyXFileDlg::SetDirectory(LString const & Path)
{
	if (!pszDirectory.empty()) {
		LString TempPath = ExpandPath(Path); // Expand ~/
		TempPath = MakeAbsPath(TempPath, pszDirectory);
		pszDirectory = MakeAbsPath(TempPath);
	} else pszDirectory = MakeAbsPath(Path);
}


// SetMask: sets dialog file mask
void LyXFileDlg::SetMask(LString const & NewMask)
{
	pszMask = NewMask;
	fl_set_input(pFileDlgForm->PatBox, pszMask.c_str());
}


// SetInfoLine: sets dialog information line
void LyXFileDlg::SetInfoLine(LString const & Line)
{
	pszInfoLine = Line;
	fl_set_object_label(pFileDlgForm->FileInfo, pszInfoLine.c_str());
}


LyXFileDlg::LyXFileDlg()
{
	pCurrentNames = 0;
	pszDirectory = MakeAbsPath(LString('.'));
	pszMask = '*';

	// Creates form if necessary. 
	if (!pFileDlgForm) {
		pFileDlgForm = create_form_FileDlg();
		// Set callbacks. This means that we don't need a patch file
		fl_set_object_callback(pFileDlgForm->DirBox,
				       LyXFileDlg::FileDlgCB,0);
		fl_set_object_callback(pFileDlgForm->PatBox,
				       LyXFileDlg::FileDlgCB,1);
		fl_set_object_callback(pFileDlgForm->List,
				       LyXFileDlg::FileDlgCB,2);
		fl_set_object_callback(pFileDlgForm->Filename,
				       LyXFileDlg::FileDlgCB,3);
		fl_set_object_callback(pFileDlgForm->Rescan,
				       LyXFileDlg::FileDlgCB,10);
		fl_set_object_callback(pFileDlgForm->Home,
				       LyXFileDlg::FileDlgCB,11);
		fl_set_object_callback(pFileDlgForm->User1,
				       LyXFileDlg::FileDlgCB,12);
		fl_set_object_callback(pFileDlgForm->User2,
				       LyXFileDlg::FileDlgCB,13);
		
		// Make sure pressing the close box doesn't crash LyX. (RvdK)
		fl_set_form_atclose(pFileDlgForm->FileDlg, CancelCB, NULL);
	   	// Register doubleclick callback
		fl_set_browser_dblclick_callback(pFileDlgForm->List,
						 DoubleClickCB,0);
	}
	fl_hide_object(pFileDlgForm->User1);
	fl_hide_object(pFileDlgForm->User2);
}


LyXFileDlg::~LyXFileDlg()
{
	// frees directory entries
	if (pCurrentNames) {
		delete [] pCurrentNames;
	}
}


// SetButton: sets file selector user button action
void LyXFileDlg::SetButton(int iIndex, LString const & pszName, 
			   LString const & pszPath)
{
	FL_OBJECT *pObject;
	LString *pTemp;

	if (iIndex == 0) {
		pObject = pFileDlgForm->User1;
		pTemp = &pszUserPath1;
	} else if (iIndex == 1) {			
		pObject = pFileDlgForm->User2;
		pTemp = &pszUserPath2;
	} else return;

	if (!pszName.empty() && !pszPath.empty()) {
		fl_set_object_label(pObject, pszName.c_str());
		fl_show_object(pObject);
		*pTemp = pszPath;
	} else {
		fl_hide_object(pObject);
		*pTemp = LString();
	}
}


// GetDirectory: gets last dialog directory
LString LyXFileDlg::GetDirectory() 
{
	if (!pszDirectory.empty())
		return pszDirectory;
	else
		return LString('.');
}


// RunDialog: handle dialog during file selection
bool LyXFileDlg::RunDialog()
{
	force_cancel = false;
	force_ok = false;
	
        // event loop
        while(true) {

                FL_OBJECT * pObject = fl_do_forms();

                if (pObject == pFileDlgForm->Ready) {
			if (HandleOK())
				return true;
		} else if (pObject == pFileDlgForm->Cancel 
			   || force_cancel) 
			return false;
		else if (force_ok)
			return true;
	}
}


// XForms objects callback (static)
void LyXFileDlg::FileDlgCB(FL_OBJECT *, long lArgument)
{
	if (!pCurrentDlg) return;

	switch (lArgument) {

	case 0: // get directory
		pCurrentDlg->SetDirectory(fl_get_input(pFileDlgForm->DirBox));
		pCurrentDlg->Reread();
		break;

	case 1: // get mask
		pCurrentDlg->SetMask(fl_get_input(pFileDlgForm->PatBox));
		pCurrentDlg->Reread();
		break;

	case 2: // list
		pCurrentDlg->HandleListHit();
		break;	

	case 10: // rescan
		pCurrentDlg->SetDirectory(fl_get_input(pFileDlgForm->DirBox));
		pCurrentDlg->SetMask(fl_get_input(pFileDlgForm->PatBox));
		pCurrentDlg->Reread();
		break;

	case 11: // home
		pCurrentDlg->SetDirectory(getEnvPath("HOME"));
		pCurrentDlg->SetMask(fl_get_input(pFileDlgForm->PatBox));
		pCurrentDlg->Reread();
		break;

	case 12: // user button 1
		if (!pCurrentDlg->pszUserPath1.empty()) {
			pCurrentDlg->SetDirectory(pCurrentDlg->pszUserPath1);
			pCurrentDlg->SetMask(fl_get_input(pFileDlgForm->PatBox));
			pCurrentDlg->Reread();
		}
		break;

	case 13: // user button 2
		if (!pCurrentDlg->pszUserPath2.empty()) {
			pCurrentDlg->SetDirectory(pCurrentDlg->pszUserPath2);
			pCurrentDlg->SetMask(fl_get_input(pFileDlgForm->PatBox));
			pCurrentDlg->Reread();
		}
		break;

	}
}


// Handle callback from list
void LyXFileDlg::HandleListHit()
{
	// set info line
	int iSelect = fl_get_browser(pFileDlgForm->List);
	if (iSelect > iDepth)  {
		SetInfoLine(pCurrentNames[iSelect - iDepth - 1].pszLsEntry);
	} else {
		SetInfoLine(LString());
	}
}


// Callback for double click in list
void LyXFileDlg::DoubleClickCB(FL_OBJECT *, long)
{
	if (pCurrentDlg->HandleDoubleClick())
		// Simulate click on OK button
		pCurrentDlg->Force(false);
}


// Handle double click from list
bool LyXFileDlg::HandleDoubleClick()
{
	bool isDir;
	LString pszTemp;
	int iSelect;  

	// set info line
	isDir = true;
	iSelect = fl_get_browser(pFileDlgForm->List);
	if (iSelect > iDepth)  {
		pszTemp = pCurrentNames[iSelect - iDepth - 1].pszName;
		SetInfoLine(pCurrentNames[iSelect - iDepth - 1].pszLsEntry);
		if (!pszTemp.suffixIs('/')) {
			isDir = false;
			fl_set_input(pFileDlgForm->Filename, pszTemp.c_str());
		}
	} else if (iSelect !=0) {
		SetInfoLine(LString());
	} else
		return true;

	// executes action
	if (isDir) {

		int i;
		LString Temp;

		// builds new directory name
		if (iSelect > iDepth) {
			// Directory deeper down
			// First, get directory with trailing /
			Temp = fl_get_input(pFileDlgForm->DirBox);
			if (!Temp.suffixIs('/'))
				Temp += '/';
			Temp += pszTemp;
		} else {
			// Directory higher up
			Temp.clean();
			for (i = 0; i < iSelect; ++i) {
				LString piece = fl_get_browser_line(pFileDlgForm->List, i+1);
				// The '+2' is here to count the '@b' (JMarc)
				Temp += piece.substring(i+2, piece.length()-1);
			}
		}

		// assigns it
		SetDirectory(Temp);
		Reread();
		return false;
	}
	return true;
}


// Handle OK button call
bool LyXFileDlg::HandleOK()
{
	LString pszTemp;

	// mask was changed
	pszTemp = fl_get_input(pFileDlgForm->PatBox);
	if (pszTemp!=pszMask) {
		SetMask(pszTemp);
		Reread();
		return false;
	}

	// directory was changed
	pszTemp = fl_get_input(pFileDlgForm->DirBox);
	if (pszTemp!=pszDirectory) {
		SetDirectory(pszTemp);
		Reread();
		return false;
	}
	
	// Handle return from list
	int select = fl_get_browser(pFileDlgForm->List);
	if (select > iDepth) {
		LString temp = pCurrentNames[select - iDepth - 1].pszName;
		if (!temp.suffixIs('/')) {
			// If user didn't type anything, use browser
			LString name = fl_get_input(pFileDlgForm->Filename);
			if (name.empty()) {
				fl_set_input(pFileDlgForm->Filename, temp.c_str());
			}
			return true;
		}
	}
	
	// Emulate a doubleclick
	return HandleDoubleClick();
}


// Handle Cancel CB from WM close
int LyXFileDlg::CancelCB(FL_FORM *, void *)
{
	// Simulate a click on the cancel button
	pCurrentDlg->Force(true);
  	return FL_IGNORE;
}


// Simulates a click on OK/Cancel
void LyXFileDlg::Force(bool cancel)
{
	if (cancel) {
		force_cancel = true;
		fl_set_button(pFileDlgForm->Cancel, 1);
	} else {
		force_ok = true;
		fl_set_button(pFileDlgForm->Ready, 1);
	}
	// Start timer to break fl_do_forms loop soon
	fl_set_timer(pFileDlgForm->timer, 0.1);
}


// Select: launches dialog and returns selected file
LString LyXFileDlg::Select(LString const & title, LString const & path, 
			   LString const & mask, LString const & suggested)
{
	bool isOk;

	// handles new mask and path
	isOk = true;
	if (!mask.empty()) {
		SetMask(mask);
		isOk = false;
	}
	if (!path.empty()) {
		SetDirectory(path);
		isOk = false;
	}
	if (!isOk) Reread();
	else {
		fl_select_browser_line(pFileDlgForm->List, 1);
		fl_set_browser_topline(pFileDlgForm->List, 1);
	}

	// checks whether dialog can be started
	if (pCurrentDlg) return LString();
	pCurrentDlg = this;

	// runs dialog
	SetInfoLine (LString());
	fl_set_input(pFileDlgForm->Filename, suggested.c_str());
	fl_set_button(pFileDlgForm->Cancel, 0);
	fl_set_button(pFileDlgForm->Ready, 0);
	fl_set_focus_object(pFileDlgForm->FileDlg, pFileDlgForm->Filename);
	fl_deactivate_all_forms();
	fl_show_form(pFileDlgForm->FileDlg, FL_PLACE_MOUSE | FL_FREE_SIZE,
		     FL_FULLBORDER, title.c_str());

	isOk = RunDialog();

	fl_hide_form(pFileDlgForm->FileDlg);
	fl_activate_all_forms();
	pCurrentDlg = NULL;

	// Returns filename or LString() if no valid selection was made
	if (!isOk || !fl_get_input(pFileDlgForm->Filename)[0]) return LString();

	pszFileName = fl_get_input(pFileDlgForm->Filename);

	if (!AbsolutePath(pszFileName)) {
		pszFileName = AddName(fl_get_input(pFileDlgForm->DirBox), 
				      pszFileName);
	}
	return pszFileName;
}
