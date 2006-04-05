/**
 * \file session.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "session.h"
#include "debug.h"
#include "support/package.h"
#include "support/filetools.h"

#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

using lyx::support::AddName; 
using lyx::support::package;  

namespace fs = boost::filesystem;

using std::vector;
using std::getline;
using std::string;
using std::ifstream;
using std::ofstream;
using std::endl;
using std::istringstream;
using std::copy;
using std::find;
using std::ostream_iterator;

namespace lyx{

namespace {

string const sec_lastfiles = "[recent files]";
string const sec_lastfilepos = "[cursor positions]";
string const sec_lastopened = "[last opened files]";
string const sec_bookmarks = "[bookmarks]";
string const sec_session = "[session info]";
int const id_lastfiles = 0;
int const id_lastfilepos = 1;
int const id_lastopened = 2;
int const id_bookmarks = 3;
int const id_session = 4;

} // anon namespace


Session::Session(unsigned int num) :
	default_num_last_files(4),
	absolute_max_last_files(100),
	num_lastfilepos(100)
{
	setNumberOfLastFiles(num);
	// locate the session file
	// note that the session file name 'session' is hard-coded
	session_file = AddName(package().user_support(), "session");
	//
	readFile();
}


void Session::setNumberOfLastFiles(unsigned int no)
{
	if (0 < no && no <= absolute_max_last_files)
		num_lastfiles = no;
	else {
		lyxerr << "LyX: session: too many last files\n"
		       << "\tdefault (=" << default_num_last_files
		       << ") used." << endl;
		num_lastfiles = default_num_last_files;
	}
}


void Session::readFile()
{
	// we will not complain if we can't find session_file nor will
	// we issue a warning. (Lgb)
	ifstream ifs(session_file.c_str());
	string tmp;
	int section = -1;

	// the following is currently not implemented very
	// robustly. (Manually editing of the session file may crash lyx)
	// 
	while (getline(ifs, tmp)) {
		// Ignore comments, empty line or line stats with ' '
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		// Determine section id
		if (tmp == sec_lastfiles) {
			section = id_lastfiles;
		} else if (tmp == sec_lastfilepos) {
			section = id_lastfilepos;
		} else if (tmp == sec_lastopened) {
			section = id_lastopened;
		} else if (tmp == sec_bookmarks) {
			section = id_bookmarks;
		} else if (tmp == sec_session) {
			section = id_session;
		} else if (section == id_lastfiles) {
			// read lastfiles
			if (!fs::exists(tmp) || lastfiles.size() >= num_lastfiles)
				continue;
			lastfiles.push_back(tmp);
		} else if (section == id_lastfilepos) {
			// read lastfilepos
			// pos, file\n
			lyx::pit_type pit;
			lyx::pos_type pos;
			string fname;
			istringstream itmp(tmp);
			itmp >> pit;
			itmp.ignore(2);  // ignore ", "
			itmp >> pos;
			itmp.ignore(2);  // ignore ", "
			itmp >> fname;
			if (!fs::exists(fname) || lastfilepos.size() >= num_lastfilepos)
				continue;
			lastfilepos[fname] = boost::tie(pit, pos);
		} else if (section == id_lastopened) {
			// read lastopened
			// files
			if (!fs::exists(tmp))
				continue;
			lastopened.push_back(tmp);
		} else if (section == id_bookmarks) {
			// read bookmarks 
			// bookmarkid, id, pos, file\n
			unsigned int num;
			unsigned int id;
			lyx::pos_type pos;
			string fname;
			istringstream itmp(tmp);
			itmp >> num;
			itmp.ignore(2);  // ignore ", "
			itmp >> id;
			itmp.ignore(2);  // ignore ", "
			itmp >> pos;
			itmp.ignore(2);  // ignore ", "
			itmp >> fname;
			// only load valid bookmarks
			if (fs::exists(fname))
				bookmarks.push_back(boost::tie(num, fname, id, pos));
		} else if (section == id_session) {
			// Read session info, saved as key/value pairs
			// would better yell if pos returns npos
			string::size_type pos = tmp.find_first_of(" = ");
			string key = tmp.substr(0, pos);
			string value = tmp.substr(pos + 3);
			sessioninfo[key] = value;
		}
	}
}


void Session::writeFile() const
{
	ofstream ofs(session_file.c_str());
	if (ofs) {
		ofs << "## Automatically generated lyx session file \n"
		    << "## Editing this file manually may cause lyx to crash.\n";
		// first section
		ofs << '\n' << sec_lastfiles << '\n';
		copy(lastfiles.begin(), lastfiles.end(),
		     ostream_iterator<string>(ofs, "\n"));
		// second section
		ofs << '\n' << sec_lastfilepos << '\n';
		for (FilePosMap::const_iterator file = lastfilepos.begin(); 
			file != lastfilepos.end(); ++file) {
			ofs << file->second.get<0>() << ", " 
			    << file->second.get<1>() << ", "
			    << file->first << '\n';
		}
		// third section
		ofs << '\n' << sec_lastopened << '\n';
		copy(lastopened.begin(), lastopened.end(),
		     ostream_iterator<string>(ofs, "\n"));
		// fourth section
		ofs << '\n' << sec_bookmarks << '\n';
		for (BookmarkList::const_iterator bm = bookmarks.begin(); 
			bm != bookmarks.end(); ++bm) {
			// save bookmark number, id, pos, fname
			ofs << bm->get<0>() << ", "
				<< bm->get<2>() << ", " 
				<< bm->get<3>() << ", "
				<< bm->get<1>() << '\n';
		}
		// fifth section
		ofs << '\n' << sec_session << '\n';
		for (MiscInfo::const_iterator val = sessioninfo.begin();
			val != sessioninfo.end(); ++val) {
			ofs << val->first << " = " << val->second << '\n';
		}
	} else
		lyxerr << "LyX: Warning: unable to save Session: "
		       << session_file << endl;
}


void Session::addLastFile(string const & file)
{
	// If file already exist, delete it and reinsert at front.
	LastFiles::iterator it = find(lastfiles.begin(), lastfiles.end(), file);
	if (it != lastfiles.end())
		lastfiles.erase(it);
	lastfiles.push_front(file);
	if (lastfiles.size() > num_lastfiles)
		lastfiles.pop_back();
}


void Session::saveFilePosition(string const & fname, FilePos pos)
{
	lastfilepos[fname] = pos;
}


Session::FilePos Session::loadFilePosition(string const & fname) const
{
	FilePosMap::const_iterator entry = lastfilepos.find(fname);
	// Has position information, return it.
	if (entry != lastfilepos.end())
		return entry->second;
	// Not found, return the first paragraph
	else 
		return 0;
}


void Session::clearLastOpenedFiles()
{
	lastopened.clear();
}


void Session::setLastOpenedFiles(vector<string> const & files)
{
	lastopened = files;
}


void Session::saveBookmark(Bookmark const & bookmark)
{
	bookmarks.push_back(bookmark);
}


void Session::saveSessionInfo(string const & key, string const & value)
{
	sessioninfo[key] = value;
}


string const Session::loadSessionInfo(string const & key, bool release)
{
	MiscInfo::const_iterator pos = sessioninfo.find(key);
	string value;
	if (pos != sessioninfo.end())
		value = pos->second;
	if (release)
		sessioninfo.erase(key);
	return value;
}

}
