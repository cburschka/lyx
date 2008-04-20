/**
 * \file Session.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Session.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/Package.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace lyx::support;

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
		FileName const file(tmp);
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ' || !file.isAbsolute())
			continue;

		// read lastfiles
		if (file.exists() && !file.isDirectory()
		    && lastfiles.size() < num_lastfiles)
			lastfiles.push_back(file);
		else
			LYXERR(Debug::INIT, "LyX: Warning: Ignore last file: " << tmp);
	} while (is.good());
}


void LastFilesSection::write(ostream & os) const
{
	os << '\n' << sec_lastfiles << '\n';
	copy(lastfiles.begin(), lastfiles.end(),
	     ostream_iterator<FileName>(os, "\n"));
}


void LastFilesSection::add(FileName const & file)
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
		LYXERR(Debug::INIT, "LyX: session: too many last files\n"
			<< "\tdefault (=" << default_num_last_files << ") used.");
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
		FileName const file(tmp);
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ' || !file.isAbsolute())
			continue;

		if (file.exists() && !file.isDirectory())
			lastopened.push_back(file);
		else
			LYXERR(Debug::INIT, "LyX: Warning: Ignore last opened file: " << tmp);
	} while (is.good());
}


void LastOpenedSection::write(ostream & os) const
{
	os << '\n' << sec_lastopened << '\n';
	copy(lastopened.begin(), lastopened.end(),
	     ostream_iterator<FileName>(os, "\n"));
}


void LastOpenedSection::add(FileName const & file)
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
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		try {
			// read lastfilepos
			// pos, file\n
			FilePos filepos;
			string fname;
			istringstream itmp(tmp);
			itmp >> filepos.pit;
			itmp.ignore(2);  // ignore ", "
			itmp >> filepos.pos;
			itmp.ignore(2);  // ignore ", "
			getline(itmp, fname);
			FileName const file(fname);
			if (!file.isAbsolute())
				continue;
			if (file.exists() && !file.isDirectory()
			    && lastfilepos.size() < num_lastfilepos)
				lastfilepos[file] = filepos;
			else
				LYXERR(Debug::INIT, "LyX: Warning: Ignore pos of last file: " << fname);
		} catch (...) {
			LYXERR(Debug::INIT, "LyX: Warning: unknown pos of last file: " << tmp);
		}
	} while (is.good());
}


void LastFilePosSection::write(ostream & os) const
{
	os << '\n' << sec_lastfilepos << '\n';
	for (FilePosMap::const_iterator file = lastfilepos.begin();
		file != lastfilepos.end(); ++file) {
		os << file->second.pit << ", " << file->second.pos << ", "
		   << file->first << '\n';
	}
}


void LastFilePosSection::save(FileName const & fname, FilePos const & pos)
{
	lastfilepos[fname] = pos;
}


LastFilePosSection::FilePos LastFilePosSection::load(FileName const & fname) const
{
	FilePosMap::const_iterator entry = lastfilepos.find(fname);
	// Has position information, return it.
	if (entry != lastfilepos.end())
		return entry->second;
	// Not found, return the first paragraph
	return FilePos();
}


void BookmarksSection::clear()
{
	// keep bookmark[0], the temporary one
	bookmarks.resize(1);
	bookmarks.resize(max_bookmarks + 1);
}


void BookmarksSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		try {
			// read bookmarks
			// idx, pit, pos, file\n
			unsigned int idx;
			pit_type pit;
			pos_type pos;
			string fname;
			istringstream itmp(tmp);
			itmp >> idx;
			itmp.ignore(2);  // ignore ", "
			itmp >> pit;
			itmp.ignore(2);  // ignore ", "
			itmp >> pos;
			itmp.ignore(2);  // ignore ", "
			getline(itmp, fname);
			FileName const file(fname);
			if (!file.isAbsolute())
				continue;
			// only load valid bookmarks
			if (file.exists() && !file.isDirectory() && idx <= max_bookmarks)
				bookmarks[idx] = Bookmark(file, pit, pos, 0, 0);
			else
				LYXERR(Debug::INIT, "LyX: Warning: Ignore bookmark of file: " << fname);
		} catch (...) {
			LYXERR(Debug::INIT, "LyX: Warning: unknown Bookmark info: " << tmp);
		}
	} while (is.good());
}


void BookmarksSection::write(ostream & os) const
{
	os << '\n' << sec_bookmarks << '\n';
	for (size_t i = 1; i <= max_bookmarks; ++i) {
		if (isValid(i))
			os << i << ", "
			   << bookmarks[i].bottom_pit << ", "
			   << bookmarks[i].bottom_pos << ", "
			   << bookmarks[i].filename << '\n';
	}
}


void BookmarksSection::save(FileName const & fname,
	pit_type bottom_pit, pos_type bottom_pos,
	int top_id, pos_type top_pos, unsigned int idx)
{
	// silently ignore bookmarks when idx is out of range
	if (idx <= max_bookmarks)
		bookmarks[idx] = Bookmark(fname, bottom_pit, bottom_pos, top_id, top_pos);
}


bool BookmarksSection::isValid(unsigned int i) const
{
	return i <= max_bookmarks && !bookmarks[i].filename.empty();
}


BookmarksSection::Bookmark const & BookmarksSection::bookmark(unsigned int i) const
{
	return bookmarks[i];
}


void ToolbarSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		try {
			// Read session info, saved as key/value pairs
			// would better yell if pos returns npos
			size_t pos = tmp.find_first_of(" = ");
			// silently ignore lines without " = "
			if (pos != string::npos) {
				ToolbarItem item;
				item.key = tmp.substr(0, pos);
				int state;
				int location;
				istringstream value(tmp.substr(pos + 3));
				value >> state;
				value >> location;
				value >> item.info.posx;
				value >> item.info.posy;
				item.info.state = ToolbarInfo::State(state);
				item.info.location = ToolbarInfo::Location(location);
				toolbars.push_back(item);
			} else
				LYXERR(Debug::INIT, "LyX: Warning: Ignore toolbar info: " << tmp);
		} catch (...) {
			LYXERR(Debug::INIT, "LyX: Warning: unknown Toolbar info: " << tmp);
		}
	} while (is.good());
	// sort the toolbars by location, line and position
	sort(toolbars.begin(), toolbars.end());
}


void ToolbarSection::write(ostream & os) const
{
	os << '\n' << sec_toolbars << '\n';
	for (ToolbarList::const_iterator tb = toolbars.begin();
		tb != toolbars.end(); ++tb) {
		os << tb->key << " = "
		  << static_cast<int>(tb->info.state) << " "
		  << static_cast<int>(tb->info.location) << " "
		  << tb->info.posx << " "
		  << tb->info.posy << '\n';
	}
}


ToolbarSection::ToolbarInfo & ToolbarSection::load(string const & name)
{
	for (ToolbarList::iterator tb = toolbars.begin();
		tb != toolbars.end(); ++tb)
		if (tb->key == name)
			return tb->info;

	// add a new item
	ToolbarItem item;
	item.key = name;
	toolbars.push_back(item);
	return toolbars.back().info;
}


bool operator<(ToolbarSection::ToolbarItem const & a, ToolbarSection::ToolbarItem const & b)
{
	ToolbarSection::ToolbarInfo lhs = a.info;
	ToolbarSection::ToolbarInfo rhs = b.info;
	// on if before off
	if (lhs.state != rhs.state)
		return static_cast<int>(lhs.state) < static_cast<int>(rhs.state);
	// order of dock does not really matter
	if (lhs.location != rhs.location)
		return static_cast<int>(lhs.location) < static_cast<int>(rhs.location);
	// if the same dock, the order depends on position
	if (lhs.location == ToolbarSection::ToolbarInfo::TOP ||
		lhs.location == ToolbarSection::ToolbarInfo::BOTTOM)
		return lhs.posy < rhs.posy || (lhs.posy == rhs.posy && lhs.posx < rhs.posx);
	else if (lhs.location == ToolbarSection::ToolbarInfo::LEFT ||
		lhs.location == ToolbarSection::ToolbarInfo::RIGHT)
		return lhs.posx < rhs.posx || (lhs.posx == rhs.posx && lhs.posy < rhs.posy);
	return true;
}


void SessionInfoSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		try {
			// Read session info, saved as key/value pairs
			// would better yell if pos returns npos
			string::size_type pos = tmp.find_first_of(" = ");
			// silently ignore lines without " = "
			if (pos != string::npos) {
				string key = tmp.substr(0, pos);
				string value = tmp.substr(pos + 3);
				sessioninfo[key] = value;
			} else
				LYXERR(Debug::INIT, "LyX: Warning: Ignore session info: " << tmp);
		} catch (...) {
			LYXERR(Debug::INIT, "LyX: Warning: unknown Session info: " << tmp);
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
	session_file = FileName(addName(package().user_support().absFilename(), "session"));
	//
	readFile();
}


void Session::readFile()
{
	// we will not complain if we can't find session_file nor will
	// we issue a warning. (Lgb)
	ifstream is(session_file.toFilesystemEncoding().c_str());
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
			LYXERR(Debug::INIT, "LyX: Warning: unknown Session section: " << tmp);
	}
}


void Session::writeFile() const
{
	ofstream os(session_file.toFilesystemEncoding().c_str());
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
		LYXERR(Debug::INIT, "LyX: Warning: unable to save Session: "
		       << session_file);
}

}
