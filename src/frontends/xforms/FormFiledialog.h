// -*- C++ -*-
/**
 * \file FormFiledialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMFILEDIALOG_H
#define FORMFILEDIALOG_H

#include "forms_fwd.h"

#include "frontends/FileDialog.h"

#include <boost/signals/connection.hpp>
#include <boost/signals/trackable.hpp>

#include <vector>

class Dialogs;

/// DirEntry internal structure definition
class DirEntry {
public:
	///
	std::string name_;
	///
	std::string displayed_;
	///
	std::string ls_entry_;
};


//extern "C" {
//	void C_LyXFileDlg_FileDlgCB(FL_OBJECT * ob, long data);
//	void C_LyXFileDlg_DoubleClickCB(FL_OBJECT * ob, long data);
//	int C_LyXFileDlg_CancelCB(FL_FORM *fl, void *xev);
//}


class FD_filedialog;

namespace lyx {
namespace support {

class FileFilterList;

} // namespace support
} // namespace lyx


class FileDialog::Private : public boost::signals::trackable {
public:
	///
	Private();
	///
	~Private();

	/// sets file selector user button action
	void SetButton(int iIndex, std::string const & pszName = std::string(),
		       std::string const & pszPath = std::string());
	/// gets last dialog directory
	std::string const GetDirectory() const;
	/// launches dialog and returns selected file
	std::string const Select(std::string const & pszTitle,
				 std::string const & pszPath,
				 lyx::support::FileFilterList const & filters,
				 std::string const & pszSuggested);
	/// launches dialog and returns selected directory
	std::string const SelectDir(std::string const & pszTitle = std::string(),
		       std::string const & pszPath = std::string(),
		       std::string const & pszSuggested = std::string());
	/// XForms objects callback (static)
	static void FileDlgCB(FL_OBJECT *, long);
	/// Callback for double click in list
	static void DoubleClickCB(FL_OBJECT *, long);
	/// Handle Cancel CB from WM close
	static int CancelCB(FL_FORM *, void *);

private:
	/// data
	static FD_filedialog * file_dlg_form_;
	///
	static FileDialog::Private * current_dlg_;
	///
	static int minw_;
	///
	static int minh_;
	///
	std::string user_path1_;
	///
	std::string user_path2_;
	///
	std::string directory_;
	///
	std::string mask_;
	///
	std::string file_name_;
	///
	int depth_;
	///
	int last_sel_;
	///
	long last_time_;
	///
	std::string info_line_;
	///
	typedef std::vector<DirEntry> DirEntries;
	///
	DirEntries dir_entries_;
	///
	bool force_cancel_;
	///
	bool force_ok_;

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped).
	*/
	void redraw();
	/// updates dialog list to match class directory
	void Reread();
	/// sets dialog current directory
	void SetDirectory(std::string const & pszPath);
	/// sets dialog file mask
	void SetFilters(std::string const & filters);
	void SetFilters(lyx::support::FileFilterList const & filters);
	/// sets dialog information line
	void SetInfoLine(std::string const & pszLine);
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
