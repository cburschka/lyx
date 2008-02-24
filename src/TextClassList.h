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

#ifndef TEXTCLASSLIST_H
#define TEXTCLASSLIST_H

#include "TextClass.h"

#include "support/strfwd.h"

#include <vector>


namespace lyx {

class Layout;

/// Reads the style files
extern bool LyXSetStyle();

///
class TextClassList {
public:
	///
	TextClassList() {}
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
	TextClassList(TextClassList const &);
	/// nonassignable
	void operator=(TextClassList const &);

	///
	mutable ClassList classlist_;
};

///
extern TextClassList textclasslist;
///
BaseClassIndex defaultTextclass();


} // namespace lyx

#endif
