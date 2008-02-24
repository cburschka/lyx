// -*- C++ -*-
/**
 * \file BaseClassList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASECLASSLIST_H
#define BASECLASSLIST_H

#include "TextClass.h"

#include "support/strfwd.h"

#include <vector>


namespace lyx {

class Layout;

/// Reads the style files
extern bool LyXSetStyle();

/// Index into BaseClassList. Basically a 'strong typedef'.
class BaseClassIndex {
public:
	///
	typedef size_t   base_type;
	///
	BaseClassIndex(base_type t) { data_ = t; }
	///
	operator base_type() const { return data_; }
	///
private:
	base_type data_;
};

/// A list of base document classes (*.layout files).
class BaseClassList {
public:
	///
	BaseClassList() {}
	///
	typedef std::vector<TextClass> ClassList;
	///
	typedef ClassList::const_iterator const_iterator;
	///
	const_iterator begin() const { return classlist_.begin(); }
	///
	const_iterator end() const { return classlist_.end(); }
	///
	bool empty() const { return classlist_.empty(); }

	/// Gets textclass number from name, -1 if textclass name does not exist
	std::pair<bool, BaseClassIndex> const
		numberOfClass(std::string const & textclass) const;

	///
	TextClass const & operator[](BaseClassIndex textclass) const;

	/// Read textclass list.  Returns false if this fails.
	bool read();
	
	/// Clears the textclass so as to force it to be reloaded
	void reset(BaseClassIndex const textclass);

	/// add a textclass from user local directory.
	/// Return ture/false, and textclass number
	std::pair<bool, BaseClassIndex> const
		addTextClass(std::string const & textclass, std::string const & path);

private:
	/// noncopyable
	BaseClassList(BaseClassList const &);
	/// nonassignable
	void operator=(BaseClassList const &);

	///
	mutable ClassList classlist_;
};

///
extern BaseClassList baseclasslist;
///
BaseClassIndex defaultBaseclass();


} // namespace lyx

#endif
