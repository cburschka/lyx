/**
 * \file FormFiledialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon
 */

#include <config.h>

#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <grp.h>
//#include <cstring>
#include <map>
#include <algorithm>

using std::map;
using std::max;
using std::sort;

#include "frontends/Alert.h"
#include "support/FileInfo.h"
#include "support/lyxlib.h"
#include "support/lstrings.h"
#include "gettext.h"
#include "frontends/Dialogs.h"
#include "xforms_helpers.h"


#ifdef HAVE_ERRNO_H
#include <cerrno>
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
# include <ctime>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <ctime>
# endif
#endif

// FIXME: should be autoconfiscated
#ifdef BROKEN_HEADERS
extern "C" int gettimeofday(struct timeval *, struct timezone *);
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/filetools.h"
#include "FormFiledialog.h"

namespace {

// six months, in seconds
long const SIX_MONTH_SEC = 6L * 30L * 24L * 60L * 60L;
//static
long const ONE_HOUR_SEC = 60L * 60L;

extern "C" {
	
	static
	int C_LyXFileDlg_CancelCB(FL_FORM *fl, void *xev)
	{
		return FileDialog::Private::CancelCB(fl, xev);
	}
	
	static
	void C_LyXFileDlg_DoubleClickCB(FL_OBJECT * ob, long data)
	{
		FileDialog::Private::DoubleClickCB(ob, data);
	}

	static
	void C_LyXFileDlg_FileDlgCB(FL_OBJECT * ob, long data)
	{
		FileDialog::Private::FileDlgCB(ob, data);
	}

}

} // namespace anon


// *** User cache class implementation
/// User cache class definition
class UserCache {
public:
	/// seeks user name from group ID
	string const & find(uid_t ID) const {
		Users::const_iterator cit = users.find(ID);
		if (cit == users.end()) {
			add(ID);
			return users[ID];
		}
		return cit->second;
	}
private:
	///
	void add(uid_t ID) const;
	///
	typedef map<uid_t, string> Users;
	///
	mutable Users users;
};


void UserCache::add(uid_t ID) const
{
	string pszNewName;
	struct passwd * pEntry;
	
	// gets user name
	if ((pEntry = getpwuid(ID)))
		pszNewName = pEntry->pw_name;
	else {
		pszNewName = tostr(ID);
	}
	
	// adds new node
	users[ID] = pszNewName;
}	


/// Group cache class definition
class GroupCache {
public:
	/// seeks group name from group ID
	string const & find(gid_t ID) const ;
private:
	///
	void add(gid_t ID) const;
	///
	typedef map<gid_t, string> Groups;
	///
	mutable Groups groups;
};


string const & GroupCache::find(gid_t ID) const
{
	Groups::const_iterator cit = groups.find(ID);
	if (cit == groups.end()) {
		add(ID);
		return groups[ID];
	}
	return cit->second;
}


void GroupCache::add(gid_t ID) const
{
	string pszNewName;
	struct group * pEntry;
	
	// gets user name
	if ((pEntry = getgrgid(ID))) pszNewName = pEntry->gr_name;
	else {
		pszNewName = tostr(ID);
	}
	// adds new node
	groups[ID] = pszNewName;
}


namespace {

// local instances
UserCache lyxUserCache;
GroupCache lyxGroupCache;

} // namespace anon


// compares two LyXDirEntry objects content (used for sort)
class comp_direntry {
public:
	int operator()(DirEntry const & r1,
		       DirEntry const & r2) const ;
};
	int comp_direntry::operator()(DirEntry const & r1,
		       DirEntry const & r2) const {
		bool r1d = suffixIs(r1.pszName, '/');
		bool r2d = suffixIs(r2.pszName, '/');
		if (r1d && !r2d) return 1;
		if (!r1d && r2d) return 0;
		return r1.pszName < r2.pszName;
	}


// *** FileDialog::Private class implementation

// static members
FD_form_filedialog * FileDialog::Private::pFileDlgForm = 0;
FileDialog::Private * FileDialog::Private::pCurrentDlg = 0;


// Reread: updates dialog list to match class directory
void FileDialog::Private::Reread()
{
	// Opens directory
	DIR * pDirectory = ::opendir(pszDirectory.c_str());
	if (!pDirectory) {
		Alert::err_alert(_("Warning! Couldn't open directory."),
			     pszDirectory);
		pszDirectory = lyx::getcwd();
		pDirectory = ::opendir(pszDirectory.c_str());
	}

	// Clear the present namelist
	direntries.clear();

	// Updates display
	fl_hide_object(pFileDlgForm->List);
	fl_clear_browser(pFileDlgForm->List);
	fl_set_input(pFileDlgForm->DirBox, pszDirectory.c_str());

	// Splits complete directory name into directories and compute depth
	iDepth = 0;
	string line, Temp;
	char szMode[15];
	string File = pszDirectory;
	if (File != "/") {
		File = split(File, Temp, '/');
	}
	while (!File.empty() || !Temp.empty()) {
		string dline = "@b"+line + Temp + '/';		
		fl_add_browser_line(pFileDlgForm->List, dline.c_str());
		File = split(File, Temp, '/');
		line += ' ';
		++iDepth;
	}

	// Parses all entries of the given subdirectory
	time_t curTime = time(0);
	rewinddir(pDirectory);
	struct dirent * pDirEntry;
	while ((pDirEntry = readdir(pDirectory))) {
		bool isLink = false, isDir = false;

		// If the pattern doesn't start with a dot, skip hidden files
		if (!pszMask.empty() && pszMask[0] != '.' &&
		    pDirEntry->d_name[0] == '.')
                        continue;

		// Gets filename
		string fname = pDirEntry->d_name;

		// Under all circumstances, "." and ".." are not wanted
		if (fname == "." || fname == "..")
			continue;

		// gets file status
		File = AddName(pszDirectory, fname);

		// FIXME: we don't get this file exists/stattable
		FileInfo fileInfo(File, true);
		fileInfo.modeString(szMode);
		unsigned int nlink = fileInfo.getNumberOfLinks();
		string user = 	lyxUserCache.find(fileInfo.getUid());
		string group = lyxGroupCache.find(fileInfo.getGid());

		time_t modtime = fileInfo.getModificationTime();
		string Time = ctime(&modtime);
		
		if (curTime > modtime + SIX_MONTH_SEC
		    || curTime < modtime + ONE_HOUR_SEC) {
			// The file is fairly old or in the future. POSIX says
			// the cutoff is 6 months old. Allow a 1 hour slop
			// factor for what is considered "the future", to
			// allow for NFS server/client clock disagreement.
			// Show the year instead of the time of day.
			Time.erase(10, 9);
			Time.erase(15, string::npos);
		} else {
			Time.erase(16, string::npos);
		}

		string Buffer = string(szMode) + ' ' +
			tostr(nlink) + ' ' +
			user + ' ' +
			group + ' ' +
			Time.substr(4, string::npos) + ' ';

		Buffer += pDirEntry->d_name;
		Buffer += fileInfo.typeIndicator();

		if ((isLink = fileInfo.isLink())) {
			string Link;

			if (LyXReadLink(File, Link)) {
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
			if (!regexMatch(fname, pszMask))
				continue;
		} else if (!(isDir = fileInfo.isDir()))
			continue;

		DirEntry tmp;

		// Note pszLsEntry is an string!
		tmp.pszLsEntry = Buffer;
		// creates used name
		string temp = fname;
		if (isDir) temp += '/';

		tmp.pszName = temp;
		// creates displayed name
		temp = pDirEntry->d_name;
		if (isLink)
			temp += '@';
		else
			temp += fileInfo.typeIndicator();
		tmp.pszDisplayed = temp;

		direntries.push_back(tmp);
	}

	closedir(pDirectory);

	// Sort the names
	sort(direntries.begin(), direntries.end(), comp_direntry());
	
	// Add them to directory box
	for (DirEntries::const_iterator cit = direntries.begin();
	     cit != direntries.end(); ++cit) {
		string const temp = line + cit->pszDisplayed;
		fl_add_browser_line(pFileDlgForm->List, temp.c_str());
	}
	fl_set_browser_topline(pFileDlgForm->List, iDepth);
	fl_show_object(pFileDlgForm->List);
	iLastSel = -1;
}


// SetDirectory: sets dialog current directory
void FileDialog::Private::SetDirectory(string const & Path)
{
	
	string tmp;

	if (Path.empty()) 
		tmp = lyx::getcwd();
	else
		tmp = MakeAbsPath(ExpandPath(Path), pszDirectory);
 
	// must check the directory exists
	DIR * pDirectory = ::opendir(tmp.c_str());
	if (!pDirectory) {
		Alert::err_alert(_("Warning! Couldn't open directory."), tmp);
	} else {
		::closedir(pDirectory);
		pszDirectory = tmp;
	}
}


// SetMask: sets dialog file mask
void FileDialog::Private::SetMask(string const & NewMask)
{
	pszMask = NewMask;
	fl_set_input(pFileDlgForm->PatBox, pszMask.c_str());
}


// SetInfoLine: sets dialog information line
void FileDialog::Private::SetInfoLine(string const & Line)
{
	pszInfoLine = Line;
	fl_set_object_label(pFileDlgForm->FileInfo, pszInfoLine.c_str());
}


FileDialog::Private::Private()
{
	pszDirectory = MakeAbsPath(string("."));
	pszMask = '*';

	// Creates form if necessary.
	if (!pFileDlgForm) {
		pFileDlgForm = build_filedialog();
		// Set callbacks. This means that we don't need a patch file
		fl_set_object_callback(pFileDlgForm->DirBox,
				       C_LyXFileDlg_FileDlgCB, 0);
		fl_set_object_callback(pFileDlgForm->PatBox,
				       C_LyXFileDlg_FileDlgCB, 1);
		fl_set_object_callback(pFileDlgForm->List,
				       C_LyXFileDlg_FileDlgCB, 2);
		fl_set_object_callback(pFileDlgForm->Filename,
				       C_LyXFileDlg_FileDlgCB, 3);
		fl_set_object_callback(pFileDlgForm->Rescan,
				       C_LyXFileDlg_FileDlgCB, 10);
		fl_set_object_callback(pFileDlgForm->Home,
				       C_LyXFileDlg_FileDlgCB, 11);
		fl_set_object_callback(pFileDlgForm->User1,
				       C_LyXFileDlg_FileDlgCB, 12);
		fl_set_object_callback(pFileDlgForm->User2,
				       C_LyXFileDlg_FileDlgCB, 13);
		
		// Make sure pressing the close box doesn't crash LyX. (RvdK)
		fl_set_form_atclose(pFileDlgForm->form,
				    C_LyXFileDlg_CancelCB, 0);
	   	// Register doubleclick callback
		fl_set_browser_dblclick_callback(pFileDlgForm->List,
						 C_LyXFileDlg_DoubleClickCB,
						 0);
	}
	fl_hide_object(pFileDlgForm->User1);
	fl_hide_object(pFileDlgForm->User2);

	r_ = Dialogs::redrawGUI.connect(SigC::slot(this, &FileDialog::Private::redraw));
}


FileDialog::Private::~Private()
{
	r_.disconnect();
}


void FileDialog::Private::redraw()
{
	if (pFileDlgForm->form && pFileDlgForm->form->visible)
		fl_redraw_form(pFileDlgForm->form);
}


// SetButton: sets file selector user button action
void FileDialog::Private::SetButton(int iIndex, string const & pszName,
			   string const & pszPath)
{
	FL_OBJECT * pObject;
	string * pTemp;

	if (iIndex == 0) {
		pObject = pFileDlgForm->User1;
		pTemp = &pszUserPath1;
	} else if (iIndex == 1) {			
		pObject = pFileDlgForm->User2;
		pTemp = &pszUserPath2;
	} else return;

	if (!pszName.empty()) {
		fl_set_object_label(pObject, idex(pszName.c_str()));
		fl_set_button_shortcut(pObject, scex(pszName.c_str()), 1);
		fl_show_object(pObject);
		*pTemp = pszPath;
	} else {
		fl_hide_object(pObject);
		pTemp->erase();
	}
}


// GetDirectory: gets last dialog directory
string const FileDialog::Private::GetDirectory() const
{
	if (!pszDirectory.empty())
		return pszDirectory;
	else
		return string(".");
}


// RunDialog: handle dialog during file selection
bool FileDialog::Private::RunDialog()
{
	force_cancel = false;
	force_ok = false;
	
        // event loop
        while (true) {
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
void FileDialog::Private::FileDlgCB(FL_OBJECT *, long lArgument)
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
		pCurrentDlg->SetDirectory(GetEnvPath("HOME"));
		pCurrentDlg->SetMask(fl_get_input(pFileDlgForm->PatBox));
		pCurrentDlg->Reread();
		break;

	case 12: // user button 1
		pCurrentDlg->SetDirectory(pCurrentDlg->pszUserPath1);
		pCurrentDlg->SetMask(fl_get_input(pFileDlgForm
						  ->PatBox));
		pCurrentDlg->Reread();
		break;

	case 13: // user button 2
		pCurrentDlg->SetDirectory(pCurrentDlg->pszUserPath2);
		pCurrentDlg->SetMask(fl_get_input(pFileDlgForm
						  ->PatBox));
		pCurrentDlg->Reread();
		break;

	}
}


// Handle callback from list
void FileDialog::Private::HandleListHit()
{
	// set info line
	int const iSelect = fl_get_browser(pFileDlgForm->List);
	if (iSelect > iDepth)  {
		SetInfoLine(direntries[iSelect - iDepth - 1].pszLsEntry);
	} else {
		SetInfoLine(string());
	}
}


// Callback for double click in list
void FileDialog::Private::DoubleClickCB(FL_OBJECT *, long)
{
	if (pCurrentDlg->HandleDoubleClick())
		// Simulate click on OK button
		pCurrentDlg->Force(false);
}


// Handle double click from list
bool FileDialog::Private::HandleDoubleClick()
{
	string pszTemp;

	// set info line
	bool isDir = true;
	int const iSelect = fl_get_browser(pFileDlgForm->List);
	if (iSelect > iDepth)  {
		pszTemp = direntries[iSelect - iDepth - 1].pszName;
		SetInfoLine(direntries[iSelect - iDepth - 1].pszLsEntry);
		if (!suffixIs(pszTemp, '/')) {
			isDir = false;
			fl_set_input(pFileDlgForm->Filename, pszTemp.c_str());
		}
	} else if (iSelect != 0) {
		SetInfoLine(string());
	} else
		return true;

	// executes action
	if (isDir) {
		string Temp;

		// builds new directory name
		if (iSelect > iDepth) {
			// Directory deeper down
			// First, get directory with trailing /
			Temp = fl_get_input(pFileDlgForm->DirBox);
			if (!suffixIs(Temp, '/'))
				Temp += '/';
			Temp += pszTemp;
		} else {
			// Directory higher up
			Temp.erase();
			for (int i = 0; i < iSelect; ++i) {
				string piece = fl_get_browser_line(pFileDlgForm->List, i+1);
				// The '+2' is here to count the '@b' (JMarc)
				Temp += piece.substr(i + 2);
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
bool FileDialog::Private::HandleOK()
{
	// mask was changed
	string pszTemp = fl_get_input(pFileDlgForm->PatBox);
	if (pszTemp != pszMask) {
		SetMask(pszTemp);
		Reread();
		return false;
	}

	// directory was changed
	pszTemp = fl_get_input(pFileDlgForm->DirBox);
	if (pszTemp!= pszDirectory) {
		SetDirectory(pszTemp);
		Reread();
		return false;
	}
	
	// Handle return from list
	int const select = fl_get_browser(pFileDlgForm->List);
	if (select > iDepth) {
		string const temp = direntries[select - iDepth - 1].pszName;
		if (!suffixIs(temp, '/')) {
			// If user didn't type anything, use browser
			string const name =
				fl_get_input(pFileDlgForm->Filename);
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
int FileDialog::Private::CancelCB(FL_FORM *, void *)
{
	// Simulate a click on the cancel button
	pCurrentDlg->Force(true);
  	return FL_IGNORE;
}


// Simulates a click on OK/Cancel
void FileDialog::Private::Force(bool cancel)
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
string const FileDialog::Private::Select(string const & title,
					 string const & path,
					 string const & mask,
					 string const & suggested)
{
	// handles new mask and path
	bool isOk = true;
	if (!mask.empty()) {
		SetMask(mask);
		isOk = false;
	}
	if (!path.empty()) {
		SetDirectory(path);
		isOk = false;
	}
	if (!isOk) Reread();

	// highlight the suggested file in the browser, if it exists.
	int sel = 0;
	string const filename = OnlyFilename(suggested);
	if (!filename.empty()) {
		for (int i = 0;
		     i < fl_get_browser_maxline(pFileDlgForm->List); ++i) {
			string s =
				fl_get_browser_line(pFileDlgForm->List, i + 1);
			s = strip(frontStrip(s));
			if (s == filename) {
				sel = i + 1;
				break;
			}
		}
	}
	
	if (sel != 0) fl_select_browser_line(pFileDlgForm->List, sel);
	int const top = max(sel - 5, 1);
	fl_set_browser_topline(pFileDlgForm->List, top);

	// checks whether dialog can be started
	if (pCurrentDlg) return string();
	pCurrentDlg = this;

	// runs dialog
	SetInfoLine(string());
	fl_set_input(pFileDlgForm->Filename, suggested.c_str());
	fl_set_button(pFileDlgForm->Cancel, 0);
	fl_set_button(pFileDlgForm->Ready, 0);
	fl_set_focus_object(pFileDlgForm->form, pFileDlgForm->Filename);
	fl_deactivate_all_forms();
	fl_show_form(pFileDlgForm->form,
		     FL_PLACE_MOUSE | FL_FREE_SIZE, 0,
		     title.c_str());

	isOk = RunDialog();
	
	fl_hide_form(pFileDlgForm->form);
	fl_activate_all_forms();
	pCurrentDlg = 0;

	// Returns filename or string() if no valid selection was made
	if (!isOk || !fl_get_input(pFileDlgForm->Filename)[0]) return string();

	pszFileName = fl_get_input(pFileDlgForm->Filename);

	if (!AbsolutePath(pszFileName)) {
		pszFileName = AddName(fl_get_input(pFileDlgForm->DirBox),
				      pszFileName);
	}
	return pszFileName;
}
