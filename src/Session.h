// -*- C++ -*-
/**
 * \file Session.h
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

#include "support/FileName.h"
#include "support/types.h"

#include <string>
#include <deque>
#include <vector>
#include <map>

/** This session file maintains
  1. the latest documents loaded (lastfiles)
  2. cursor positions of files closed (lastfilepos)
  3. opened files when a lyx session is closed (lastopened)
  4. bookmarks
  5. general purpose session info in the form of key/value pairs
 */
namespace lyx {

/* base class for all sections in the session file
*/
class SessionSection
{
public:
	///
	SessionSection() {}
	///
	virtual ~SessionSection() {}

	/// read section from std::istream
	virtual void read(std::istream & is) = 0;

	/// write to std::ostream
	virtual void write(std::ostream & os) const = 0;

private:
	/// uncopiable
	SessionSection(SessionSection const &);
	void operator=(SessionSection const &);
};


class LastFilesSection : SessionSection
{
public:
	///
	typedef std::deque<support::FileName> LastFiles;

public:
	///
	explicit LastFilesSection(unsigned int num = 4);

	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/// Return lastfiles container (deque)
	LastFiles const lastFiles() const { return lastfiles; }

	/** Insert #file# into the lastfile dequeue.
	    This funtion inserts #file# into the last files list. If the file
	    already exists it is moved to the top of the list, else exist it
	    is placed on the top of the list. If the list is full the last
	    file in the list is popped from the end.
	    @param file the file to insert in the lastfile list.
	*/
	void add(support::FileName const & file);

private:
	/// Default number of lastfiles.
	unsigned int const default_num_last_files;

	/// Max number of lastfiles.
	unsigned int const absolute_max_last_files;

	/// a list of lastfiles
	LastFiles lastfiles;

	/// number of files in the lastfiles list.
	unsigned int num_lastfiles;

	/** Used by the constructor to set the number of stored last files.
	    @param num the number of lastfiles to set.
	*/
	void setNumberOfLastFiles(unsigned int num);
};


class LastOpenedSection : SessionSection
{
public:
	///
	typedef std::vector<support::FileName> LastOpened;

public:
	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/// Return lastopened container (vector)
	LastOpened const getfiles() const { return lastopened; }

	/** add file to lastopened file list
	    @param file filename to add
	*/
	void add(support::FileName const & file);

	/** clear lastopened file list
	 */
	void clear();

private:
	/// a list of lastopened files
	LastOpened lastopened;
};


class LastFilePosSection : SessionSection
{
public:
	///
	struct FilePos {
		FilePos() : pit(0), pos(0) {}
		pit_type pit;
		pos_type pos;
	};

	///
	typedef std::map<support::FileName, FilePos> FilePosMap;

public:
	///
	LastFilePosSection() : num_lastfilepos(100) {}

	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/** add cursor position to the fname entry in the filepos map
	    @param fname file entry for which to save position information
	    @param pos position of the cursor when the file is closed.
	*/
	void save(support::FileName const & fname, FilePos const & pos);

	/** load saved cursor position from the fname entry in the filepos map
	    @param fname file entry for which to load position information
	*/
	FilePos load(support::FileName const & fname) const;

private:
	/// default number of lastfilepos to save */
	unsigned int const num_lastfilepos;


	/// a map of file positions
	FilePosMap lastfilepos;
};


class BookmarksSection : SessionSection
{
public:
	/// A bookmark is composed of three parts
	/// 1. filename
	/// 2. bottom (whole document) level pit and pos, used to (inaccurately) save/restore a bookmark
	/// 3. top level id and pos, used to accurately locate bookmark when lyx is running
	/// top and bottom level information sometimes needs to be sync'ed. In particular,
	/// top_id is determined when a bookmark is restored from session; and
	/// bottom_pit and bottom_pos are determined from top_id when a bookmark
	/// is save to session. (What a mess! :-)
	///
	/// TODO: bottom level pit and pos will be replaced by StableDocIterator
	class Bookmark {
	public:
		/// Filename
		support::FileName filename;
		/// Bottom level cursor pit, will be saved/restored by .lyx/session
		pit_type bottom_pit;
		/// Bottom level cursor position, will be saved/restore by .lyx/session
		pos_type bottom_pos;
		/// Top level cursor id, used to lcoate bookmarks for opened files
		int top_id;
		/// Top level cursor position within a paragraph
		pos_type top_pos;
		///
		Bookmark() : bottom_pit(0), bottom_pos(0), top_id(0), top_pos(0) {}
		///
		Bookmark(support::FileName const & f, pit_type pit, pos_type pos, int id, pos_type tpos)
			: filename(f), bottom_pit(pit), bottom_pos(pos), top_id(id), top_pos(tpos) {}
		/// set bookmark top_id, this is because newly loaded bookmark
		/// may have zero par_id and par_pit can change during editing, see bug 3092
		void updatePos(pit_type pit, pos_type pos, int id) {
			bottom_pit = pit;
			bottom_pos = pos;
			top_id = id;
		}
	};

	///
	typedef std::vector<Bookmark> BookmarkList;

public:
	/// constructor, set max_bookmarks
	/// allow 9 regular bookmarks, bookmark 0 is temporary
	BookmarksSection() : bookmarks(10), max_bookmarks(9) {}

	/// Save the current position as bookmark
	void save(support::FileName const & fname, pit_type bottom_pit, pos_type bottom_pos,
		int top_id, pos_type top_pos, unsigned int idx);

	/// return bookmark 0-9, bookmark 0 is the temporary bookmark
	Bookmark const & bookmark(unsigned int i) const;

	/// does the given bookmark have a saved position ?
	bool isValid(unsigned int i) const;

	///
	unsigned int size() const { return max_bookmarks; }

	/// clear all bookmarks
	void clear();

	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/** return bookmark list. Non-const container is used since
		bookmarks will be cleaned after use.
	*/
	BookmarkList & load() { return bookmarks; }

private:

	/// a list of bookmarks
	BookmarkList bookmarks;

	///
	unsigned int const max_bookmarks;
};


class Session
{
public:
	/// Read the session file.  @param num length of lastfiles
	explicit Session(unsigned int num = 4);
	/// Write the session file.
	void writeFile() const;
	///
	LastFilesSection & lastFiles() { return last_files; }
	///
	LastFilesSection const & lastFiles() const { return last_files; }
	///
	LastOpenedSection & lastOpened() { return last_opened; }
	///
	LastOpenedSection const & lastOpened() const { return last_opened; }
	///
	LastFilePosSection & lastFilePos() { return last_file_pos; }
	///
	LastFilePosSection const & lastFilePos() const { return last_file_pos; }
	///
	BookmarksSection & bookmarks() { return bookmarks_; }
	///
	BookmarksSection const & bookmarks() const { return bookmarks_; }

private:
	friend class LyX;
	/// uncopiable
	Session(Session const &);
	void operator=(Session const &);

	/// file to save session, determined in the constructor.
	support::FileName session_file;

	/** Read the session file.
	    Reads the #.lyx/session# at the beginning of the LyX session.
	    This will read the session file (usually #.lyx/session#).
	    @param file the file containing the session.
	*/
	void readFile();

	///
	LastFilesSection last_files;
	///
	LastOpenedSection last_opened;
	///
	LastFilePosSection last_file_pos;
	///
	BookmarksSection bookmarks_;
};

/// This is a singleton class. Get the instance.
/// Implemented in LyX.cpp.
Session & theSession();

} // lyx

#endif
