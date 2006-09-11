// -*- C++ -*-
/**
 * \file format.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMAT_H
#define FORMAT_H

#include "support/docstring.h"

#include <vector>
#include <string>

class Buffer;

class Format {
public:
	///
	Format(std::string const & n, std::string const & e, std::string const & p,
	       std::string const & s, std::string const & v, std::string const & ed,
	       bool ex);
	///
	bool dummy() const;
	/// Tell whether this format is a child format.
	/// Child formats inherit settings like the viewer from their parent.
	bool isChildFormat() const;
	/// Name fo the parent format
	std::string const parentFormat() const;
	///
	std::string const & name() const {
		return name_;
	}
	///
	std::string const & extension() const {
		return extension_;
	}
	///
	std::string const & prettyname() const {
		return prettyname_;
	}
	///
	std::string const & shortcut() const {
		return shortcut_;
	}
	///
	std::string const & viewer() const {
		return viewer_;
	}
	///
	void setViewer(std::string const & v) {
		viewer_ = v;
	}
	///
	std::string const & editor() const {
		return editor_;
	}
	///
	void setEditor(std::string const & v) {
		editor_ = v;
	}
	///
	bool documentFormat() const {
		return document_;
	}
private:
	/// Internal name. Needs to be unique.
	std::string name_;
	/// Filename extension
	std::string extension_;
	/// Name presented to the user. Needs to be unique.
	std::string prettyname_;
	/// Keyboard shortcut for the View and Export menu.
	std::string shortcut_;
	/*!
	 * Viewer for this format. Needs to be in the PATH or an absolute
	 * filename.
	 * This format cannot be viewed if \c viewer_ is empty.
	 * If it is \c auto the default viewer of the OS for this format is
	 * used.
	 */
	std::string viewer_;
	/// Editor for this format. \sa viewer_.
	std::string editor_;
	/// Is this format a document format? (as opposed to e.g. image formats)
	/// Some formats are both (e.g. pdf), they have this flag set.
	bool document_;
};


bool operator<(Format const & a, Format const & b);

///
class Formats {
public:
	///
	typedef std::vector<Format> FormatList;
	///
	typedef FormatList::const_iterator const_iterator;
	///
	Format const & get(FormatList::size_type i) const {
		return formatlist[i];
	}
	/// \returns format named \p name if it exists, otherwise 0
	Format const * getFormat(std::string const & name) const;
	/*!
	 * Get the format of \p filename from file contents or, if this
	 * fails, from file extension.
	 * \returns file format if it could be found, otherwise an empty
	 * string.
	 */
	std::string getFormatFromFile(std::string const & filename) const;
	/// Set editor and/or viewer to "auto" for formats that can be
	/// opened by the OS.
	void setAutoOpen();
	///
	int getNumber(std::string const & name) const;
	///
	void add(std::string const & name);
	///
	void add(std::string const & name, std::string const & extension,
	         std::string const & prettyname, std::string const & shortcut,
	         std::string const & viewer, std::string const & editor,
	         bool document);
	///
	void erase(std::string const & name);
	///
	void sort();
	///
	void setViewer(std::string const & name, std::string const & command);
	///
	bool view(Buffer const & buffer, std::string const & filename,
		  std::string const & format_name) const;
	///
	bool edit(Buffer const & buffer, std::string const & filename,
		  std::string const & format_name) const;
	///
	lyx::docstring const prettyName(std::string const & name) const;
	///
	std::string const extension(std::string const & name) const;
	///
	const_iterator begin() const {
		return formatlist.begin();
	}
	///
	const_iterator end() const {
		return formatlist.end();
	}
	///
	FormatList::size_type size() const {
		return formatlist.size();
	}
private:
	///
	FormatList formatlist;
};

extern Formats formats;

extern Formats system_formats;

#endif //FORMAT_H
