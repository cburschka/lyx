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
#include "DispatchResult.h"
#include "LyX.h"
#include "FuncRequest.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"
#include "support/regex.h"
#include "support/TempFile.h"

#include <fstream>

using namespace std;
using namespace lyx::support;


namespace lyx {


int VCS::doVCCommandCall(string const & cmd, FileName const & path)
{
	LYXERR(Debug::LYXVC, "doVCCommandCall: " << cmd);
	Systemcall one;
	support::PathChanger p(path);
	return one.startscript(Systemcall::Wait, cmd, string(), string(), false);
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
			bformat(_("Some problem occurred while running the command:\n"
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
			rsplit(version, base , '.');
			rev = base + '.' + rev;
		}
		if (back == 0)
			rev = version;
		// we care about the last number from revision string
		// in case of backward indexing
		if (back < 0) {
			string cur, base;
			cur = rsplit(version, base , '.');
			if (!isStrInt(cur))
				return false;
			int want = convert<int>(cur) + back;
			if (want <= 0)
				return false;

			rev = base + '.' + convert<string>(want);
		}
	}

	revis = rev;
	return true;
}


bool VCS::checkparentdirs(FileName const & file, std::string const & vcsdir)
{
	FileName dirname = file.onlyPath();
	do {
		FileName tocheck = FileName(addName(dirname.absFileName(), vcsdir));
		LYXERR(Debug::LYXVC, "check file: " << tocheck.absFileName());
		if (tocheck.exists())
			return true;
		//this construct because of #8295
		dirname = FileName(dirname.absFileName()).parentPath();
	} while (!dirname.empty());
	return false;
}


/////////////////////////////////////////////////////////////////////
//
// RCS
//
/////////////////////////////////////////////////////////////////////

RCS::RCS(FileName const & m, Buffer * b) : VCS(b)
{
	// Here we know that the buffer file is either already in RCS or
	// about to be registered
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


bool RCS::retrieve(FileName const & file)
{
	LYXERR(Debug::LYXVC, "LyXVC::RCS: retrieve.\n\t" << file);
	// The caller ensures that file does not exist, so no need to check that.
	return doVCCommandCall("co -q -r " + quoteName(file.toFilesystemEncoding()),
	                       FileName()) == 0;
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


bool RCS::renameEnabled()
{
	return false;
}


string RCS::rename(support::FileName const & /*newFile*/, string const & /*msg*/)
{
	// not implemented, since a left-over file.lyx,v would be confusing.
	return string();
}


bool RCS::copyEnabled()
{
	return true;
}


string RCS::copy(support::FileName const & newFile, string const & msg)
{
	// RCS has no real copy command, so we create a poor mans version
	support::FileName const oldFile(owner_->absFileName());
	if (!oldFile.copyTo(newFile))
		return string();
	FileName path(oldFile.onlyPath());
	string relFile(to_utf8(newFile.relPath(path.absFileName())));
	string cmd = "ci -q -u -i -t-\"";
	cmd += msg;
	cmd += "\" ";
	cmd += quoteName(relFile);
	return doVCCommand(cmd, path) ? string() : "RCS: Proceeded";
}


LyXVC::CommandResult RCS::checkIn(string const & msg, string & log)
{
	int ret = doVCCommand("ci -q -u -m\"" + msg + "\" "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
	if (ret)
		return LyXVC::ErrorCommand;
	log = "RCS: Proceeded";
	return LyXVC::VCSuccess;
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

	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return true;
	}

	doVCCommandCall("rcsdiff " + quoteName(owner_->absFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));

	docstring diff = tmpf.fileContents("UTF-8");

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
	if (doVCCommand("co -f -u" + version_ + ' '
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
	doVCCommand("rcs -o" + version_ + ' '
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
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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

	TempFile tempfile("lyxvcrev_" + rev + '_');
	tempfile.setAutoRemove(false);
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("co -p" + rev + ' '
	              + quoteName(onlyFileName(owner_->absFileName()))
		      + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));
	tmpf.refresh();
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

CVS::CVS(FileName const & m, Buffer * b) : VCS(b)
{
	// Here we know that the buffer file is either already in CVS or
	// about to be registered
	master_ = m;
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
	string name = onlyFileName(owner_->absFileName());
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
			FileName file(owner_->absFileName());
			if (file.isReadableFile()) {
				time_t mod = file.lastModified();
				string mod_date = rtrim(asctime(gmtime(&mod)), "\n");
				LYXERR(Debug::LYXVC, "Date in Entries: `" << file_date
					<< "'\nModification date of file: `" << mod_date << '\'');
				if (file.isReadOnly()) {
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


bool CVS::retrieve(FileName const & file)
{
	LYXERR(Debug::LYXVC, "LyXVC::CVS: retrieve.\n\t" << file);
	// The caller ensures that file does not exist, so no need to check that.
	return doVCCommandCall("cvs -q update " + quoteName(file.toFilesystemEncoding()),
	                       file.onlyPath()) == 0;
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
	return docstring();
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
	TempFile tempfile("lyxvout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return StatusError;
	}

	if (doVCCommandCallWithOutput("cvs status " + getTarget(File),
		FileName(owner_->filePath()), tmpf)) {
		return StatusError;
	}

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	CvsStatus status = NoCvsFile;

	while (ifs) {
		string line;
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line << '\n');
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
	return status;
}

void CVS::getRevisionInfo()
{
	if (have_rev_info_)
		return;
	have_rev_info_ = true;
	TempFile tempfile("lyxvout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return;
	}

	int rc = doVCCommandCallWithOutput("cvs log -r" + version_
		+ ' ' + getTarget(File),
		FileName(owner_->filePath()), tmpf);
	if (rc) {
		LYXERR(Debug::LYXVC, "cvs log failed with exit code " << rc);
		return;
	}

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	static regex const reg("date: (.*) (.*) (.*);  author: (.*);  state: (.*);(.*)");

	while (ifs) {
		string line;
		getline(ifs, line);
		LYXERR(Debug::LYXVC, line << '\n');
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


bool CVS::renameEnabled()
{
	return true;
}


string CVS::rename(support::FileName const & newFile, string const & msg)
{
	// CVS has no real rename command, so we create a poor mans version
	support::FileName const oldFile(owner_->absFileName());
	string ret = copy(newFile, msg);
	if (ret.empty())
		return ret;
	string cmd = "cvs -q remove -m \"" + msg + "\" " +
		quoteName(oldFile.onlyFileName());
	FileName path(oldFile.onlyPath());
	return doVCCommand(cmd, path) ? string() : ret;
}


bool CVS::copyEnabled()
{
	return true;
}


string CVS::copy(support::FileName const & newFile, string const & msg)
{
	// CVS has no real copy command, so we create a poor mans version
	support::FileName const oldFile(owner_->absFileName());
	if (!oldFile.copyTo(newFile))
		return string();
	FileName path(oldFile.onlyPath());
	string relFile(to_utf8(newFile.relPath(path.absFileName())));
	string cmd("cvs -q add -m \"" + msg + "\" " + quoteName(relFile));
	return doVCCommand(cmd, path) ? string() : "CVS: Proceeded";
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
		LYXERR(Debug::LYXVC, line << '\n');
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


LyXVC::CommandResult CVS::checkIn(string const & msg, string & log)
{
	CvsStatus status = getStatus();
	switch (status) {
	case UpToDate:
		if (vcstatus != NOLOCKING)
			if (unedit())
				return LyXVC::ErrorCommand;
		log = "CVS: Proceeded";
		return LyXVC::VCSuccess;
	case LocallyModified:
	case LocallyAdded: {
		int rc = doVCCommand("cvs -q commit -m \"" + msg + "\" "
			+ getTarget(File),
		    FileName(owner_->filePath()));
		if (rc)
			return LyXVC::ErrorCommand;
		log = "CVS: Proceeded";
		return LyXVC::VCSuccess;
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
	return LyXVC::ErrorBefore;
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
	TempFile tempfile("lyxvout");
	FileName tmpf = tempfile.name();
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
	TempFile tempfile("lyxvout");
	FileName tmpf = tempfile.name();
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
		if (ret == 2) {
			dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + tmpf.absFileName()));
			ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Continue"), _("&Abort"));
			hideDialogs("file", 0);
		}
		if (ret == 1)
			return string();
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

	TempFile tempfile("lyxvcrev_" + rev + '_');
	tempfile.setAutoRemove(false);
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommandWithOutput("cvs update -p -r" + rev + ' '
		+ getTarget(File),
		FileName(owner_->filePath()), tmpf);
	tmpf.refresh();
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

SVN::SVN(FileName const & m, Buffer * b) : VCS(b)
{
	// Here we know that the buffer file is either already in SVN or
	// about to be registered
	master_ = m;
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
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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
	// vcstatus code is somewhat superflous,
	// until we want to implement read-only toggle for svn.
	vcstatus = NOLOCKING;
	if (checkLockMode()) {
		if (isLocked())
			vcstatus = LOCKED;
		else
			vcstatus = UNLOCKED;
	}
}


bool SVN::checkLockMode()
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()){
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	LYXERR(Debug::LYXVC, "Detecting locking mode...");
	if (doVCCommandCall("svn proplist " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath())))
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
	FileName file(owner_->absFileName());
	file.refresh();
	return !file.isReadOnly();
}


bool SVN::retrieve(FileName const & file)
{
	LYXERR(Debug::LYXVC, "LyXVC::SVN: retrieve.\n\t" << file);
	// The caller ensures that file does not exist, so no need to check that.
	return doVCCommandCall("svn update -q --non-interactive " + quoteName(file.onlyFileName()),
	                       file.onlyPath()) == 0;
}


void SVN::registrer(string const & /*msg*/)
{
	doVCCommand("svn add -q " + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
}


bool SVN::renameEnabled()
{
	return true;
}


string SVN::rename(support::FileName const & newFile, string const & msg)
{
	// svn move does not require a log message, since it does not commit.
	// In LyX we commit immediately afterwards, otherwise it could be
	// confusing to the user to have two uncommitted files.
	FileName path(owner_->filePath());
	string relFile(to_utf8(newFile.relPath(path.absFileName())));
	string cmd("svn move -q " + quoteName(onlyFileName(owner_->absFileName())) +
	           ' ' + quoteName(relFile));
	if (doVCCommand(cmd, path)) {
		cmd = "svn revert -q " +
			quoteName(onlyFileName(owner_->absFileName())) + ' ' +
			quoteName(relFile);
		doVCCommand(cmd, path);
		if (newFile.exists())
			newFile.removeFile();
		return string();
	}
	vector<support::FileName> f;
	f.push_back(owner_->fileName());
	f.push_back(newFile);
	string log;
	if (checkIn(f, msg, log) != LyXVC::VCSuccess) {
		cmd = "svn revert -q " +
			quoteName(onlyFileName(owner_->absFileName())) + ' ' +
			quoteName(relFile);
		doVCCommand(cmd, path);
		if (newFile.exists())
			newFile.removeFile();
		return string();
	}
	return log;
}


bool SVN::copyEnabled()
{
	return true;
}


string SVN::copy(support::FileName const & newFile, string const & msg)
{
	// svn copy does not require a log message, since it does not commit.
	// In LyX we commit immediately afterwards, otherwise it could be
	// confusing to the user to have an uncommitted file.
	FileName path(owner_->filePath());
	string relFile(to_utf8(newFile.relPath(path.absFileName())));
	string cmd("svn copy -q " + quoteName(onlyFileName(owner_->absFileName())) +
	           ' ' + quoteName(relFile));
	if (doVCCommand(cmd, path))
		return string();
	vector<support::FileName> f(1, newFile);
	string log;
	if (checkIn(f, msg, log) == LyXVC::VCSuccess)
		return log;
	return string();
}


LyXVC::CommandResult SVN::checkIn(string const & msg, string & log)
{
	vector<support::FileName> f(1, owner_->fileName());
	return checkIn(f, msg, log);
}


LyXVC::CommandResult
SVN::checkIn(vector<support::FileName> const & f, string const & msg, string & log)
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()){
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		log = N_("Error: Could not generate logfile.");
		return LyXVC::ErrorBefore;
	}

	ostringstream os;
	os << "svn commit -m \"" << msg << '"';
	for (size_t i = 0; i < f.size(); ++i)
		os << ' ' << quoteName(f[i].onlyFileName());
	os << " > " << quoteName(tmpf.toFilesystemEncoding());
	LyXVC::CommandResult ret =
		doVCCommand(os.str(), FileName(owner_->filePath())) ?
			LyXVC::ErrorCommand : LyXVC::VCSuccess;

	string res = scanLogFile(tmpf, log);
	if (!res.empty()) {
		frontend::Alert::error(_("Revision control error."),
				_("Error when committing to repository.\n"
				"You have to manually resolve the problem.\n"
				"LyX will reopen the document after you press OK."));
		ret = LyXVC::ErrorCommand;
	}
	else
		if (!fileLock(false, tmpf, log))
			ret = LyXVC::ErrorCommand;

	if (!log.empty())
		log.insert(0, "SVN: ");
	if (ret == LyXVC::VCSuccess && log.empty())
		log = "SVN: Proceeded";
	return ret;
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

	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return true;
	}

	doVCCommandCall("svn diff " + quoteName(owner_->absFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));

	docstring diff = tmpf.fileContents("UTF-8");

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
		LYXERR(Debug::LYXVC, line << '\n');
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


bool SVN::fileLock(bool lock, FileName const & tmpf, string &status)
{
	if (!locked_mode_ || (isLocked() == lock))
		return true;

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

	if (isLocked() == lock)
		return true;

	if (lock)
		frontend::Alert::error(_("Revision control error."),
			_("Error while acquiring write lock.\n"
			"Another user is most probably editing\n"
			"the current document now!\n"
			"Also check the access to the repository."));
	else
		frontend::Alert::error(_("Revision control error."),
			_("Error while releasing write lock.\n"
			"Check the access to the repository."));
	return false;
}


string SVN::checkOut()
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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
		if (ret == 2) {
			dispatch(FuncRequest(LFUN_DIALOG_SHOW, "file " + tmpf.absFileName()));
			ret = frontend::Alert::prompt(_("Changes detected"),
				text, 0, 1, _("&Yes"), _("&No"));
			hideDialogs("file", 0);
		}
		if (ret == 1)
			return string();
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
	return to_utf8(res);
}


bool SVN::repoUpdateEnabled()
{
	return true;
}


string SVN::lockingToggle()
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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

	frontend::Alert::warning(_("SVN File Locking"),
		(locking ? _("Locking property unset.") : _("Locking property set.")) + '\n'
		+ _("Do not forget to commit the locking property into the repository."),
		true);

	return string("SVN: ") + (locking ?
		N_("Locking property unset.") : N_("Locking property set."));
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
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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
	return !rev.empty();
}


bool SVN::getTreeRevisionInfo()
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
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
	TempFile tempfile("lyxvcrev_" + revname + '_');
	tempfile.setAutoRemove(false);
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("svn cat -r " + revname + ' '
	              + quoteName(onlyFileName(owner_->absFileName()))
		      + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));
	tmpf.refresh();
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


/////////////////////////////////////////////////////////////////////
//
// GIT
//
/////////////////////////////////////////////////////////////////////

GIT::GIT(FileName const & m, Buffer * b) : VCS(b)
{
	// Here we know that the buffer file is either already in GIT or
	// about to be registered
	master_ = m;
	scanMaster();
}


FileName const GIT::findFile(FileName const & file)
{
	// First we check the existence of repository meta data.
	if (!VCS::checkparentdirs(file, ".git")) {
		LYXERR(Debug::LYXVC, "Cannot find GIT meta data for " << file);
		return FileName();
	}

	// Now we check the status of the file.
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return FileName();
	}

	string const fname = onlyFileName(file.absFileName());
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under git control for `"
			<< fname << '\'');
	doVCCommandCall("git ls-files " +
			quoteName(fname) + " > " +
			quoteName(tmpf.toFilesystemEncoding()),
			file.onlyPath());
	tmpf.refresh();
	bool found = !tmpf.isFileEmpty();
	LYXERR(Debug::LYXVC, "GIT control: " << (found ? "enabled" : "disabled"));
	return found ? file : FileName();
}


void GIT::scanMaster()
{
	// vcstatus code is somewhat superflous,
	// until we want to implement read-only toggle for git.
	vcstatus = NOLOCKING;
}


bool GIT::retrieve(FileName const & file)
{
	LYXERR(Debug::LYXVC, "LyXVC::GIT: retrieve.\n\t" << file);
	// The caller ensures that file does not exist, so no need to check that.
	return doVCCommandCall("git checkout -q " + quoteName(file.onlyFileName()),
	                       file.onlyPath()) == 0;
}


void GIT::registrer(string const & /*msg*/)
{
	doVCCommand("git add " + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
}


bool GIT::renameEnabled()
{
	return true;
}


string GIT::rename(support::FileName const & newFile, string const & msg)
{
	// git mv does not require a log message, since it does not commit.
	// In LyX we commit immediately afterwards, otherwise it could be
	// confusing to the user to have two uncommitted files.
	FileName path(owner_->filePath());
	string relFile(to_utf8(newFile.relPath(path.absFileName())));
	string cmd("git mv " + quoteName(onlyFileName(owner_->absFileName())) +
	           ' ' + quoteName(relFile));
	if (doVCCommand(cmd, path)) {
		cmd = "git checkout -q " +
			quoteName(onlyFileName(owner_->absFileName())) + ' ' +
			quoteName(relFile);
		doVCCommand(cmd, path);
		if (newFile.exists())
			newFile.removeFile();
		return string();
	}
	vector<support::FileName> f;
	f.push_back(owner_->fileName());
	f.push_back(newFile);
	string log;
	if (checkIn(f, msg, log) != LyXVC::VCSuccess) {
		cmd = "git checkout -q " +
			quoteName(onlyFileName(owner_->absFileName())) + ' ' +
			quoteName(relFile);
		doVCCommand(cmd, path);
		if (newFile.exists())
			newFile.removeFile();
		return string();
	}
	return log;
}


bool GIT::copyEnabled()
{
	return false;
}


string GIT::copy(support::FileName const & /*newFile*/, string const & /*msg*/)
{
	// git does not support copy with history preservation
	return string();
}


LyXVC::CommandResult GIT::checkIn(string const & msg, string & log)
{
	vector<support::FileName> f(1, owner_->fileName());
	return checkIn(f, msg, log);
}


LyXVC::CommandResult
GIT::checkIn(vector<support::FileName> const & f, string const & msg, string & log)
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()){
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		log = N_("Error: Could not generate logfile.");
		return LyXVC::ErrorBefore;
	}

	ostringstream os;
	os << "git commit -m \"" << msg << '"';
	for (size_t i = 0; i < f.size(); ++i)
		os << ' ' << quoteName(f[i].onlyFileName());
	os << " > " << quoteName(tmpf.toFilesystemEncoding());
	LyXVC::CommandResult ret =
		doVCCommand(os.str(), FileName(owner_->filePath())) ?
			LyXVC::ErrorCommand : LyXVC::VCSuccess;

	string res = scanLogFile(tmpf, log);
	if (!res.empty()) {
		frontend::Alert::error(_("Revision control error."),
				_("Error when committing to repository.\n"
				"You have to manually resolve the problem.\n"
				"LyX will reopen the document after you press OK."));
		ret = LyXVC::ErrorCommand;
	}

	if (!log.empty())
		log.insert(0, "GIT: ");
	if (ret == LyXVC::VCSuccess && log.empty())
		log = "GIT: Proceeded";
	return ret;
}


bool GIT::checkInEnabled()
{
	return true;
}


bool GIT::isCheckInWithConfirmation()
{
	// FIXME one day common getDiff and perhaps OpMode for all backends

	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return true;
	}

	doVCCommandCall("git diff " + quoteName(owner_->absFileName())
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));

	docstring diff = tmpf.fileContents("UTF-8");

	if (diff.empty())
		return false;

	return true;
}


// FIXME Correctly return code should be checked instead of this.
// This would need another solution than just plain startscript.
// Hint from Andre': QProcess::readAllStandardError()...
string GIT::scanLogFile(FileName const & f, string & status)
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
	}
	ifs.close();
	return string();
}


string GIT::checkOut()
{
	return string();
}


bool GIT::checkOutEnabled()
{
	return false;
}


string GIT::repoUpdate()
{
	return string();
}


bool GIT::repoUpdateEnabled()
{
	return false;
}


string GIT::lockingToggle()
{
	return string();
}


bool GIT::lockingToggleEnabled()
{
	return false;
}


bool GIT::revert()
{
	// Reverts to the version in GIT repository and
	// gets the updated version from the repository.
	string const fil = quoteName(onlyFileName(owner_->absFileName()));

	if (doVCCommand("git checkout -q " + fil,
		    FileName(owner_->filePath())))
		return false;
	owner_->markClean();
	return true;
}


bool GIT::isRevertWithConfirmation()
{
	//FIXME owner && diff
	return true;
}


void GIT::undoLast()
{
	// merge the current with the previous version
	// in a reverse patch kind of way, so that the
	// result is to revert the last changes.
	lyxerr << "Sorry, not implemented." << endl;
}


bool GIT::undoLastEnabled()
{
	return false;
}


string GIT::revisionInfo(LyXVC::RevisionInfo const info)
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


bool GIT::getFileRevisionInfo()
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("git log -n 1 --pretty=format:%H%n%an%n%ai " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	if (tmpf.empty())
		return false;

	ifstream ifs(tmpf.toFilesystemEncoding().c_str());

	if (ifs)
		getline(ifs, rev_file_cache_);
	if (ifs)
		getline(ifs, rev_author_cache_);
	if (ifs) {
		string line;
		getline(ifs, line);
		rev_time_cache_ = split(line, rev_date_cache_, ' ');
	}

	ifs.close();
	return !rev_file_cache_.empty();
}


bool GIT::getTreeRevisionInfo()
{
	TempFile tempfile("lyxvcout");
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("git describe --abbrev --dirty --long > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));

	if (tmpf.empty())
		return false;

	// only first line in case something bad happens.
	ifstream ifs(tmpf.toFilesystemEncoding().c_str());
	getline(ifs, rev_tree_cache_);
	ifs.close();

	return !rev_tree_cache_.empty();
}


void GIT::getLog(FileName const & tmpf)
{
	doVCCommand("git log " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


//at this moment we don't accept revision SHA, but just number of revision steps back
//GUI and infrastucture needs to be changed first
bool GIT::prepareFileRevision(string const & revis, string & f)
{
	// anything positive means we got hash, not "0" or minus revision
	int rev = 1;

	// hash is rarely number and should be long
	if (isStrInt(revis) && revis.length()<20)
		rev = convert<int>(revis);

	// revision and filename
	string pointer;

	// go back for "minus" revisions
	if (rev <= 0)
		pointer = "HEAD~" + convert<string>(-rev);
	// normal hash
	else
		pointer = revis;

	pointer += ':';

	TempFile tempfile("lyxvcrev_" + revis + '_');
	tempfile.setAutoRemove(false);
	FileName tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return false;
	}

	doVCCommand("git show " + pointer + "./"
	              + quoteName(onlyFileName(owner_->absFileName()))
		      + " > " + quoteName(tmpf.toFilesystemEncoding()),
		FileName(owner_->filePath()));
	tmpf.refresh();
	if (tmpf.isFileEmpty())
		return false;

	f = tmpf.absFileName();
	return true;
}


bool GIT::prepareFileRevisionEnabled()
{
	return true;
}


bool GIT::toggleReadOnlyEnabled()
{
	return true;
}


} // namespace lyx
