#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "vc-backend.h"
#include "debug.h"
#include "buffer.h"
#include "BufferView.h"
#include "LyXView.h"
#include "lyxfunc.h"

#include "support/FileInfo.h"
#include "support/LRegex.h"
#include "support/LSubstring.h"
#include "support/path.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/systemcall.h"

#include <fstream>

using std::endl;
using std::ifstream;
using std::getline;

int VCS::doVCCommand(string const & cmd, string const & path)
{
	lyxerr[Debug::LYXVC] << "doVCCommand: " << cmd << endl;
        Systemcall one;
	Path p(path);
	int const ret = one.startscript(Systemcall::Wait, cmd);
	return ret;
}


RCS::RCS(string const & m)
{
	master_ = m;
	scanMaster();
}


string const RCS::find_file(string const & file)
{
	string tmp(file);
	// Check if *,v exists.
	tmp += ",v";
	FileInfo f;
	lyxerr[Debug::LYXVC] << "Checking if file is under rcs: "
			     << tmp << endl;
	if (f.newFile(tmp).readable()) {
		lyxerr[Debug::LYXVC] << "Yes " << file
				     << " is under rcs." << endl;
		return tmp;
	} else {
		// Check if RCS/*,v exists.
		tmp = AddName(AddPath(OnlyPath(file), "RCS"), file);
		tmp += ",v";
		lyxerr[Debug::LYXVC] << "Checking if file is under rcs: "
				     << tmp << endl;
		if (f.newFile(tmp).readable()) {
			lyxerr[Debug::LYXVC] << "Yes " << file
					     << " it is under rcs."<< endl;
			return tmp;
		}
	}
	return string();
}


void RCS::retrieve(string const & file)
{
	lyxerr[Debug::LYXVC] << "LyXVC::RCS: retrieve.\n\t" << file << endl;
	VCS::doVCCommand("co -q -r \""
			 + file + "\"",
			 string());
}


void RCS::scanMaster()
{
	lyxerr[Debug::LYXVC] << "LyXVC::RCS: scanMaster." << endl;

	ifstream ifs(master_.c_str());

	string token;
	bool read_enough = false;

	while (!read_enough && ifs >> token) {
		lyxerr[Debug::LYXVC]
			<< "LyXVC::scanMaster: current lex text: `"
			<< token << "'" << endl;

		if (token.empty())
			continue;
		else if (token == "head") {
			// get version here
			string tmv;
			ifs >> tmv;
			tmv = strip(tmv, ';');
			version_ = tmv;
			lyxerr[Debug::LYXVC] << "LyXVC: version found to be "
					     << tmv << endl;
		} else if (contains(token, "access")
			   || contains(token, "symbols")
			   || contains(token, "strict")) {
			// nothing
		} else if (contains(token, "locks")) {
			// get locker here
			if (contains(token, ";")) {
				locker_ = "Unlocked";
				vcstatus = UNLOCKED;
				continue;
			}
			string tmpt;
			string s1;
			string s2;
			do {
				ifs >> tmpt;
				s1 = strip(tmpt, ';');
				// tmp is now in the format <user>:<version>
				s1 = split(s1, s2, ':');
				// s2 is user, and s1 is version
				if (s1 == version_) {
					locker_ = s2;
					vcstatus = LOCKED;
					break;
				}
			} while (!contains(tmpt, ";"));
			
		} else if (token == "comment") {
			// we don't need to read any further than this.
			read_enough = true;
		} else {
			// unexpected
			lyxerr[Debug::LYXVC]
				<< "LyXVC::scanMaster(): unexpected token"
				<< endl;
		}
	}
}


void RCS::registrer(string const & msg)
{
	string cmd = "ci -q -u -i -t-\"";
	cmd += msg;
	cmd += "\" \"";
	cmd += OnlyFilename(owner_->fileName());
	cmd += "\"";
	doVCCommand(cmd, owner_->filePath());
	owner_->getUser()->owner()->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void RCS::checkIn(string const & msg)
{
	doVCCommand("ci -q -u -m\"" + msg + "\" \""
		    + OnlyFilename(owner_->fileName()) + "\"", owner_->filePath());
	owner_->getUser()->owner()->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void RCS::checkOut()
{
	owner_->markLyxClean();
	doVCCommand("co -q -l \""
		    + OnlyFilename(owner_->fileName()) + "\"", owner_->filePath());
	owner_->getUser()->owner()->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void RCS::revert()
{
	doVCCommand("co -f -u" + version() + " \""
		    + OnlyFilename(owner_->fileName()) + "\"", owner_->filePath());
	// We ignore changes and just reload!
	owner_->markLyxClean();
	owner_->getUser()->owner()
		->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void RCS::undoLast()
{
	lyxerr[Debug::LYXVC] << "LyXVC: undoLast" << endl;
	doVCCommand("rcs -o" + version() + " \""
		    + OnlyFilename(owner_->fileName()) + "\"",
		    owner_->filePath());
}


void RCS::getLog(string const & tmpf)
{
	doVCCommand("rlog \""
		    + OnlyFilename(owner_->fileName()) + "\" > " + tmpf, owner_->filePath());
}


CVS::CVS(string const & m, string const & f)
{
	master_ = m;
	file_ = f;
	scanMaster();
}


string const CVS::find_file(string const & file)
{
	// First we look for the CVS/Entries in the same dir
	// where we have file.
	string const dir = OnlyPath(file) + "/CVS/Entries";
	string const tmpf = "/" + OnlyFilename(file) + "/";
	lyxerr[Debug::LYXVC] << "LyXVC: checking in `" << dir
			     << "' for `" << tmpf << "'" << endl;
	FileInfo const f(dir);
	if (f.readable()) {
		// Ok we are at least in a CVS dir. Parse the CVS/Entries
		// and see if we can find this file. We do a fast and
		// dirty parse here.
		ifstream ifs(dir.c_str());
		string line;
		while (getline(ifs, line)) {
			lyxerr[Debug::LYXVC] << "\tEntries: " << line << endl;
			if (contains(line, tmpf)) return dir;
		}
	}
	return string();
}


void CVS::scanMaster()
{
	lyxerr[Debug::LYXVC] << "LyXVC::CVS: scanMaster. \n     Checking: "
			     << master_ << endl;
	// Ok now we do the real scan...
	ifstream ifs(master_.c_str());
	string tmpf = "/" + OnlyFilename(file_) + "/";
	lyxerr[Debug::LYXVC] << "\tlooking for `" << tmpf << "'" << endl;
	string line;
	LRegex reg("/(.*)/(.*)/(.*)/(.*)/(.*)");
	while (getline(ifs, line)) {
		lyxerr[Debug::LYXVC] << "\t  line: " << line << endl;
		if (contains(line, tmpf)) {
			// Ok extract the fields.
			LRegex::SubMatches const & sm = reg.exec(line);
			//sm[0]; // whole matched string
			//sm[1]; // filename
			version_ = LSubstring(line, sm[2].first,
					      sm[2].second);
			string file_date = LSubstring(line, sm[3].first,
						      sm[3].second);
			//sm[4]; // options
			//sm[5]; // tag or tagdate
			FileInfo fi(file_);
			// FIXME: must double check file is stattable/existing
			time_t mod = fi.getModificationTime();
			string mod_date = strip(asctime(gmtime(&mod)), '\n');
			lyxerr[Debug::LYXVC]
				<<  "Date in Entries: `" << file_date
				<< "'\nModification date of file: `"
				<< mod_date << "'" << endl;
			if (file_date == mod_date) {
				locker_ = "Unlocked";
				vcstatus = UNLOCKED;
			} else {
				// Here we should also to some more checking
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
	doVCCommand("cvs -q add -m \"" + msg + "\" \""
		    + OnlyFilename(owner_->fileName()) + "\"", owner_->filePath());
	owner_->getUser()->owner()->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void CVS::checkIn(string const & msg)
{
	doVCCommand("cvs -q commit -m \"" + msg + "\" \""
		    + OnlyFilename(owner_->fileName()) + "\"",
		    owner_->filePath());
	owner_->getUser()->owner()->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void CVS::checkOut()
{
	// cvs update or perhaps for cvs this should be a noop
	lyxerr << "Sorry not implemented." << endl;
}


void CVS::revert()
{
	// Reverts to the version in CVS repository and
	// gets the updated version from the repository.
	string const fil = OnlyFilename(owner_->fileName());
	
	doVCCommand("rm -f \"" + fil + "\"; cvs update \"" + fil + "\"",
		    owner_->filePath());
	owner_->markLyxClean();
	owner_->getUser()->owner()
		->getLyXFunc()->dispatch(LFUN_MENURELOAD);
}


void CVS::undoLast()
{
	// merge the current with the previous version
	// in a reverse patch kind of way, so that the
	// result is to revert the last changes.
	lyxerr << "Sorry not implemented." << endl;
}


void CVS::getLog(string const & tmpf)
{
	doVCCommand("cvs log \""
		    + OnlyFilename(owner_->fileName()) + "\" > " + tmpf,
		    owner_->filePath());
}
