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

/** base class for all sections in the session file
*/
class SessionSection : boost::noncopyable {

public:
	///
	virtual ~SessionSection() {}

	/// read section from std::istream
	virtual void read(std::istream & is) = 0;

	/// write to std::ostream
	virtual void write(std::ostream & os) const = 0;
};


class LastFilesSection : SessionSection
{
public:
	///
	typedef std::deque<std::string> LastFiles;

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
	void add(std::string const & file);

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
	typedef std::vector<std::string> LastOpened;

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
	void add(std::string const & file);

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
	typedef boost::tuple<pit_type, pos_type> FilePos;

	///
	typedef std::map<std::string, FilePos> FilePosMap;

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
	void save(std::string const & fname, FilePos pos);

	/** load saved cursor position from the fname entry in the filepos map
	    @param fname file entry for which to load position information
	*/
	FilePos load(std::string const & fname) const;

private:
	/// default number of lastfilepos to save */
	unsigned int const num_lastfilepos;


	/// a map of file positions
	FilePosMap lastfilepos;
};


class BookmarksSection : SessionSection
{
public:
	/// bookmarks
	class Bookmark {
	public:
		/// Filename
		std::string filename;
		/// Cursor paragraph Id
		int par_id;
		/// Cursor position
		pos_type par_pos;
		///
		Bookmark() : par_id(0), par_pos(0) {}
		///
		Bookmark(std::string const & f, int id, pos_type pos)
			: filename(f), par_id(id), par_pos(pos) {}
	};

	///
	typedef std::deque<Bookmark> BookmarkList;

public:
	/// constructor, set max_bookmarks
	/// allow 20 regular bookmarks
	BookmarksSection() : bookmarks(0), max_bookmarks(20) {}

	/// Save the current position as bookmark
	/// if save==false, save to temp_bookmark
	void save(std::string const & fname, int par_id, pos_type par_pos, bool persistent);

	/// return bookmark, return temp_bookmark if i==0
	Bookmark const & bookmark(unsigned int i) const;

	/// does the given bookmark have a saved position ?
	bool isValid(unsigned int i) const;

	///
	unsigned int size() const { return bookmarks.size(); }

	/// clear all bookmarks
	void clear() { bookmarks.clear(); }

	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/** return bookmark list. Non-const container is used since
		bookmarks will be cleaned after use.
	*/
	BookmarkList & load() { return bookmarks; }

private:
	/// temp bookmark (previously saved_positions[0]), this is really ugly
	/// c.f. ./frontends/controllers/ControlRef.C
	/// FIXME: a separate LFUN may be a better solution
	Bookmark temp_bookmark;

	/// a list of bookmarks
	BookmarkList bookmarks;

	///
	unsigned int const max_bookmarks;
};


class ToolbarSection : SessionSection
{
public:
	/// information about a toolbar, not all information can be
	/// saved/restored by all frontends, but this class provides
	/// a superset of things that can be managed by session.
	class ToolbarInfo
	{
	public:
		///
		ToolbarInfo() :
			state(ON), location(NOTSET) { }
		///
		ToolbarInfo(int s, int loc) :
			state(static_cast<State>(s)), location(static_cast<Location>(loc)) { }

	public:
		enum State {
			ON,
			OFF,
			AUTO
		};

		/// on/off/auto
		State state;

		/// location: this can be intepreted differently.
		enum Location {
			TOP,
			BOTTOM,
			LEFT,
			RIGHT,
			NOTSET
		};

		Location location;

		/// potentially, icons
	};

	/// info for each toolbar
	typedef std::map<std::string, ToolbarInfo> ToolbarMap;

public:
	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/// return reference to toolbar info, create a new one if needed
	ToolbarInfo & load(std::string const & name);

private:
	/// toolbar information
	ToolbarMap toolbars;
};


class SessionInfoSection : SessionSection
{
public:
	///
	typedef std::map<std::string, std::string> MiscInfo;

public:
	///
	void read(std::istream & is);

	///
	void write(std::ostream & os) const;

	/** set session info
		@param key key of the value to store
		@param value value, a string without newline ('\n')
	*/
	void save(std::string const & key, std::string const & value);

	/** load session info
		@param key a key to extract value from the session file
		@param release whether or not clear the value. Default to true
			since most of such values are supposed to be used only once.
	*/
	std::string const load(std::string const & key, bool release = true);

private:
	/// a map to save session info
	MiscInfo sessioninfo;
};


class Session : boost::noncopyable {

public:
	/** Read the session file.
	    @param num length of lastfiles
	*/
	explicit Session(unsigned int num = 4);

	/** Write the session file.
	*/
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

	///
	ToolbarSection & toolbars() { return toolbars_; }

	///
	ToolbarSection const & toolbars() const { return toolbars_; }

	///
	SessionInfoSection & sessionInfo() { return session_info; }

	///
	SessionInfoSection const & sessionInfo() const { return session_info; }

private:
	/// file to save session, determined in the constructor.
	std::string session_file;

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

	///
	ToolbarSection toolbars_;

	///
	SessionInfoSection session_info;
};

}

#endif
