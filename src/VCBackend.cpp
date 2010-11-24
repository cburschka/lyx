/**
 * \file VCBackend.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "VCBackend.h"
#include "Buffer.h"
#include "LyXFunc.h"
#include "FuncRequest.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#include <boost/regex.hpp>

#include <fstream>

using namespace std;
using namespace lyx::support;

using boost::regex;
using boost::regex_match;
using boost::smatch;

namespace lyx {


int VCS::doVCCommandCall(string const & cmd, FileName const & path)
{
	LYXERR(Debug::LYXVC, "doVCCommandCall: " << cmd);
	Systemcall one;
	support::PathChanger p(path);
	return one.startscript(Systemcall::Wait, cmd);
}


int VCS::doVCCommand(string const & cmd, FileName const & path, bool reportError)
{
	if (owner_)
		owner_->setBusy(true);

	int const ret = doVCCommandCall(cmd, path);

	if (owner_)
		owner_->setBusy(false);
	if (ret && reportError)
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Some problem occured while running the command:\n"
				  "'%1$s'."),
			from_utf8(cmd)));
	return ret;
}


/////////////////////////////////////////////////////////////////////
//
// RCS
//
/////////////////////////////////////////////////////////////////////

RCS::RCS(FileName const & m)
{
	master_ = m;
	scanMaster();
}


FileName const RCS::findFile(FileName const & file)
{
	// Check if *,v exists.
	FileName tmp(file.absFilename() + ",v");
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under rcs: " << tmp);
	if (tmp.isReadableFile()) {
		LYXERR(Debug::LYXVC, "Yes, " << file << " is under rcs.");
		return tmp;
	}

	// Check if RCS/*,v exists.
	tmp = FileName(addName(addPath(onlyPath(file.absFilename()), "RCS"), file.absFilename()) + ",v");
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under rcs: " << tmp);
	if (tmp.isReadableFile()) {
		LYXERR(Debug::LYXVC, "Yes, " << file << " is under rcs.");
		return tmp;
	}

	return FileName();
}


void RCS::retrieve(FileName const & file)
{
	LYXERR(Debug::LYXVC, "LyXVC::RCS: retrieve.\n\t" << file);
	doVCCommandCall("co -q -r " + quoteName(file.toFilesystemEncoding()),
			 FileName());
}


void RCS::scanMaster()
{
	if (master_.empty())
		return;

	LYXERR(Debug::LYXVC, "LyXVC::RCS: scanMaster: " << master_);

	ifstream ifs(master_.toFilesystemEncoding().c_str());

	string token;
	bool read_enough = false;

	while (!read_enough && ifs >> token) {
		LYXERR(Debug::LYXVC, "LyXVC::scanMaster: current lex text: `"
			<< token << '\'');

		if (token.empty())
			continue;
		else if (token == "head") {
			// get version here
			string tmv;
			ifs >> tmv;
			tmv = rtrim(tmv, ";");
			version_ = tmv;
			LYXERR(Debug::LYXVC, "LyXVC: version found to be " << tmv);
		} else if (contains(token, "access")
			   || contains(token, "symbols")
			   || contains(token, "strict")) {
			// nothing
		} else if (contains(token, "locks")) {
			// get locker here
			if (contains(token, ';')) {
				locker_ = "Unlocked";
				vcstatus = UNLOCKED;
				continue;
			}
			string tmpt;
			string s1;
			string s2;
			do {
				ifs >> tmpt;
				s1 = rtrim(tmpt, ";");
				// tmp is now in the format <user>:<version>
				s1 = split(s1, s2, ':');
				// s2 is user, and s1 is version
				if (s1 == version_) {
					locker_ = s2;
					vcstatus = LOCKED;
					break;
				}
			} while (!contains(tmpt, ';'));

		} else if (token == "comment") {
			// we don't need to read any further than this.
			read_enough = true;
		} else {
			// unexpected
			LYXERR(Debug::LYXVC, "LyXVC::scanMaster(): unexpected token");
		}
	}
}


void RCS::registrer(string const & msg)
{
	string cmd = "ci -q -u -i -t-\"";
	cmd += msg;
	cmd += "\" ";
	cmd += quoteName(onlyFilename(owner_->absFileName()));
	doVCCommand(cmd, FileName(owner_->filePath()));
}


string RCS::checkIn(string const & msg)
{
	int ret = doVCCommand("ci -q -u -m\"" + msg + "\" "
		    + quoteName(onlyFilename(owner_->absFileName())),
		    FileName(owner_->filePath()));
	return ret ? string() : "RCS: Proceeded";
}


bool RCS::checkInEnabled()
{
	return owner_ && !owner_->isReadonly();
}


string RCS::checkOut()
{
	owner_->markClean();
	int ret = doVCCommand("co -q -l " + quoteName(onlyFilename(owner_->absFileName())),
		    FileName(owner_->filePath()));
	return ret ? string() : "RCS: Proceeded";
}


bool RCS::checkOutEnabled()
{
	return owner_ && owner_->isReadonly();
}


string RCS::repoUpdate()
{
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool RCS::repoUpdateEnabled()
{
	return false;
}


string RCS::lockingToggle()
{
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool RCS::lockingToggleEnabled()
{
	return false;
}


void RCS::revert()
{
	doVCCommand("co -f -u" + version() + " "
		    + quoteName(onlyFilename(owner_->absFileName())),
		    FileName(owner_->filePath()));
	// We ignore changes and just reload!
	owner_->markClean();
}


void RCS::undoLast()
{
	LYXERR(Debug::LYXVC, "LyXVC: undoLast");
	doVCCommand("rcs -o" + version() + " "
		    + quoteName(onlyFilename(owner_->absFileName())),
		    FileName(owner_->filePath()));
}


bool RCS::undoLastEnabled()
{
	return true;
}


void RCS::getLog(FileName const & tmpf)
{
	doVCCommand("rlog " + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


bool RCS::toggleReadOnlyEnabled()
{
	return true;
}


/////////////////////////////////////////////////////////////////////
//
// CVS
//
/////////////////////////////////////////////////////////////////////

CVS::CVS(FileName const & m, FileName const & f)
{
	master_ = m;
	file_ = f;
	scanMaster();
}


FileName const CVS::findFile(FileName const & file)
{
	// First we look for the CVS/Entries in the same dir
	// where we have file.
	FileName const entries(onlyPath(file.absFilename()) + "/CVS/Entries");
	string const tmpf = '/' + onlyFilename(file.absFilename()) + '/';
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under cvs in `" << entries
			     << "' for `" << tmpf << '\'');
	if (entries.isReadableFile()) {
		// Ok we are at least in a CVS dir. Parse the CVS/Entries
		// and see if we can find this file. We do a fast and
		// dirty parse here.
		ifstream ifs(entries.toFilesystemEncoding().c_str());
		string line;
		while (getline(ifs, line)) {
			LYXERR(Debug::LYXVC, "\tEntries: " << line);
			if (contains(line, tmpf))
				return entries;
		}
	}
	return FileName();
}


void CVS::scanMaster()
{
	LYXERR(Debug::LYXVC, "LyXVC::CVS: scanMaster. \n     Checking: " << master_);
	// Ok now we do the real scan...
	ifstream ifs(master_.toFilesystemEncoding().c_str());
	string name = onlyFilename(file_.absFilename());
	string tmpf = '/' + name + '/';
	LYXERR(Debug::LYXVC, "\tlooking for `" << tmpf << '\'');
	string line;
	static regex const reg("/(.*)/(.*)/(.*)/(.*)/(.*)");
	while (getline(ifs, line)) {
		LYXERR(Debug::LYXVC, "\t  line: " << line);
		if (contains(line, tmpf)) {
			// Ok extract the fields.
			smatch sm;

			regex_match(line, sm, reg);

			//sm[0]; // whole matched string
			//sm[1]; // filename
			version_ = sm.str(2);
			string const file_date = sm.str(3);

			//sm[4]; // options
			//sm[5]; // tag or tagdate
			if (file_.isReadableFile()) {
			time_t mod = file_.lastModified();
			string mod_date = rtrim(asctime(gmtime(&mod)), "\n");
			LYXERR(Debug::LYXVC, "Date in Entries: `" << file_date
				<< "'\nModification date of file: `" << mod_date << '\'');
				if (file_.isReadOnly()) {
					// readonly checkout is unlocked
				vcstatus = UNLOCKED;
			} else {
					FileName bdir(addPath(master_.onlyPath().absFilename(),"Base"));
					FileName base(addName(bdir.absFilename(),name));
					// if base version is existent "cvs edit" was used to lock
					vcstatus = base.isReadableFile() ? LOCKED : NOLOCKING;
				}
			} else {
				vcstatus = NOLOCKING;
			}
			break;
		}
	}
}


string const CVS::getTarget(OperationMode opmode) const
{
	switch(opmode) {
	case Directory:
		return ".";
	case File:
		return quoteName(onlyFilename(owner_->absFileName()));
	}
	return string();
}


docstring CVS::toString(CvsStatus status) const
{
	switch (status) {
	case UpToDate:
		return _("Up-to-date");
	case LocallyModified:
		return _("Locally Modified");
	case LocallyAdded:
		return _("Locally Added");
	case NeedsMerge:
		return _("Needs Merge");
	case NeedsCheckout:
		return _("Needs Checkout");
	case NoCvsFile:
		return _("No CVS file");
	case StatusError:
		return _("Cannot retrieve CVS status");
	}
	return 0;
}


CVS::CvsStatus CVS::getStatus()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return StatusError;
	}

	if (doVCCommand("cvs status " + getTarget(File)
		+ " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()))) {
		tmpf.removeFile();
		return StatusError;
	}

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	CvsStatus status = NoCvsFile;

	while (ifs) {
		string line;
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line << "\n");
		if (prefixIs(line, "File:")) {
			if (contains(line, "Up-to-date"))
				status = UpToDate;
			else if (contains(line, "Locally Modified"))
				status = LocallyModified;
			else if (contains(line, "Locally Added"))
				status = LocallyAdded;
			else if (contains(line, "Needs Merge"))
				status = NeedsMerge;
			else if (contains(line, "Needs Checkout"))
				status = NeedsCheckout;
		}
	}
	tmpf.removeFile();
	return status;
}


void CVS::registrer(string const & msg)
{
	doVCCommand("cvs -q add -m \"" + msg + "\" "
		+ getTarget(File),
		    FileName(owner_->filePath()));
}


void CVS::getDiff(OperationMode opmode, FileName const & tmpf)
{
	doVCCommand("cvs diff " + getTarget(opmode)
		+ " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()), false);
}


int CVS::edit()
{
	vcstatus = LOCKED;
	return doVCCommand("cvs -q edit " + getTarget(File),
		FileName(owner_->filePath()));
}


int CVS::unedit()
{
	vcstatus = UNLOCKED;
	return doVCCommand("cvs -q unedit " + getTarget(File),
		FileName(owner_->filePath()));
}


int CVS::update(OperationMode opmode, FileName const & tmpf)
{
	string const redirection = tmpf.empty() ? ""
		: " > " + quoteName(tmpf.toFilesystemEncoding());

	return doVCCommand("cvs -q update "
		+ getTarget(opmode) + redirection,
		FileName(owner_->filePath()));
}


string CVS::scanLogFile(FileName const & f, string & status)
{
	ifstream ifs(f.toFilesystemEncoding().c_str());

	while (ifs) {
		string line;
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line << "\n");
		if (!line.empty())
			status += line + "; ";
		if (prefixIs(line, "C ")) {
			ifs.close();
			return line;
		}
	}
	ifs.close();
	return string();
}
	
	
string CVS::checkIn(string const & msg)
{
	CvsStatus status = getStatus();
	switch (status) {
	case UpToDate:
		if (vcstatus != NOLOCKING)
			unedit();
		return "CVS: Proceeded";
	case LocallyModified:
	case LocallyAdded: {
		int rc = doVCCommand("cvs -q commit -m \"" + msg + "\" "
			+ getTarget(File),
		    FileName(owner_->filePath()));
		return rc ? string() : "CVS: Proceeded";
	}
	case NeedsMerge:
	case NeedsCheckout:
		frontend::Alert::error(_("Revision control error."),
			_("The repository version is newer then the current check out.\n"
			  "You have to update from repository first or revert your changes.")) ;
		break;
	default:
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Bad status when checking in changes.\n"
					  "\n'%1$s'\n\n"),
				toString(status)));
		break;
	}
	return string();
}


bool CVS::isLocked() const
{
	FileName fn(owner_->absFileName());
	fn.refresh();
	return !fn.isReadOnly();
}


bool CVS::checkInEnabled()
{
	if (vcstatus != NOLOCKING)
		return isLocked();
	else
	return true;
}


string CVS::checkOut()
{
	if (vcstatus != NOLOCKING && edit())
	return string();
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return string();
	}
	
	int rc = update(File, tmpf);
	string log;
	string const res = scanLogFile(tmpf, log);
	if (!res.empty())
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Error when updating from repository.\n"
				"You have to manually resolve the conflicts NOW!\n'%1$s'.\n\n"
				"After pressing OK, LyX will try to reopen the resolved document."),
				from_local8bit(res)));
	
	tmpf.erase();
	return rc ? string() : log.empty() ? "CVS: Proceeded" : "CVS: " + log;
}


bool CVS::checkOutEnabled()
{
	if (vcstatus != NOLOCKING)
		return !isLocked();
	else
		return true;
}


string CVS::repoUpdate()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return string();
	}
	
	getDiff(Directory, tmpf);
	docstring res = tmpf.fileContents("UTF-8");
	if (!res.empty()) {
		LYXERR(Debug::LYXVC, "Diff detected:\n" << res);
		docstring const file = from_utf8(owner_->filePath());
		docstring text = bformat(_("There were detected changes "
				"in the working directory:\n%1$s\n\n"
				"In case of file conflict you have to resolve them "
				"manually or revert to repository version later."), file);
		int ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Continue"), _("&Abort"), _("View &Log ..."));
		if (ret == 2 ) {
			dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + tmpf.absFilename()));
			ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Continue"), _("&Abort"));
			hideDialogs("file", 0);
		}
		if (ret == 1 ) {
			tmpf.removeFile();
			return string();
		}
	}

	int rc = update(Directory, tmpf);
	res += "Update log:\n" + tmpf.fileContents("UTF-8");
	tmpf.removeFile();

	LYXERR(Debug::LYXVC, res);
	return rc ? string() : "CVS: Proceeded" ;
}


bool CVS::repoUpdateEnabled()
{
	return true;
}


string CVS::lockingToggle()
{
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool CVS::lockingToggleEnabled()
{
	return false;
}


void CVS::revert()
{
	// Reverts to the version in CVS repository and
	// gets the updated version from the repository.
	CvsStatus status = getStatus();
	switch (status) {
	case UpToDate:
		if (vcstatus != NOLOCKING)
			unedit();
		break;
	case NeedsMerge:
	case NeedsCheckout:
	case LocallyModified: {
		FileName f(owner_->absFileName());
		f.removeFile();
		update(File, FileName());
		owner_->markClean();
		break;
	}
	case LocallyAdded: {
		docstring const file = owner_->fileName().displayName(20);
		frontend::Alert::error(_("Revision control error."),
			bformat(_("The document %1$s is not in repository.\n"
			          "You have to check in the first revision before you can revert."),
				file)) ;
		break;
	}
	default: {
		docstring const file = owner_->fileName().displayName(20);
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Cannot revert document %1$s to repository version.\n"
			          "The status '%2$s' is unexpected."),
				file, toString(status)));
		break;
		}
	}
}


void CVS::undoLast()
{
	// merge the current with the previous version
	// in a reverse patch kind of way, so that the
	// result is to revert the last changes.
	lyxerr << "Sorry, not implemented." << endl;
}


bool CVS::undoLastEnabled()
{
	return false;
}


void CVS::getLog(FileName const & tmpf)
{
	doVCCommand("cvs log " + getTarget(File)
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


bool CVS::toggleReadOnlyEnabled()
{
	return false;
}


/////////////////////////////////////////////////////////////////////
//
// SVN
//
/////////////////////////////////////////////////////////////////////

SVN::SVN(FileName const & m, FileName const & f)
{
	owner_ = 0;
	master_ = m;
	file_ = f;
	locked_mode_ = 0;
	scanMaster();
}


FileName const SVN::findFile(FileName const & file)
{
	// First we look for the CVS/Entries in the same dir
	// where we have file.
	FileName const entries(onlyPath(file.absFilename()) + "/.svn/entries");
	string const tmpf = onlyFilename(file.absFilename());
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under svn in `" << entries
			     << "' for `" << tmpf << '\'');
	if (entries.isReadableFile()) {
		// Ok we are at least in a CVS dir. Parse the CVS/Entries
		// and see if we can find this file. We do a fast and
		// dirty parse here.
		ifstream ifs(entries.toFilesystemEncoding().c_str());
		string line, oldline;
		while (getline(ifs, line)) {
			if (line == "dir" || line == "file")
				LYXERR(Debug::LYXVC, "\tEntries: " << oldline);
			if (oldline == tmpf && line == "file")
				return entries;
			oldline = line;
		}
	}
	return FileName();
}


void SVN::scanMaster()
{
	locker_.clear();
	vcstatus = NOLOCKING;
	if (checkLockMode()) {
		if (isLocked()) {
			locker_ = "Locked";
			vcstatus = LOCKED;
		} else {
			locker_ = "Unlocked";
			vcstatus = LOCKED;
		}
	}
}


bool SVN::checkLockMode()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()){
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	LYXERR(Debug::LYXVC, "Detecting locking mode...");
	if (doVCCommandCall("svn proplist " + quoteName(file_.onlyFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    file_.onlyPath()))
		return false;

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;
	bool ret = false;

	while (ifs) {
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line);
		if (contains(line, "svn:needs-lock"))
			ret = true;
	}
	LYXERR(Debug::LYXVC, "Locking enabled: " << ret);
	ifs.close();
	locked_mode_ = ret;
	return ret;

}


bool SVN::isLocked() const
{
	//refresh file info
	FileName file(file_.absFilename());
	return !file.isReadOnly();
}


void SVN::registrer(string const & /*msg*/)
{
	doVCCommand("svn add -q " + quoteName(onlyFilename(owner_->absFileName())),
		    FileName(owner_->filePath()));
}


string SVN::checkIn(string const & msg)
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()){
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	doVCCommand("svn commit -m \"" + msg + "\" "
		    + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	string log;
	string res = scanLogFile(tmpf, log);
	if (!res.empty())
		frontend::Alert::error(_("Revision control error."),
				_("Error when committing to repository.\n"
				"You have to manually resolve the problem.\n"
				"After pressing OK, LyX will reopen the document."));
	else
		fileLock(false, tmpf, log);

	tmpf.erase();
	return "SVN: " + log;
}


bool SVN::checkInEnabled()
{
	if (locked_mode_)
		return isLocked();
	else
		return true;
}


// FIXME Correctly return code should be checked instead of this.
// This would need another solution than just plain startscript.
// Hint from Andre': QProcess::readAllStandardError()...
string SVN::scanLogFile(FileName const & f, string & status)
{
	ifstream ifs(f.toFilesystemEncoding().c_str());
	string line;

	while (ifs) {
		getline(ifs, line);
		lyxerr << line << "\n";
		if (!line.empty()) status += line + "; ";
		if (prefixIs(line, "C ") || prefixIs(line, "CU ") ||
				contains(line, "Commit failed")) {
			ifs.close();
			return line;
		}
		if (contains(line, "svn:needs-lock")) {
			ifs.close();
			return line;
		}
	}
	ifs.close();
	return string();
}


void SVN::fileLock(bool lock, FileName const & tmpf, string &status)
{
	if (!locked_mode_ || (isLocked() == lock))
		return;

	string arg = lock ? "lock " : "unlock ";
	doVCCommand("svn "+ arg + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;
	while (ifs) {
		getline(ifs, line);
		if (!line.empty()) status += line + "; ";
	}
	ifs.close();

	if (!isLocked() && lock)
		frontend::Alert::error(_("Revision control error."),
			_("Error when acquiring write lock.\n"
			"Most probably another user is editing\n"
			"the current document now!\n"
			"Also check the access to the repository."));
	if (isLocked() && !lock)
		frontend::Alert::error(_("Revision control error."),
			_("Error when releasing write lock.\n"
			"Check the access to the repository."));
}


string SVN::checkOut()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	doVCCommand("svn update --non-interactive " + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	string log;
	string res = scanLogFile(tmpf, log);
	if (!res.empty())
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Error when updating from repository.\n"
				"You have to manually resolve the conflicts NOW!\n'%1$s'.\n\n"
				"After pressing OK, LyX will try to reopen resolved document."),
			from_local8bit(res)));

	fileLock(true, tmpf, log);

	tmpf.erase();
	return "SVN: " + log;
}


bool SVN::checkOutEnabled()
{
	if (locked_mode_)
		return !isLocked();
	else
		return true;
}


string SVN::repoUpdate()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	doVCCommand("svn diff " + quoteName(owner_->filePath())
	+ " > " + quoteName(tmpf.toFilesystemEncoding()),
	FileName(owner_->filePath()));
	docstring res = tmpf.fileContents("UTF-8");
	if (!res.empty()) {
		LYXERR(Debug::LYXVC, "Diff detected:\n" << res);
		docstring const file = from_utf8(owner_->filePath());
		docstring text = bformat(_("There were detected changes "
				"in the working directory:\n%1$s\n\n"
				"In case of file conflict version of the local directory files "
				"will be preferred."
				"\n\nContinue?"), file);
		int ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Yes"), _("&No"), _("View &Log ..."));
		if (ret == 2 ) {
			dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + tmpf.absFilename()));
			ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Yes"), _("&No"));
			hideDialogs("file", 0);
		}
		if (ret == 1 ) {
			tmpf.erase();
			return string();
		}
	}

	// Reverting looks too harsh, see bug #6255.
	// doVCCommand("svn revert -R " + quoteName(owner_->filePath())
	// + " > " + quoteName(tmpf.toFilesystemEncoding()),
	// FileName(owner_->filePath()));
	// res = "Revert log:\n" + tmpf.fileContents("UTF-8");
	doVCCommand("svn update --accept mine-full " + quoteName(owner_->filePath())
	+ " > " + quoteName(tmpf.toFilesystemEncoding()),
	FileName(owner_->filePath()));
	res += "Update log:\n" + tmpf.fileContents("UTF-8");

	LYXERR(Debug::LYXVC, res);
	tmpf.erase();
	return to_utf8(res);
}


bool SVN::repoUpdateEnabled()
{
	return true;
}


string SVN::lockingToggle()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	int ret = doVCCommand("svn proplist " + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
	if (ret)
		return string();

	string log;
	string res = scanLogFile(tmpf, log);
	bool locking = contains(res, "svn:needs-lock");
	if (!locking)
		ret = doVCCommand("svn propset svn:needs-lock ON "
		    + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
	else
		ret = doVCCommand("svn propdel svn:needs-lock "
		    + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
	if (ret)
		return string();

	tmpf.erase();
	frontend::Alert::warning(_("VCN File Locking"),
		(locking ? _("Locking property unset.") : _("Locking property set.")) + "\n"
		+ _("Do not forget to commit the locking property into the repository."),
		true);

	return string("SVN: ") +  N_("Locking property set.");
}


bool SVN::lockingToggleEnabled()
{
	return true;
}


void SVN::revert()
{
	// Reverts to the version in CVS repository and
	// gets the updated version from the repository.
	string const fil = quoteName(onlyFilename(owner_->absFileName()));

	doVCCommand("svn revert -q " + fil,
		    FileName(owner_->filePath()));
	owner_->markClean();
}


void SVN::undoLast()
{
	// merge the current with the previous version
	// in a reverse patch kind of way, so that the
	// result is to revert the last changes.
	lyxerr << "Sorry, not implemented." << endl;
}


bool SVN::undoLastEnabled()
{
	return false;
}


void SVN::getLog(FileName const & tmpf)
{
	doVCCommand("svn log " + quoteName(onlyFilename(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


bool SVN::toggleReadOnlyEnabled()
{
	return false;
}


} // namespace lyx
