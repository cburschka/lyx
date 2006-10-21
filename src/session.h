// -*- C++ -*-
/**
 * \file session.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SESSION_H
#define SESSION_H

#include <support/types.h>

#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>
#include <deque>
#include <vector>
#include <map>

// used by at least frontends/qt2/QPref.C
const long maxlastfiles = 20;

/** This session file maintains
  1. the latest documents loaded (lastfiles)
  2. cursor positions of files closed (lastfilepos)
  3. opened files when a lyx session is closed (lastopened)
  4. bookmarks
  5. general purpose session info in the form of key/value pairs
 */
namespace lyx {

class Session : boost::noncopyable {

public:
	///
	typedef boost::tuple<pit_type, pos_type> FilePos;
	///
	typedef std::map<std::string, FilePos> FilePosMap;
	///
	typedef std::deque<std::string> LastFiles;
	///
	typedef std::vector<std::string> LastOpened;
	///
	typedef boost::tuple<unsigned int, std::string, unsigned int, pos_type> Bookmark;
	///
	typedef std::vector<Bookmark> BookmarkList;
	///
	typedef std::map<std::string, std::string> MiscInfo;

public:
	/** Read the session file.
	    @param num length of lastfiles
	*/
	explicit Session(unsigned int num = 4);

	/** Write the session file.
	*/
	void writeFile() const;

	/** Insert #file# into the lastfile dequeue.
	    This funtion inserts #file# into the last files list. If the file
	    already exists it is moved to the top of the list, else exist it
	    is placed on the top of the list. If the list is full the last
	    file in the list is popped from the end.
	    @param file the file to insert in the lastfile list.
	*/
	void addLastFile(std::string const & file);

	/** add cursor position to the fname entry in the filepos map
	    @param fname file entry for which to save position information
	    @param pos position of the cursor when the file is closed.
	*/
	void saveFilePosition(std::string const & fname, FilePos pos);

	/** clear lastopened file list
	 */
	void clearLastOpenedFiles();

	/** add file to lastopened file list
	    @param file filename to add
	*/
	void addLastOpenedFile(std::string const & file);

	/** load saved cursor position from the fname entry in the filepos map
	    @param fname file entry for which to load position information
	*/
	FilePos loadFilePosition(std::string const & fname) const;

	/// Return lastfiles container (deque)
	LastFiles const lastFiles() const { return lastfiles; }

	/// Return lastopened container (vector)
	LastOpened const lastOpenedFiles() const { return lastopened; }

	/** save a bookmark
		@bookmark bookmark to be saved
	*/
	void saveBookmark(Bookmark const & bookmark);

	/** return bookmark list. Non-const container is used since
		bookmarks will be cleaned after use.
	*/
	BookmarkList & loadBookmarks() { return bookmarks; }

	/** set session info
		@param key key of the value to store
		@param value value, a string without newline ('\n')
	*/
	void saveSessionInfo(std::string const & key, std::string const & value);

	/** load session info
		@param key a key to extract value from the session file
		@param release whether or not clear the value. Default to true
			since most of such values are supposed to be used only once.
	*/
	std::string const loadSessionInfo(std::string const & key, bool release = true);
private:
	/// Default number of lastfiles.
	unsigned int const default_num_last_files;

	/// Max number of lastfiles.
	unsigned int const absolute_max_last_files;

	/// default number of lastfilepos to save */
	unsigned int const num_lastfilepos;

	/// file to save session, determined in the constructor.
	std::string session_file;

	/// a list of lastfiles
	LastFiles lastfiles;

	/// a list of bookmarks
	BookmarkList bookmarks;

	/// a map to save session info
	MiscInfo sessioninfo;

	/// number of files in the lastfiles list.
	unsigned int num_lastfiles;

	/// a map of file positions
	FilePosMap lastfilepos;

	/// a list of lastopened files
	LastOpened lastopened;

	/** Read the session file.
	    Reads the #.lyx/session# at the beginning of the LyX session.
	    This will read the session file (usually #.lyx/session#).
	    @param file the file containing the session.
	*/
	void readFile();

	/** Used by the constructor to set the number of stored last files.
	    @param num the number of lastfiles to set.
	*/
	void setNumberOfLastFiles(unsigned int num);
};

}

#endif
