/**
 * \file VCBackend.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "VCBackend.h"
#include "Buffer.h"
#include "LyX.h"
#include "FuncRequest.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Path.h"
#include "support/Systemcall.h"
#include "support/regex.h"

#include <fstream>

using namespace std;
using namespace lyx::support;



namespace lyx {


int VCS::doVCCommandCall(string const & cmd, FileName const & path)
{
	LYXERR(Debug::LYXVC, "doVCCommandCall: " << cmd);
	Systemcall one;
	support::PathChanger p(path);
	return one.startscript(Systemcall::Wait, cmd, string(), false);
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


bool VCS::makeRCSRevision(string const &version, string &revis) const
{
	string rev = revis;
	
	if (isStrInt(rev)) {
		int back = convert<int>(rev);
		// if positive use as the last number in the whole revision string
		if (back > 0) {
			string base;
			rsplit(version, base , '.' );
			rev = base + "." + rev;
		}
		if (back == 0)
			rev = version;
		// we care about the last number from revision string
		// in case of backward indexing
		if (back < 0) {
			string cur, base;
			cur = rsplit(version, base , '.' );
			if (!isStrInt(cur))
				return false;
			int want = convert<int>(cur) + back;
			if (want <= 0)
				return false;
			
			rev = base + "." + convert<string>(want);
		}
	}

	revis = rev;
	return true;
}

bool VCS::checkparentdirs(FileName const & file, std::string const & pathname)
{
	FileName dirname = file.onlyPath();
	FileName tocheck = FileName(addName(dirname.absFileName(),pathname));
	LYXERR(Debug::LYXVC, "check file: " << tocheck.absFileName());
	bool result = tocheck.exists();
	while ( !result && !dirname.empty() ) {
		dirname = dirname.parentPath();
		LYXERR(Debug::LYXVC, "check directory: " << dirname.absFileName());
		tocheck = FileName(addName(dirname.absFileName(),pathname));
		result = tocheck.exists();
	}
	return result;
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
	FileName tmp(file.absFileName() + ",v");
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under rcs: " << tmp);
	if (tmp.isReadableFile()) {
		LYXERR(Debug::LYXVC, "Yes, " << file << " is under rcs.");
		return tmp;
	}

	// Check if RCS/*,v exists.
	tmp = FileName(addName(addPath(onlyPath(file.absFileName()), "RCS"), file.absFileName()) + ",v");
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
	cmd += quoteName(onlyFileName(owner_->absFileName()));
	doVCCommand(cmd, FileName(owner_->filePath()));
}


string RCS::checkIn(string const & msg)
{
	int ret = doVCCommand("ci -q -u -m\"" + msg + "\" "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
	return ret ? string() : "RCS: Proceeded";
}


bool RCS::checkInEnabled()
{
	return owner_ && !owner_->isReadonly();
}


bool RCS::isCheckInWithConfirmation()
{
	// FIXME one day common getDiff for all backends
	// docstring diff;
	// if (getDiff(file, diff) && diff.empty())
	//	return false;

	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return true;
	}

	doVCCommandCall("rcsdiff " + quoteName(owner_->absFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));

	docstring diff = tmpf.fileContents("UTF-8");
	tmpf.erase();

	if (diff.empty())
		return false;

	return true;
}


string RCS::checkOut()
{
	owner_->markClean();
	int ret = doVCCommand("co -q -l " + quoteName(onlyFileName(owner_->absFileName())),
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
	//FIXME this might be actually possible, study rcs -U, rcs -L.
	//State should be easy to get inside scanMaster.
	//It would fix #4370 and make rcs/svn usage even more closer.
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool RCS::lockingToggleEnabled()
{
	return false;
}


bool RCS::revert()
{
	if (doVCCommand("co -f -u" + version_ + " "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath())))
		return false;
	// We ignore changes and just reload!
	owner_->markClean();
	return true;
}


bool RCS::isRevertWithConfirmation()
{
	//FIXME owner && diff ?
	return true;
}


void RCS::undoLast()
{
	LYXERR(Debug::LYXVC, "LyXVC: undoLast");
	doVCCommand("rcs -o" + version_ + " "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
}


bool RCS::undoLastEnabled()
{
	return true;
}


void RCS::getLog(FileName const & tmpf)
{
	doVCCommand("rlog " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


bool RCS::toggleReadOnlyEnabled()
{
	// This got broken somewhere along lfuns dispatch reorganization.
	// reloadBuffer would be needed after this, but thats problematic
	// since we are inside Buffer::dispatch.
	// return true;
	return false;
}


string RCS::revisionInfo(LyXVC::RevisionInfo const info)
{
	if (info == LyXVC::File)
		return version_;
	// fill the rest of the attributes for a single file
	if (rev_date_cache_.empty())
		if (!getRevisionInfo())
			return string();

	switch (info) {
		case LyXVC::Author:
			return rev_author_cache_;
		case LyXVC::Date:
			return rev_date_cache_;
		case LyXVC::Time:
			return rev_time_cache_;
		default: ;
	}

	return string();
}


bool RCS::getRevisionInfo()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}
	doVCCommand("rlog -r " + quoteName(onlyFileName(owner_->absFileName()))
		+ " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));

	if (tmpf.empty())
		return false;

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;

	// we reached to the entry, i.e. after initial log message
	bool entry=false;
	// line with critical info, e.g:
	//"date: 2011/07/02 11:02:54;  author: sanda;  state: Exp;  lines: +17 -2"
	string result;

	while (ifs) {
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line);
		if (entry && prefixIs(line, "date:")) {
			result = line;
			break;
		}
		if (prefixIs(line, "revision"))
			entry = true;
	}
	if (result.empty())
		return false;

	rev_date_cache_ = token(result, ' ', 1);
	rev_time_cache_ = rtrim(token(result, ' ', 2), ";");
	rev_author_cache_ = trim(token(token(result, ';', 1), ':', 1));

	return !rev_author_cache_.empty();
}

bool RCS::prepareFileRevision(string const &revis, string & f)
{
	string rev = revis;
	if (!VCS::makeRCSRevision(version_, rev))
		return false;

	FileName tmpf = FileName::tempName("lyxvcrev_" + rev + "_");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("co -p" + rev + " "
	              + quoteName(onlyFileName(owner_->absFileName()))
		      + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));
	if (tmpf.isFileEmpty())
		return false;

	f = tmpf.absFileName();
	return true;
}


bool RCS::prepareFileRevisionEnabled()
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
	have_rev_info_ = false;
	scanMaster();
}


FileName const CVS::findFile(FileName const & file)
{
	// First we look for the CVS/Entries in the same dir
	// where we have file.
	FileName const entries(onlyPath(file.absFileName()) + "/CVS/Entries");
	string const tmpf = '/' + onlyFileName(file.absFileName()) + '/';
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
	string name = onlyFileName(file_.absFileName());
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
					FileName bdir(addPath(master_.onlyPath().absFileName(),"Base"));
					FileName base(addName(bdir.absFileName(),name));
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
		// in client server mode CVS does not like full path operand for directory operation
		// since LyX switches to the repo dir "." is good enough as target
		return ".";
	case File:
		return quoteName(onlyFileName(owner_->absFileName()));
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


int CVS::doVCCommandWithOutput(string const & cmd, FileName const & path,
	FileName const & output, bool reportError)
{
	string redirection = output.empty() ? "" : " > " + quoteName(output.toFilesystemEncoding());
	return doVCCommand(cmd + redirection, path, reportError);
}


int CVS::doVCCommandCallWithOutput(std::string const & cmd,
	support::FileName const & path,
	support::FileName const & output)
{
	string redirection = output.empty() ? "" : " > " + quoteName(output.toFilesystemEncoding());
	return doVCCommandCall(cmd + redirection, path);
}


CVS::CvsStatus CVS::getStatus()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return StatusError;
	}

	if (doVCCommandCallWithOutput("cvs status " + getTarget(File),
		FileName(owner_->filePath()), tmpf)) {
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

void CVS::getRevisionInfo()
{
	if (have_rev_info_)
		return;
	have_rev_info_ = true;
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return;
	}
	
	int rc = doVCCommandCallWithOutput("cvs log -r" + version_ 
		+ " " + getTarget(File),
		FileName(owner_->filePath()), tmpf);
	if (rc) {
		tmpf.removeFile();
		LYXERR(Debug::LYXVC, "cvs log failed with exit code " << rc);
		return;
	}
	
	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	static regex const reg("date: (.*) (.*) (.*);  author: (.*);  state: (.*);(.*)");

	while (ifs) {
		string line;
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line << "\n");
		if (prefixIs(line, "date:")) {
			smatch sm;
			regex_match(line, sm, reg);
			//sm[0]; // whole matched string
			rev_date_cache_ = sm[1];
			rev_time_cache_ = sm[2];
			//sm[3]; // GMT offset
			rev_author_cache_ = sm[4];
			break;
		}
	}
	tmpf.removeFile();
	if (rev_author_cache_.empty())
		LYXERR(Debug::LYXVC,
		   "Could not retrieve revision info for " << version_ <<
		   " of " << getTarget(File));
}


void CVS::registrer(string const & msg)
{
	doVCCommand("cvs -q add -m \"" + msg + "\" "
		+ getTarget(File),
		FileName(owner_->filePath()));
}


void CVS::getDiff(OperationMode opmode, FileName const & tmpf)
{
	doVCCommandWithOutput("cvs diff " + getTarget(opmode),
		FileName(owner_->filePath()), tmpf, false);
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
	return doVCCommandWithOutput("cvs -q update "
		+ getTarget(opmode),
		FileName(owner_->filePath()), tmpf, false);
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


bool CVS::isCheckInWithConfirmation()
{
	CvsStatus status = getStatus();
	return status == LocallyModified || status == LocallyAdded;
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
	if (!res.empty()) {
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Error when updating from repository.\n"
				"You have to manually resolve the conflicts NOW!\n'%1$s'.\n\n"
				"After pressing OK, LyX will try to reopen the resolved document."),
				from_local8bit(res)));
		rc = 0;
	}
	
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
				"Possible file conflicts must be then resolved manually "
				"or you will need to revert back to the repository version."), file);
		int ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Continue"), _("&Abort"), _("View &Log ..."));
		if (ret == 2 ) {
			dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + tmpf.absFileName()));
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
	LYXERR(Debug::LYXVC, res);

	string log;
	string sres = scanLogFile(tmpf, log);
	if (!sres.empty()) {
		docstring const file = owner_->fileName().displayName(20);
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Error when updating document %1$s from repository.\n"
					  "You have to manually resolve the conflicts NOW!\n'%2$s'.\n\n"
					  "After pressing OK, LyX will try to reopen the resolved document."),
				file, from_local8bit(sres)));
		rc = 0;
	}
	
	tmpf.removeFile();

	return rc ? string() : log.empty() ? "CVS: Proceeded" : "CVS: " + log;
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


bool CVS::isRevertWithConfirmation()
{
	CvsStatus status = getStatus();
	return !owner_->isClean() || status == LocallyModified || status == NeedsMerge;
}


bool CVS::revert()
{
	// Reverts to the version in CVS repository and
	// gets the updated version from the repository.
	CvsStatus status = getStatus();
	switch (status) {
	case UpToDate:
		if (vcstatus != NOLOCKING)
			return 0 == unedit();
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
		return false;
	}
	default: {
		docstring const file = owner_->fileName().displayName(20);
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Cannot revert document %1$s to repository version.\n"
			          "The status '%2$s' is unexpected."),
				file, toString(status)));
		return false;
		}
	}
	return true;
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
	doVCCommandWithOutput("cvs log " + getTarget(File),
		FileName(owner_->filePath()),
		tmpf);
}


bool CVS::toggleReadOnlyEnabled()
{
	return false;
}


string CVS::revisionInfo(LyXVC::RevisionInfo const info)
{
	if (!version_.empty()) {
		getRevisionInfo();
		switch (info) {
		case LyXVC::File:
			return version_;
		case LyXVC::Author:
			return rev_author_cache_;
		case LyXVC::Date:
			return rev_date_cache_;
		case LyXVC::Time:
			return rev_time_cache_;
		default: ;
		}
	}
	return string();
}


bool CVS::prepareFileRevision(string const & revis, string & f)
{
	string rev = revis;
	if (!VCS::makeRCSRevision(version_, rev))
		return false;

	FileName tmpf = FileName::tempName("lyxvcrev_" + rev + "_");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommandWithOutput("cvs update -p -r" + rev + " "
		+ getTarget(File),
		FileName(owner_->filePath()), tmpf);
	if (tmpf.isFileEmpty())
		return false;

	f = tmpf.absFileName();
	return true;
}


bool CVS::prepareFileRevisionEnabled()
{
	return true;
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
	// First we check the existence of repository meta data.
	if (!VCS::checkparentdirs(file, ".svn")) {
		LYXERR(Debug::LYXVC, "Cannot find SVN meta data for " << file);
		return FileName();
	}

	// Now we check the status of the file.
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return FileName();
	}

	string const fname = onlyFileName(file.absFileName());
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under svn control for `" << fname << '\'');
	bool found = 0 == doVCCommandCall("svn info " + quoteName(fname)
						+ " > " + quoteName(tmpf.toFilesystemEncoding()),
						file.onlyPath());
	LYXERR(Debug::LYXVC, "SVN control: " << (found ? "enabled" : "disabled"));
	return found ? file : FileName();
}


void SVN::scanMaster()
{
	// vcstatus code is somewhat superflous, until we want
	// to implement read-only toggle for svn.
	vcstatus = NOLOCKING;
	if (checkLockMode()) {
		if (isLocked()) {
			vcstatus = LOCKED;
		} else {
			vcstatus = UNLOCKED;
		}
	}
}


bool SVN::checkLockMode()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()){
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	LYXERR(Debug::LYXVC, "Detecting locking mode...");
	if (doVCCommandCall("svn proplist " + quoteName(file_.onlyFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    file_.onlyPath()))
		return false;

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;
	bool ret = false;

	while (ifs && !ret) {
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
	file_.refresh();
	return !file_.isReadOnly();
}


void SVN::registrer(string const & /*msg*/)
{
	doVCCommand("svn add -q " + quoteName(onlyFileName(owner_->absFileName())),
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
		    + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	string log;
	string res = scanLogFile(tmpf, log);
	if (!res.empty())
		frontend::Alert::error(_("Revision control error."),
				_("Error when committing to repository.\n"
				"You have to manually resolve the problem.\n"
				"LyX will reopen the document after you press OK."));
	else
		fileLock(false, tmpf, log);

	tmpf.erase();
	return log.empty() ? string() : "SVN: " + log;
}


bool SVN::checkInEnabled()
{
	if (locked_mode_)
		return isLocked();
	else
		return true;
}


bool SVN::isCheckInWithConfirmation()
{
	// FIXME one day common getDiff and perhaps OpMode for all backends

	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return true;
	}

	doVCCommandCall("svn diff " + quoteName(owner_->absFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));

	docstring diff = tmpf.fileContents("UTF-8");
	tmpf.erase();

	if (diff.empty())
		return false;

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
		LYXERR(Debug::LYXVC, line << "\n");
		if (!line.empty()) 
			status += line + "; ";
		if (prefixIs(line, "C ") || prefixIs(line, "CU ")
					 || contains(line, "Commit failed")) {
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

	string const arg = lock ? "lock " : "unlock ";
	doVCCommand("svn "+ arg + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	// Lock error messages go unfortunately on stderr and are unreachible this way.
	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;
	while (ifs) {
		getline(ifs, line);
		if (!line.empty()) status += line + "; ";
	}
	ifs.close();

	if (!isLocked() && lock)
		frontend::Alert::error(_("Revision control error."),
			_("Error while acquiring write lock.\n"
			"Another user is most probably editing\n"
			"the current document now!\n"
			"Also check the access to the repository."));
	if (isLocked() && !lock)
		frontend::Alert::error(_("Revision control error."),
			_("Error while releasing write lock.\n"
			"Check the access to the repository."));
}


string SVN::checkOut()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	doVCCommand("svn update --non-interactive " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	string log;
	string const res = scanLogFile(tmpf, log);
	if (!res.empty())
		frontend::Alert::error(_("Revision control error."),
			bformat(_("Error when updating from repository.\n"
				"You have to manually resolve the conflicts NOW!\n'%1$s'.\n\n"
				"After pressing OK, LyX will try to reopen the resolved document."),
			from_local8bit(res)));

	fileLock(true, tmpf, log);

	tmpf.erase();
	return log.empty() ? string() : "SVN: " + log;
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
			dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + tmpf.absFileName()));
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

	int ret = doVCCommand("svn proplist " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
	if (ret)
		return string();

	string log;
	string res = scanLogFile(tmpf, log);
	bool locking = contains(res, "svn:needs-lock");
	if (!locking)
		ret = doVCCommand("svn propset svn:needs-lock ON "
		    + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
	else
		ret = doVCCommand("svn propdel svn:needs-lock "
		    + quoteName(onlyFileName(owner_->absFileName()))
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


bool SVN::revert()
{
	// Reverts to the version in SVN repository and
	// gets the updated version from the repository.
	string const fil = quoteName(onlyFileName(owner_->absFileName()));

	if (doVCCommand("svn revert -q " + fil,
		    FileName(owner_->filePath())))
		return false;
	owner_->markClean();
	return true;
}


bool SVN::isRevertWithConfirmation()
{
	//FIXME owner && diff
	return true;
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


string SVN::revisionInfo(LyXVC::RevisionInfo const info)
{
	if (info == LyXVC::Tree) {
			if (rev_tree_cache_.empty())
				if (!getTreeRevisionInfo())
					rev_tree_cache_ = "?";
			if (rev_tree_cache_ == "?")
				return string();

			return rev_tree_cache_;
	}

	// fill the rest of the attributes for a single file
	if (rev_file_cache_.empty())
		if (!getFileRevisionInfo())
			rev_file_cache_ = "?";

	switch (info) {
		case LyXVC::File:
			if (rev_file_cache_ == "?")
				return string();
			return rev_file_cache_;
		case LyXVC::Author:
			return rev_author_cache_;
		case LyXVC::Date:
			return rev_date_cache_;
		case LyXVC::Time:
			return rev_time_cache_;
		default: ;

	}

	return string();
}


bool SVN::getFileRevisionInfo()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("svn info --xml " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	if (tmpf.empty())
		return false;

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;
	// commit log part
	bool c = false;
	string rev;

	while (ifs) {
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line);
		if (prefixIs(line, "<commit"))
			c = true;
		if (c && prefixIs(line, "   revision=\"") && suffixIs(line, "\">")) {
			string l1 = subst(line, "revision=\"", "");
			string l2 = trim(subst(l1, "\">", ""));
			if (isStrInt(l2))
				rev_file_cache_ = rev = l2;
		}
		if (c && prefixIs(line, "<author>") && suffixIs(line, "</author>")) {
			string l1 = subst(line, "<author>", "");
			string l2 = subst(l1, "</author>", "");
			rev_author_cache_ = l2;
		}
		if (c && prefixIs(line, "<date>") && suffixIs(line, "</date>")) {
			string l1 = subst(line, "<date>", "");
			string l2 = subst(l1, "</date>", "");
			l2 = split(l2, l1, 'T');
			rev_date_cache_ = l1;
			l2 = split(l2, l1, '.');
			rev_time_cache_ = l1;
		}
	}

	ifs.close();
	tmpf.erase();
	return !rev.empty();
}


bool SVN::getTreeRevisionInfo()
{
	FileName tmpf = FileName::tempName("lyxvcout");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("svnversion -n . > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	if (tmpf.empty())
		return false;

	// only first line in case something bad happens.
	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	string line;
	getline(ifs, line);
	ifs.close();
	tmpf.erase();

	rev_tree_cache_ = line;
	return !line.empty();
}


void SVN::getLog(FileName const & tmpf)
{
	doVCCommand("svn log " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


bool SVN::prepareFileRevision(string const & revis, string & f)
{
	if (!isStrInt(revis))
		return false;

	int rev = convert<int>(revis);
	if (rev <= 0)
		if (!getFileRevisionInfo())
			return false;
	if (rev == 0)
		rev = convert<int>(rev_file_cache_);
	// go back for minus rev
	else if (rev < 0) {
		rev = rev + convert<int>(rev_file_cache_);
		if (rev < 1)
			return false;
	}

	string revname = convert<string>(rev);
	FileName tmpf = FileName::tempName("lyxvcrev_" + revname + "_");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("svn cat -r " + revname + " "
	              + quoteName(onlyFileName(owner_->absFileName()))
		      + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));
	if (tmpf.isFileEmpty())
		return false;

	f = tmpf.absFileName();
	return true;
}


bool SVN::prepareFileRevisionEnabled()
{
	return true;
}



bool SVN::toggleReadOnlyEnabled()
{
	return false;
}


} // namespace lyx
