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


#include "LString.h"

#include <vector>

class Buffer;

class Format {
public:
	///
	Format(string const & n, string const & e, string const & p,
	       string const & s, string const & v);
	///
	bool dummy() const;
	///
	bool isChildFormat() const;
	///
	string const parentFormat() const;
	///
	string const & name() const {
		return name_;
	}
	///
	string const & extension() const {
		return extension_;
	}
	///
	string const & prettyname() const {
		return prettyname_;
	}
	///
	string const & shortcut() const {
		return shortcut_;
	}
	///
	string const & viewer() const {
		return viewer_;
	}
	///
	void setViewer(string const & v) {
		viewer_ = v;
	}
private:
	string name_;
	///
	string extension_;
	///
	string prettyname_;
	///
	string shortcut_;
	///
	string viewer_;
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
	///
	Format const * getFormat(string const & name) const;
	///
	int getNumber(string const & name) const;
	///
	void add(string const & name);
	///
	void add(string const & name, string const & extension,
		 string const & prettyname, string const & shortcut);
	///
	void erase(string const & name);
	///
	void sort();
	///
	void setViewer(string const & name, string const & command);
	///
	bool view(Buffer const & buffer, string const & filename,
		  string const & format_name) const;
	///
	string const prettyName(string const & name) const;
	///
	string const extension(string const & name) const;
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
