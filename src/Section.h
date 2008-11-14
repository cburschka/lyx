// -*- C++ -*-
/**
 * \file Section.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SECTION_H
#define SECTION_H

#include "Font.h"

#include <map>


namespace lyx {


///
class Section {
public:
	///
	std::string const & name() const;
	///
	int level() const;
	///
	std::string const & indent() const;
	///
	std::string const & beforeskip() const;
	///
	std::string const & afterskip() const;
	///
	Font const & style() const;
	///
	bool display() const;
private:
	///
	std::string name_;
	///
	int level_;
	///
	std::string indent_;
	///
	std::string beforeskip_;
	///
	std::string afterskip_;
	///
	Font style_;
};


///
class SectioningList {
private:
	///
	typedef std::map<std::string, Section> List_;
	///
	List_ list_;
};


} // namespace lyx

#endif
