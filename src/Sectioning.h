// -*- C++ -*-

#ifndef SECTIONING_H
#define SECTIONING_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "LString.h"
#include "lyxfont.h"

///
class Section {
public:
	///
	string const & name() const;
	///
	int level() const;
	///
	string const & indent() const;
	///
	string const & beforeskip() const;
	///
	string const & afterskip() const;
	///
	LyXFont const & style() const;
	///
	bool display() const;
private:
	///
	string name_;
	///
	int level_;
	///
	string indent_;
	///
	string beforeskip_;
	///
	string afterskip_;
	///
	LyXFont style_;
};


///
class SectioningList {
public:
private:
	///
	typedef std::map<string, Section> List_;
	///
	List_ list_;
};

#endif
