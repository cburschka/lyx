#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <fstream>

#include "vc-backend.h"
#include "debug.h"
#include "support/FileInfo.h"
#include "support/LRegex.h"
#include "support/LSubstring.h"
#include "support/path.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h"


int VCS::doVCCommand(string const & cmd)
{
	lyxerr[Debug::LYXVC] << "doVCCommand: " << cmd << endl;
        Systemcalls one;
	Path p(owner_->filepath);
	int ret = one.startscript(Systemcalls::System, cmd);
	return ret;
}


RCS::RCS(string const & m)
{
	master_ = m;
	scanMaster();
}


string RCS::find_file(string const & file)
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
				vcstat = UNLOCKED;
				continue;
			}
			string tmpt, s1, s2;
			do {
				ifs >> tmpt;
				s1 = strip(tmpt, ';');
				// tmp is now in the format <user>:<version>
				s1 = split(s1, s2, ':');
				// s2 is user, and s1 is version
				if (s1 == version_) {
					locker_ = s2;
					vcstat = LOCKED;
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
	version_ = "RCS: " + version_;
}


void RCS::registrer(string const & msg)
{
	string cmd = "ci -q -u -i -t-\"";
	cmd += msg;
	cmd += "\" \"";
	cmd += OnlyFilename(owner_->getFileName());
	cmd += "\"";
	doVCCommand(cmd);
	owner_->getUser()->owner()->getLyXFunc()->Dispatch("buffer-reload");
}


void RCS::checkIn(string const & msg)
{
	doVCCommand("ci -q -u -m\"" + msg + "\" \""
		    + OnlyFilename(owner_->getFileName()) + "\"");
	owner_->getUser()->owner()->getLyXFunc()->Dispatch("buffer-reload");
}


void RCS::checkOut()
{
	owner_->markLyxClean();
	doVCCommand("co -q -l \""
		    + OnlyFilename(owner_->getFileName()) + "\"");
	owner_->getUser()->owner()->getLyXFunc()->Dispatch("buffer-reload");
}


void RCS::revert()
{
	doVCCommand("co -f -u" + version() + " \""
		    + OnlyFilename(owner_->getFileName()) + "\"");
	// We ignore changes and just reload!
	owner_->markLyxClean();
	owner_->getUser()->owner()
		->getLyXFunc()->Dispatch("buffer-reload");
}


void RCS::undoLast()
{
	lyxerr[Debug::LYXVC] << "LyXVC: undoLast" << endl;
	doVCCommand("rcs -o" + version() + " \""
		    + OnlyFilename(owner_->getFileName()) + "\"");
}


void RCS::getLog(string const & tmpf)
{
	doVCCommand("rlog \""
		    + OnlyFilename(owner_->getFileName()) + "\" > " + tmpf);
}


CVS::CVS(string const & m, string const & f)
{
	master_ = m;
	file_ = f;
	scanMaster();
}


string CVS::find_file(string const & file)
{
	// First we look for the CVS/Entries in the same dir
	// where we have file.
	string dir = OnlyPath(file);
	string tmpf = "/" + OnlyFilename(file) + "/";
	dir += "/CVS/Entries";
	lyxerr[Debug::LYXVC] << "LyXVC: checking in `" << dir
			     << "' for `" << tmpf << "'" << endl;
	FileInfo f(dir);
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
	while(getline(ifs, line)) {
		lyxerr[Debug::LYXVC] << "\t  line: " << line << endl;
		if (contains(line, tmpf)) {
			// Ok extract the fields.
			LRegex reg("/(.*)/(.*)/(.*)/(.*)/(.*)");
			LRegex::SubMatches const & sm = reg.exec(line);
			//sm[0]; // whole matched string
			//sm[1]; // filename
			version_ = "CVS: ";
			version_ += LSubstring(line, sm[2].first,
					      sm[2].second);
			string file_date = LSubstring(line, sm[3].first,
						      sm[3].second);
			//sm[4]; // options
			//sm[5]; // tag or tagdate
			FileInfo fi(file_);
			time_t mod = fi.getModificationTime();
			string mod_date = strip(asctime(gmtime(&mod)), '\n');
			lyxerr[Debug::LYXVC]
				<<  "Date in Entries: `" << file_date
				<< "'\nModification date of file: `"
				<< mod_date << "'" << endl;
			if (file_date == mod_date) {
				locker_ = "Unlocked";
				vcstat = UNLOCKED;
			} else {
				// Here we should also to some more checking
				// to see if there are conflicts or not.
				locker_ = "Locked";
				vcstat = LOCKED;
			}
#if 0			
			for (LRegex::SubMatches::const_iterator cit = sm.begin();
			     cit != sm.end(); ++cit) {
				if ((*cit).first != string::npos)
					lyxerr << string(line, (*cit).first,
							 (*cit).second) << endl;
			}
#endif
			break;
		}
	}
}


void CVS::registrer(string const & msg)
{
	// cvs add
}


void CVS::checkIn(string const & msg)
{
	// cvs commit
}


void CVS::checkOut()
{
	// cvs update
}


void CVS::revert()
{
	// not sure how to do this...
	// rm file
	// cvs update  
}


void CVS::undoLast()
{
	// merge the current with the previous version
	// in a reverse patch kind of way, so that the
	// result is to revert the last changes.
}


void CVS::getLog(string const &)
{
}
