// -*- C++ -*-
/**
 * \file ModuleList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LAYOUTMODULELIST_H
#define LAYOUTMODULELIST_H

#include <list>
#include <string>

namespace lyx {

class LayoutModuleList {
public:
	///
	typedef std::list<std::string>::const_iterator const_iterator;
	///
	typedef std::list<std::string>::iterator iterator;
	///
	iterator begin() { return lml_.begin(); }
	///
	iterator end() { return lml_.end(); }
	///
	const_iterator begin() const { return lml_.begin(); }
	///
	const_iterator end() const { return lml_.end(); }
	///
	void clear() { lml_.clear(); }
	///
	bool empty() const { return lml_.empty(); }
	///
	iterator erase(iterator pos) { return lml_.erase(pos); }
	///
	iterator insert(iterator pos, std::string const & str)
		{ return lml_.insert(pos, str); }
	///
	void push_back(std::string const & str) { lml_.push_back(str); }
	/// 
	size_t size() const { return lml_.size(); }
	/// This is needed in GuiDocument. It seems better than an
	/// implicit conversion.
	std::list<std::string> const & list() const { return lml_; }
private:
	std::list<std::string> lml_;
};
}
#endif
