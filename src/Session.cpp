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
#include "support/FileNameList.h"
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


void LastOpenedSection::read(istream & /*is*/)
{
	lastopened.clear();
	FileNameList list;// = theApp()->fileNameListFromSession("last_opened");
	for (size_t i = 0; i != list.size(); ++i) {
		FileName const & file = list[i];
		if (!file.isAbsolute() || !file.exists() || file.isDirectory())
			LYXERR(Debug::INIT, "Warning: invalid last opened file: " << file);
		else
			lastopened.push_back(file);
	}
}


void LastOpenedSection::write(ostream & /*os*/) const
{
	//theApp()->toSession(lastopened);
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
	} else
		LYXERR(Debug::INIT, "LyX: Warning: unable to save Session: "
		       << session_file);
}

}
