/**
 * \file FormFiledialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <grp.h>
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
#include "forms_gettext.h"

#include <boost/bind.hpp>

//#ifdef HAVE_ERRNO_H
//#include <cerrno>
//#endif

#if HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/filetools.h"
#include "FormFiledialog.h"
#include "forms/form_filedialog.h"
#include FORMS_H_LOCATION

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
	struct passwd const * entry = getpwuid(ID);
	users[ID] = entry ? entry->pw_name : tostr(ID);
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
	struct group const * entry = getgrgid(ID);
	groups[ID] = entry ? entry->gr_name : tostr(ID);
}

// local instances
UserCache lyxUserCache;
GroupCache lyxGroupCache;

// compares two LyXDirEntry objects content (used for sort)
class comp_direntry {
public:
	bool operator()(DirEntry const & r1, DirEntry const & r2) const
	{
		bool const r1d = suffixIs(r1.name_, '/');
		bool const r2d = suffixIs(r2.name_, '/');
		if (r1d && !r2d)
			return true;
		if (!r1d && r2d)
			return false;
		return r1.name_ < r2.name_;
	}
};


} // namespace anon



// *** FileDialog::Private class implementation

// static members
FD_filedialog * FileDialog::Private::file_dlg_form_ = 0;
FileDialog::Private * FileDialog::Private::current_dlg_ = 0;


// Reread: updates dialog list to match class directory
void FileDialog::Private::Reread()
{
	// Opens directory
	DIR * dir = ::opendir(directory_.c_str());
	if (!dir) {
		Alert::err_alert(_("Warning! Couldn't open directory."),
			directory_);
		directory_ = lyx::getcwd();
		dir = ::opendir(directory_.c_str());
	}

	// Clear the present namelist
	dir_entries_.clear();

	// Updates display
	fl_hide_object(file_dlg_form_->List);
	fl_clear_browser(file_dlg_form_->List);
	fl_set_input(file_dlg_form_->DirBox, directory_.c_str());

	// Splits complete directory name into directories and compute depth
	depth_ = 0;
	string line, Temp;
	string mode;
	string File = directory_;
	if (File != "/") {
		File = split(File, Temp, '/');
	}
	while (!File.empty() || !Temp.empty()) {
		string dline = "@b" + line + Temp + '/';
		fl_add_browser_line(file_dlg_form_->List, dline.c_str());
		File = split(File, Temp, '/');
		line += ' ';
		++depth_;
	}

	// Parses all entries of the given subdirectory
	time_t curTime = time(0);
	rewinddir(dir);
	while (dirent * entry = readdir(dir)) {
		bool isLink = false, isDir = false;

		// If the pattern doesn't start with a dot, skip hidden files
		if (!mask_.empty() && mask_[0] != '.' &&
		    entry->d_name[0] == '.')
			continue;

		// Gets filename
		string fname = entry->d_name;

		// Under all circumstances, "." and ".." are not wanted
		if (fname == "." || fname == "..")
			continue;

		// gets file status
		File = AddName(directory_, fname);

		FileInfo fileInfo(File, true);

		// can this really happen?
		if (!fileInfo.isOK())
			continue;

		mode = fileInfo.modeString();
		unsigned int const nlink = fileInfo.getNumberOfLinks();
		string const user  = lyxUserCache.find(fileInfo.getUid());
		string const group = lyxGroupCache.find(fileInfo.getGid());

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

		string buffer = mode + ' ' +
			tostr(nlink) + ' ' +
			user + ' ' +
			group + ' ' +
			Time.substr(4, string::npos) + ' ';

		buffer += entry->d_name;
		buffer += fileInfo.typeIndicator();

		isLink = fileInfo.isLink();
		if (isLink) {
			string Link;

			if (LyXReadLink(File, Link)) {
				buffer += " -> ";
				buffer += Link;

				// This gives the FileType of the file that
				// is really pointed too after resolving all
				// symlinks. This is not necessarily the same
				// as the type of Link (which could again be a
				// link). Is that intended?
				//                              JV 199902
				fileInfo.newFile(File);
				if (fileInfo.isOK())
					buffer += fileInfo.typeIndicator();
				else
					continue;
			}
		}

		// filters files according to pattern and type
		if (fileInfo.isRegular()
		    || fileInfo.isChar()
		    || fileInfo.isBlock()
		    || fileInfo.isFifo()) {
			if (!regexMatch(fname, mask_))
				continue;
		} else if (!(isDir = fileInfo.isDir()))
			continue;

		DirEntry tmp;

		// Note ls_entry_ is an string!
		tmp.ls_entry_ = buffer;
		// creates used name
		string temp = fname;
		if (isDir)
			temp += '/';

		tmp.name_ = temp;
		// creates displayed name
		temp = entry->d_name;
		if (isLink)
			temp += '@';
		else
			temp += fileInfo.typeIndicator();
		tmp.displayed_ = temp;

		dir_entries_.push_back(tmp);
	}

	closedir(dir);

	// Sort the names
	sort(dir_entries_.begin(), dir_entries_.end(), comp_direntry());

	// Add them to directory box
	for (DirEntries::const_iterator cit = dir_entries_.begin();
	     cit != dir_entries_.end(); ++cit) {
		string const temp = line + cit->displayed_;
		fl_add_browser_line(file_dlg_form_->List, temp.c_str());
	}
	fl_set_browser_topline(file_dlg_form_->List, depth_);
	fl_show_object(file_dlg_form_->List);
	last_sel_ = -1;
}


// SetDirectory: sets dialog current directory
void FileDialog::Private::SetDirectory(string const & path)
{
	string tmp;
	if (path.empty())
		tmp = lyx::getcwd();
	else
		tmp = MakeAbsPath(ExpandPath(path), directory_);

	// must check the directory exists
	DIR * dir = ::opendir(tmp.c_str());
	if (!dir) {
		Alert::err_alert(_("Warning! Couldn't open directory."), tmp);
	} else {
		::closedir(dir);
		directory_ = tmp;
	}
}


// SetMask: sets dialog file mask
void FileDialog::Private::SetMask(string const & newmask)
{
	mask_ = newmask;
	fl_set_input(file_dlg_form_->PatBox, mask_.c_str());
}


// SetInfoLine: sets dialog information line
void FileDialog::Private::SetInfoLine(string const & line)
{
	info_line_ = line;
	fl_set_object_label(file_dlg_form_->FileInfo, info_line_.c_str());
}


FileDialog::Private::Private(Dialogs & dia)
{
	directory_ = MakeAbsPath(string("."));
	mask_ = '*';

	// Creates form if necessary.
	if (!file_dlg_form_) {
		file_dlg_form_ = build_filedialog(this);
		// Set callbacks. This means that we don't need a patch file
		fl_set_object_callback(file_dlg_form_->DirBox,
				       C_LyXFileDlg_FileDlgCB, 0);
		fl_set_object_callback(file_dlg_form_->PatBox,
				       C_LyXFileDlg_FileDlgCB, 1);
		fl_set_object_callback(file_dlg_form_->List,
				       C_LyXFileDlg_FileDlgCB, 2);
		fl_set_object_callback(file_dlg_form_->Filename,
				       C_LyXFileDlg_FileDlgCB, 3);
		fl_set_object_callback(file_dlg_form_->Rescan,
				       C_LyXFileDlg_FileDlgCB, 10);
		fl_set_object_callback(file_dlg_form_->Home,
				       C_LyXFileDlg_FileDlgCB, 11);
		fl_set_object_callback(file_dlg_form_->User1,
				       C_LyXFileDlg_FileDlgCB, 12);
		fl_set_object_callback(file_dlg_form_->User2,
				       C_LyXFileDlg_FileDlgCB, 13);

		// Make sure pressing the close box doesn't crash LyX. (RvdK)
		fl_set_form_atclose(file_dlg_form_->form,
				    C_LyXFileDlg_CancelCB, 0);
		// Register doubleclick callback
		fl_set_browser_dblclick_callback(file_dlg_form_->List,
						 C_LyXFileDlg_DoubleClickCB,
						 0);
	}
	fl_hide_object(file_dlg_form_->User1);
	fl_hide_object(file_dlg_form_->User2);

	r_ = dia.redrawGUI.connect(boost::bind(&FileDialog::Private::redraw, this));
}


FileDialog::Private::~Private()
{
	r_.disconnect();
}


void FileDialog::Private::redraw()
{
	if (file_dlg_form_->form && file_dlg_form_->form->visible)
		fl_redraw_form(file_dlg_form_->form);
}


// SetButton: sets file selector user button action
void FileDialog::Private::SetButton(int index, string const & name,
			   string const & path)
{
	FL_OBJECT * ob;
	string * tmp;

	if (index == 0) {
		ob = file_dlg_form_->User1;
		tmp = &user_path1_;
	} else if (index == 1) {
		ob = file_dlg_form_->User2;
		tmp = &user_path2_;
	} else {
		return;
	}

	if (!name.empty()) {
		fl_set_object_label(ob, idex(name.c_str()));
		fl_set_button_shortcut(ob, scex(name.c_str()), 1);
		fl_show_object(ob);
		*tmp = path;
	} else {
		fl_hide_object(ob);
		tmp->erase();
	}
}


// GetDirectory: gets last dialog directory
string const FileDialog::Private::GetDirectory() const
{
	if (!directory_.empty())
		return directory_;
	else
		return string(".");
}

namespace {
	bool x_sync_kludge(bool ret)
	{
		XSync(fl_get_display(), false);
		return ret;
	}
} // namespace anon

// RunDialog: handle dialog during file selection
bool FileDialog::Private::RunDialog()
{
	force_cancel_ = false;
	force_ok_ = false;

	// event loop
	while (true) {
		FL_OBJECT * ob = fl_do_forms();

		if (ob == file_dlg_form_->Ready) {
			if (HandleOK())
				return x_sync_kludge(true);

		} else if (ob == file_dlg_form_->Cancel || force_cancel_)
			return x_sync_kludge(false);

		else if (force_ok_)
			return x_sync_kludge(true);
	}
}


// XForms objects callback (static)
void FileDialog::Private::FileDlgCB(FL_OBJECT *, long arg)
{
	if (!current_dlg_)
		return;

	switch (arg) {

	case 0: // get directory
		current_dlg_->SetDirectory(fl_get_input(file_dlg_form_->DirBox));
		current_dlg_->Reread();
		break;

	case 1: // get mask
		current_dlg_->SetMask(fl_get_input(file_dlg_form_->PatBox));
		current_dlg_->Reread();
		break;

	case 2: // list
		current_dlg_->HandleListHit();
		break;

	case 10: // rescan
		current_dlg_->SetDirectory(fl_get_input(file_dlg_form_->DirBox));
		current_dlg_->SetMask(fl_get_input(file_dlg_form_->PatBox));
		current_dlg_->Reread();
		break;

	case 11: // home
		current_dlg_->SetDirectory(GetEnvPath("HOME"));
		current_dlg_->SetMask(fl_get_input(file_dlg_form_->PatBox));
		current_dlg_->Reread();
		break;

	case 12: // user button 1
		current_dlg_->SetDirectory(current_dlg_->user_path1_);
		current_dlg_->SetMask(fl_get_input(file_dlg_form_->PatBox));
		current_dlg_->Reread();
		break;

	case 13: // user button 2
		current_dlg_->SetDirectory(current_dlg_->user_path2_);
		current_dlg_->SetMask(fl_get_input(file_dlg_form_->PatBox));
		current_dlg_->Reread();
		break;

	}
}


// Handle callback from list
void FileDialog::Private::HandleListHit()
{
	// set info line
	int const select_ = fl_get_browser(file_dlg_form_->List);
	if (select_ > depth_)
		SetInfoLine(dir_entries_[select_ - depth_ - 1].ls_entry_);
	else
		SetInfoLine(string());
}


// Callback for double click in list
void FileDialog::Private::DoubleClickCB(FL_OBJECT *, long)
{
	// Simulate click on OK button
	if (current_dlg_->HandleDoubleClick())
		current_dlg_->Force(false);
}


// Handle double click from list
bool FileDialog::Private::HandleDoubleClick()
{
	string tmp;

	// set info line
	bool isDir = true;
	int const select_ = fl_get_browser(file_dlg_form_->List);
	if (select_ > depth_) {
		tmp = dir_entries_[select_ - depth_ - 1].name_;
		SetInfoLine(dir_entries_[select_ - depth_ - 1].ls_entry_);
		if (!suffixIs(tmp, '/')) {
			isDir = false;
			fl_set_input(file_dlg_form_->Filename, tmp.c_str());
		}
	} else if (select_ != 0) {
		SetInfoLine(string());
	} else
		return true;

	// executes action
	if (isDir) {
		string Temp;

		// builds new directory name
		if (select_ > depth_) {
			// Directory deeper down
			// First, get directory with trailing /
			Temp = fl_get_input(file_dlg_form_->DirBox);
			if (!suffixIs(Temp, '/'))
				Temp += '/';
			Temp += tmp;
		} else {
			// Directory higher up
			Temp.erase();
			for (int i = 0; i < select_; ++i) {
				string piece = fl_get_browser_line(file_dlg_form_->List, i+1);
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
	string tmp = fl_get_input(file_dlg_form_->PatBox);
	if (tmp != mask_) {
		SetMask(tmp);
		Reread();
		return false;
	}

	// directory was changed
	tmp = fl_get_input(file_dlg_form_->DirBox);
	if (tmp != directory_) {
		SetDirectory(tmp);
		Reread();
		return false;
	}

	// Handle return from list
	int const select = fl_get_browser(file_dlg_form_->List);
	if (select > depth_) {
		string const temp = dir_entries_[select - depth_ - 1].name_;
		if (!suffixIs(temp, '/')) {
			// If user didn't type anything, use browser
			string const name = fl_get_input(file_dlg_form_->Filename);
			if (name.empty())
				fl_set_input(file_dlg_form_->Filename, temp.c_str());
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
	current_dlg_->Force(true);
	return FL_IGNORE;
}


// Simulates a click on OK/Cancel
void FileDialog::Private::Force(bool cancel)
{
	if (cancel) {
		force_cancel_ = true;
		fl_set_button(file_dlg_form_->Cancel, 1);
	} else {
		force_ok_ = true;
		fl_set_button(file_dlg_form_->Ready, 1);
	}
	// Start timer to break fl_do_forms loop soon
	fl_set_timer(file_dlg_form_->timer, 0.1);
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
	if (!isOk)
		Reread();

	// highlight the suggested file in the browser, if it exists.
	int sel = 0;
	string const filename = OnlyFilename(suggested);
	if (!filename.empty()) {
		for (int i = 0; i < fl_get_browser_maxline(file_dlg_form_->List); ++i) {
			string s = fl_get_browser_line(file_dlg_form_->List, i + 1);
			s = strip(frontStrip(s));
			if (s == filename) {
				sel = i + 1;
				break;
			}
		}
	}

	if (sel != 0)
		fl_select_browser_line(file_dlg_form_->List, sel);
	int const top = max(sel - 5, 1);
	fl_set_browser_topline(file_dlg_form_->List, top);

	// checks whether dialog can be started
	if (current_dlg_)
		return string();
	current_dlg_ = this;

	// runs dialog
	SetInfoLine(string());
	fl_set_input(file_dlg_form_->Filename, suggested.c_str());
	fl_set_button(file_dlg_form_->Cancel, 0);
	fl_set_button(file_dlg_form_->Ready, 0);
	fl_set_focus_object(file_dlg_form_->form, file_dlg_form_->Filename);
	fl_deactivate_all_forms();
	fl_show_form(file_dlg_form_->form,
		     FL_PLACE_MOUSE | FL_FREE_SIZE, 0,
		     title.c_str());

	isOk = RunDialog();

	fl_hide_form(file_dlg_form_->form);
	fl_activate_all_forms();
	current_dlg_ = 0;

	// Returns filename or string() if no valid selection was made
	if (!isOk || !fl_get_input(file_dlg_form_->Filename)[0])
		return string();

	file_name_ = fl_get_input(file_dlg_form_->Filename);

	if (!AbsolutePath(file_name_))
		file_name_ = AddName(fl_get_input(file_dlg_form_->DirBox), file_name_);
	return file_name_;
}
