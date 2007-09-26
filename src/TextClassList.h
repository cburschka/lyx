// -*- C++ -*-
/**
 * \file TextClassList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXTEXTCLASSLIST_H
#define LYXTEXTCLASSLIST_H

#include "TextClass.h"

#include "support/types.h"

#include <boost/utility.hpp>

#include <string>
#include <vector>


namespace lyx {

class Layout;

/// Reads the style files
extern bool LyXSetStyle();

///
class TextClassList : boost::noncopyable {
public:
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
	std::pair<bool, textclass_type> const
	numberOfClass(std::string const & textclass) const;

	///
	TextClass const & operator[](textclass_type textclass) const;

	/// Read textclass list.  Returns false if this fails.
	bool read();
	
	/// Clears the textclass so as to force it to be reloaded
	void reset(textclass_type const textclass);

	/// add a textclass from user local directory.
	/// Return ture/false, and textclass number
	std::pair<bool, textclass_type> const
	addTextClass(std::string const & textclass, std::string const & path);

private:
	///
	mutable ClassList classlist_;
};

///
extern TextClassList textclasslist;


} // namespace lyx

#endif
