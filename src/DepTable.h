// -*- C++ -*-
/**
 * \file DepTable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DEP_TABLE_H
#define DEP_TABLE_H

#include <map>
#include <string>

///
class DepTable {
public:
	/** This one is a little bit harder since we need the absolute
	  filename. Should we insert files with .sty .cls etc as
	  extension? */
	void insert(std::string const & f, bool upd = false);
	///
	void update();

	///
	void write(std::string const & f) const;
	/// returns true if dep file was read successfully
	bool read(std::string const & f);
	/// returns true if any of the files has changed
	bool sumchange() const;
	/// return true if fil has changed.
	bool haschanged(std::string const & fil) const;
	/// return true if a file with extension ext has changed.
	bool extchanged(std::string const & ext) const;
	///
	bool exist(std::string const & fil) const;
	/// returns true if any files with ext exist
	bool ext_exist(std::string const & ext) const;
	///
	void remove_files_with_extension(std::string const &);
	///
	void remove_file(std::string const &);
private:
	///
	struct dep_info {
		/// Previously calculated CRC value
		unsigned long crc_prev;
		/// Current CRC value - only re-computed if mtime has changed.
		unsigned long crc_cur;
		/// mtime from last time current CRC was calculated.
		long mtime_cur;
		///
		bool changed() const;
	};
	///
	typedef std::map<std::string, dep_info> DepList;
	///
	DepList deplist;
};

#endif
