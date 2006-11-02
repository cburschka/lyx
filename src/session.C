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

using lyx::support::addName;
using lyx::support::package;

namespace fs = boost::filesystem;

using std::vector;
using std::getline;
using std::string;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::endl;
using std::istringstream;
using std::copy;
using std::find;
using std::ostream_iterator;

namespace {

string const sec_lastfiles = "[recent files]";
string const sec_lastfilepos = "[cursor positions]";
string const sec_lastopened = "[last opened files]";
string const sec_bookmarks = "[bookmarks]";
string const sec_session = "[session info]";
string const sec_toolbars = "[toolbars]";

} // anon namespace


namespace lyx {

LastFilesSection::LastFilesSection(unsigned int num) :
	default_num_last_files(4),
	absolute_max_last_files(100)
{
	setNumberOfLastFiles(num);
}


void LastFilesSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		// read lastfiles
		if (!fs::exists(tmp) || lastfiles.size() >= num_lastfiles)
			continue;
		lastfiles.push_back(tmp);
	} while (is.good());
}


void LastFilesSection::write(ostream & os) const
{
	os << '\n' << sec_lastfiles << '\n';
	copy(lastfiles.begin(), lastfiles.end(),
	     ostream_iterator<string>(os, "\n"));
}


void LastFilesSection::add(string const & file)
{
	// If file already exist, delete it and reinsert at front.
	LastFiles::iterator it = find(lastfiles.begin(), lastfiles.end(), file);
	if (it != lastfiles.end())
		lastfiles.erase(it);
	lastfiles.push_front(file);
	if (lastfiles.size() > num_lastfiles)
		lastfiles.pop_back();
}


void LastFilesSection::setNumberOfLastFiles(unsigned int no)
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


void LastOpenedSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		if (!fs::exists(tmp))
			continue;
		lastopened.push_back(tmp);
	} while (is.good());
}


void LastOpenedSection::write(ostream & os) const
{
	os << '\n' << sec_lastopened << '\n';
	copy(lastopened.begin(), lastopened.end(),
	     ostream_iterator<string>(os, "\n"));
}


void LastOpenedSection::add(string const & file)
{
	lastopened.push_back(file);
}


void LastOpenedSection::clear()
{
	lastopened.clear();
}


void LastFilePosSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		// read lastfilepos
		// pos, file\n
		pit_type pit;
		pos_type pos;
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
	} while (is.good());
}


void LastFilePosSection::write(ostream & os) const
{
	os << '\n' << sec_lastfilepos << '\n';
	for (FilePosMap::const_iterator file = lastfilepos.begin();
		file != lastfilepos.end(); ++file) {
		os << file->second.get<0>() << ", "
		    << file->second.get<1>() << ", "
		    << file->first << '\n';
	}
}


void LastFilePosSection::save(string const & fname, FilePos pos)
{
	lastfilepos[fname] = pos;
}


LastFilePosSection::FilePos LastFilePosSection::load(string const & fname) const
{
	FilePosMap::const_iterator entry = lastfilepos.find(fname);
	// Has position information, return it.
	if (entry != lastfilepos.end())
		return entry->second;
	// Not found, return the first paragraph
	else
		return 0;
}


void BookmarksSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		// read bookmarks
		// id, pos, file\n
		unsigned int id;
		pos_type pos;
		string fname;
		istringstream itmp(tmp);
		itmp >> id;
		itmp.ignore(2);  // ignore ", "
		itmp >> pos;
		itmp.ignore(2);  // ignore ", "
		itmp >> fname;
		// only load valid bookmarks
		if (bookmarks.size() < max_bookmarks && fs::exists(fname))
			bookmarks.push_back(Bookmark(fname, id, pos));
	} while (is.good());
}


void BookmarksSection::write(ostream & os) const
{
	os << '\n' << sec_bookmarks << '\n';
	for (size_t i = 0; i < bookmarks.size(); ++i) {
		os << bookmarks[i].par_id << ", "
		   << bookmarks[i].par_pos << ", "
		   << bookmarks[i].filename << '\n';
	}
}


void BookmarksSection::save(std::string const & fname, int par_id, pos_type par_pos, bool persistent)
{
	if (persistent) {
		bookmarks.push_front(Bookmark(fname, par_id, par_pos));
		if (bookmarks.size() > max_bookmarks)
			bookmarks.pop_back();
		}
	else
		temp_bookmark = Bookmark(fname, par_id, par_pos);
}


bool BookmarksSection::isValid(unsigned int i) const
{
	// i == 0, or in the queue
	return i <= bookmarks.size();
}


BookmarksSection::Bookmark const & BookmarksSection::bookmark(unsigned int i) const
{
	if (i == 0)
		return temp_bookmark;
	else
		return bookmarks[i-1];
}


void ToolbarSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);

		// Read session info, saved as key/value pairs
		// would better yell if pos returns npos
		string::size_type pos = tmp.find_first_of(" = ");
		// silently ignore lines without " = "
		if (pos != string::npos) {
			string key = tmp.substr(0, pos);
			int state;
			int location;
			istringstream value(tmp.substr(pos + 3));
			value >> state;
			value.ignore(1); // ignore " "
			value >> location;
			toolbars[key] = ToolbarInfo(state, location);
		}
	} while (is.good());
}


void ToolbarSection::write(ostream & os) const
{
	os << '\n' << sec_toolbars << '\n';
	for (ToolbarMap::const_iterator tb = toolbars.begin();
		tb != toolbars.end(); ++tb) {
		os << tb->first << " = "
		  << static_cast<int>(tb->second.state) << " "
		  << static_cast<int>(tb->second.location) << '\n';
	}
}


ToolbarSection::ToolbarInfo & ToolbarSection::load(string const & name)
{
	return toolbars[name];
}


void SessionInfoSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);

		// Read session info, saved as key/value pairs
		// would better yell if pos returns npos
		string::size_type pos = tmp.find_first_of(" = ");
		// silently ignore lines without " = "
		if (pos != string::npos) {
			string key = tmp.substr(0, pos);
			string value = tmp.substr(pos + 3);
			sessioninfo[key] = value;
		}
	} while (is.good());
}


void SessionInfoSection::write(ostream & os) const
{
	os << '\n' << sec_session << '\n';
	for (MiscInfo::const_iterator val = sessioninfo.begin();
		val != sessioninfo.end(); ++val) {
		os << val->first << " = " << val->second << '\n';
	}
}


void SessionInfoSection::save(string const & key, string const & value)
{
	sessioninfo[key] = value;
}


string const SessionInfoSection::load(string const & key, bool release)
{
	MiscInfo::const_iterator pos = sessioninfo.find(key);
	string value;
	if (pos != sessioninfo.end())
		value = pos->second;
	if (release)
		sessioninfo.erase(key);
	return value;
}


Session::Session(unsigned int num) :
	last_files(num)
{
	// locate the session file
	// note that the session file name 'session' is hard-coded
	session_file = addName(package().user_support(), "session");
	//
	readFile();
}


void Session::readFile()
{
	// we will not complain if we can't find session_file nor will
	// we issue a warning. (Lgb)
	ifstream is(session_file.c_str());
	string tmp;

	while (getline(is, tmp)) {
		// Ignore comments, empty line or line stats with ' '
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		// Determine section id
		if (tmp == sec_lastfiles)
			lastFiles().read(is);
		else if (tmp == sec_lastopened)
			lastOpened().read(is);
		else if (tmp == sec_lastfilepos)
			lastFilePos().read(is);
		else if (tmp == sec_bookmarks)
			bookmarks().read(is);
		else if (tmp == sec_toolbars)
			toolbars().read(is);
		else if (tmp == sec_session)
			sessionInfo().read(is);
		else
			lyxerr << "LyX: Warning: unknown Session section: " << tmp << endl;
	}
}


void Session::writeFile() const
{
	ofstream os(session_file.c_str());
	if (os) {
		os << "## Automatically generated lyx session file \n"
		    << "## Editing this file manually may cause lyx to crash.\n";

		lastFiles().write(os);
		lastOpened().write(os);
		lastFilePos().write(os);
		bookmarks().write(os);
		toolbars().write(os);
		sessionInfo().write(os);
	} else
		lyxerr << "LyX: Warning: unable to save Session: "
		       << session_file << endl;
}

}
