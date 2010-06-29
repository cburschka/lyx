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
	return one.startscript(Systemcall::Wait, cmd, false);
}


int VCS::doVCCommand(string const & cmd, FileName const & path)
{
	if (owner_)
		owner_->setBusy(true);

	int const ret = doVCCommandCall(cmd, path);

	if (owner_)
		owner_->setBusy(false);
	if (ret)
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
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool RCS::lockingToggleEnabled()
{
	return false;
}


void RCS::revert()
{
	doVCCommand("co -f -u" + version_ + " "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
	// We ignore changes and just reload!
	owner_->markClean();
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
	return string();
}


bool RCS::prepareFileRevision(string const &revis, string & f)
{
	string rev = revis;

	if (isStrInt(rev)) {
		int back = convert<int>(rev);
		if (back > 0)
			return false;
		if (back == 0)
			rev = version_;
		// we care about the last number from revision string
		// in case of backward indexing
		if (back < 0) {
			string cur, base;
			cur = rsplit(version_, base , '.' );
			if (!isStrInt(cur))
				return false;
			int want = convert<int>(cur) + back;
			if (want <= 0)
				return false;

			rev = base + "." + convert<string>(want);
		}
	}

	FileName tmpf = FileName::tempName("lyxvcrev");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
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
	string tmpf = '/' + onlyFileName(file_.absFileName()) + '/';
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
			// FIXME: must double check file is stattable/existing
			time_t mod = file_.lastModified();
			string mod_date = rtrim(asctime(gmtime(&mod)), "\n");
			LYXERR(Debug::LYXVC, "Date in Entries: `" << file_date
				<< "'\nModification date of file: `" << mod_date << '\'');
			//FIXME this whole locking bussiness is not working under cvs and the machinery
			// conforms to the ci usage, not cvs.
			if (file_date == mod_date) {
				locker_ = "Unlocked";
				vcstatus = UNLOCKED;
			} else {
				// Here we should also do some more checking
				// to see if there are conflicts or not.
				locker_ = "Locked";
				vcstatus = LOCKED;
			}
			break;
		}
	}
}


void CVS::registrer(string const & msg)
{
	doVCCommand("cvs -q add -m \"" + msg + "\" "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
}


string CVS::checkIn(string const & msg)
{
	int ret = doVCCommand("cvs -q commit -m \"" + msg + "\" "
		    + quoteName(onlyFileName(owner_->absFileName())),
		    FileName(owner_->filePath()));
	return ret ? string() : "CVS: Proceeded";
}


bool CVS::checkInEnabled()
{
	return true;
}


string CVS::checkOut()
{
	// cvs update or perhaps for cvs this should be a noop
	// we need to detect conflict (eg "C" in output)
	// before we can do this.
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool CVS::checkOutEnabled()
{
	return false;
}


string CVS::repoUpdate()
{
	lyxerr << "Sorry, not implemented." << endl;
	return string();
}


bool CVS::repoUpdateEnabled()
{
	return false;
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
	string const fil = quoteName(onlyFileName(owner_->absFileName()));
	// This is sensitive operation, so at lest some check about
	// existence of cvs program and its file
	if (doVCCommand("cvs log "+ fil, FileName(owner_->filePath())))
		return;
	FileName f(owner_->absFileName());
	f.removeFile();
	doVCCommand("cvs update " + fil,
		    FileName(owner_->filePath()));
	owner_->markClean();
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
	doVCCommand("cvs log " + quoteName(onlyFileName(owner_->absFileName()))
		    + " > " + quoteName(tmpf.toFilesystemEncoding()),
		    FileName(owner_->filePath()));
}


bool CVS::toggleReadOnlyEnabled()
{
	return false;
}


string CVS::revisionInfo(LyXVC::RevisionInfo const info)
{
	if (info == LyXVC::File)
		return version_;
	return string();
}


bool CVS::prepareFileRevision(string const &, string &)
{
	return false;
}


bool CVS::prepareFileRevisionEnabled()
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
	// First we look for the .svn/entries in the same dir
	// where we have file.
	FileName const entries(onlyPath(file.absFileName()) + "/.svn/entries");
	string const tmpf = onlyFileName(file.absFileName());
	LYXERR(Debug::LYXVC, "LyXVC: Checking if file is under svn in `" << entries
			     << "' for `" << tmpf << '\'');
	if (entries.isReadableFile()) {
		// Ok we are at least in a SVN dir. Parse the .svn/entries
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


void SVN::revert()
{
	// Reverts to the version in CVS repository and
	// gets the updated version from the repository.
	string const fil = quoteName(onlyFileName(owner_->absFileName()));

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
		return N_("Error: Could not generate logfile.");
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
		return N_("Error: Could not generate logfile.");
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

	FileName tmpf = FileName::tempName("lyxvcrev");
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return N_("Error: Could not generate logfile.");
	}

	doVCCommand("svn cat -r " + convert<string>(rev) + " "
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
