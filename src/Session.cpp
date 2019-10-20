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
string const sec_lastcommands = "[last commands]";
string const sec_authfiles = "[auth files]";
string const sec_shellescape = "[shell escape files]";

} // namespace


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
		if (tmp.empty() || tmp[0] == '#' || tmp[0] == ' ' || !FileName::isAbsolute(tmp))
			continue;

		// read lastfiles
		FileName const file(tmp);
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
	lastfiles.insert(lastfiles.begin(), file);
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
		if (tmp.empty() || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		try {
			LastOpenedFile lof;
			istringstream itmp(tmp);
			itmp >> lof.active;
			itmp.ignore(2);  // ignore ", "
			string fname;
			getline(itmp, fname);
			if (!FileName::isAbsolute(fname))
				continue;

			FileName const file(fname);
			if (file.exists() && !file.isDirectory()) {
				lof.file_name = file;
				lastopened.push_back(lof);
			} else {
				LYXERR(Debug::INIT,
					"LyX: Warning: Ignore last opened file: " << tmp);
			}
		} catch (...) {
			LYXERR(Debug::INIT,
				"LyX: Warning: unknown state of last opened file: " << tmp);
		}
	} while (is.good());
}


void LastOpenedSection::write(ostream & os) const
{
	os << '\n' << sec_lastopened << '\n';
	for (size_t i = 0; i < lastopened.size(); ++i)
		os << lastopened[i].active << ", " << lastopened[i].file_name << '\n';
}


void LastOpenedSection::add(FileName const & file, bool active)
{
	LastOpenedFile lof(file, active);
	// check if file is already recorded (this can happen
	// with multiple buffer views). We do only record each
	// file once, since we cannot restore multiple views
	// currently, we even crash in some cases (see #9483).
	// FIXME: Add session support for multiple views of
	//        the same buffer (split-view etc.).
	for (size_t i = 0; i < lastopened.size(); ++i) {
		if (lastopened[i].file_name == file)
			return;
	}
	lastopened.push_back(lof);
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
			if (!FileName::isAbsolute(fname))
				continue;
			filepos.file = FileName(fname);
			if (filepos.file.exists() && !filepos.file.isDirectory()
			    && lastfilepos.size() < num_lastfilepos)
				lastfilepos.push_back(filepos);
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
	for (auto const & file_p : lastfilepos)
		os << file_p.pit << ", " << file_p.pos << ", " << file_p.file << '\n';
}


void LastFilePosSection::save(FilePos const & pos)
{
	// Remove element if it was already present. Iterating should
	// not be a problem since the list is small (<100 elements).
	for (FilePosList::const_iterator it = lastfilepos.begin();
	     it != lastfilepos.end(); ++it)
		if (it->file == pos.file) {
			lastfilepos.erase(it);
			break;
		}

	// insert new element at front.
	lastfilepos.push_front(pos);
}


LastFilePosSection::FilePos LastFilePosSection::load(FileName const & fname) const
{
	for (auto & fp : lastfilepos)
		if (fp.file == fname)
			// Has position information, return it.
			return fp;

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
			if (!FileName::isAbsolute(fname))
				continue;
			FileName const file(fname);
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
	for (size_t i = 0; i <= max_bookmarks; ++i) {
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


bool BookmarksSection::hasValid() const
{
	for (size_t i = 1; i <= size(); ++i) {
		if (isValid(i))
			return true;
	}
	return false;
}


BookmarksSection::Bookmark const & BookmarksSection::bookmark(unsigned int i) const
{
	return bookmarks[i];
}


LastCommandsSection::LastCommandsSection(unsigned int num) :
	default_num_last_commands(30),
	absolute_max_last_commands(100)
{
	setNumberOfLastCommands(num);
}


void LastCommandsSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		if (tmp == "" || tmp[0] == '#' || tmp[0] == ' ')
			continue;

		lastcommands.push_back(tmp);
	} while (is.good());
}


void LastCommandsSection::write(ostream & os) const
{
	os << '\n' << sec_lastcommands << '\n';
	copy(lastcommands.begin(), lastcommands.end(),
		ostream_iterator<std::string>(os, "\n"));
}


void LastCommandsSection::setNumberOfLastCommands(unsigned int no)
{
	if (0 < no && no <= absolute_max_last_commands)
		num_lastcommands = no;
	else {
		LYXERR(Debug::INIT, "LyX: session: too many last commands\n"
			<< "\tdefault (=" << default_num_last_commands << ") used.");
		num_lastcommands = default_num_last_commands;
	}
}


void LastCommandsSection::add(std::string const & string)
{
	lastcommands.push_back(string);
}


void LastCommandsSection::clear()
{
	lastcommands.clear();
}


Session::Session(unsigned int num_last_files, unsigned int num_last_commands) :
	last_files(num_last_files), last_commands(num_last_commands)
{
	// locate the session file
	// note that the session file name 'session' is hard-coded
	session_file = FileName(addName(package().user_support().absFileName(), "session"));
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
		else if (tmp == sec_lastcommands)
			lastCommands().read(is);
		else if (tmp == sec_authfiles)
			authFiles().read(is);
		else if (tmp == sec_shellescape)
			shellescapeFiles().read(is);

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
		lastCommands().write(os);
		bookmarks().write(os);
		authFiles().write(os);
		shellescapeFiles().write(os);
	} else
		LYXERR(Debug::INIT, "LyX: Warning: unable to save Session: "
		       << session_file);
}


AuthFilesSection::AuthFilesSection() {  }


void AuthFilesSection::read(istream & is)
{
	string tmp;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, tmp);
		if (tmp.empty() || tmp[0] == '#' || tmp[0] == ' ' || !FileName::isAbsolute(tmp))
			continue;

		// read lastfiles
		FileName const file(tmp);
		if (file.exists() && !file.isDirectory())
			auth_files_.insert(tmp);
		else
			LYXERR(Debug::INIT, "LyX: Warning: Ignore auth file: " << tmp);
	} while (is.good());
}


void AuthFilesSection::write(ostream & os) const
{
	os << '\n' << sec_authfiles << '\n';
	copy(auth_files_.begin(), auth_files_.end(),
	     ostream_iterator<std::string>(os, "\n"));
}


bool AuthFilesSection::find(string const & name) const
{
	if (auth_files_.find(name) != auth_files_.end())
		return true;

	return false;
}


void AuthFilesSection::insert(string const & name)
{
	auth_files_.insert(name);
}


void ShellEscapeSection::read(istream & is)
{
	string s;
	do {
		char c = is.peek();
		if (c == '[')
			break;
		getline(is, s);
		if (s.empty() || s[0] == '#' || s[0] == ' ' || !FileName::isAbsolute(s))
			continue;

		// read shellescape files
		FileName const file(s.substr(0, s.length() - 2));
		if (file.exists() && !file.isDirectory())
			shellescape_files_.insert(s);
		else
			LYXERR(Debug::INIT, "LyX: Warning: Ignore shellescape file: " << file);
	} while (is.good());
}


void ShellEscapeSection::write(ostream & os) const
{
	os << '\n' << sec_shellescape << '\n';
	copy(shellescape_files_.begin(), shellescape_files_.end(),
	     ostream_iterator<std::string>(os, "\n"));
}


bool ShellEscapeSection::find(string const & name) const
{
	if (shellescape_files_.find(name + ",0") != shellescape_files_.end())
		return true;

	return findAuth(name);
}


bool ShellEscapeSection::findAuth(string const & name) const
{
	if (shellescape_files_.find(name + ",1") != shellescape_files_.end())
		return true;

	return false;
}


void ShellEscapeSection::insert(string const & name, bool auth)
{
	set<string>::iterator it;
	string const name0 = name + ",0";
	string const name1 = name + ",1";

	if (auth) {
		it = shellescape_files_.find(name0);
		if (it != shellescape_files_.end())
			shellescape_files_.erase(it);
		shellescape_files_.insert(name1);
	} else {
		it = shellescape_files_.find(name1);
		if (it != shellescape_files_.end())
			shellescape_files_.erase(it);
		shellescape_files_.insert(name0);
	}
}


void ShellEscapeSection::remove(string const & name)
{
	set<string>::iterator it = shellescape_files_.find(name + ",0");
	if (it == shellescape_files_.end())
		it = shellescape_files_.find(name + ",1");
	if (it != shellescape_files_.end())
		shellescape_files_.erase(it);
}


} // namespace lyx
