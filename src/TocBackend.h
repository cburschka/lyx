// -*- C++ -*-
/**
 * \file TocBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOC_BACKEND_H
#define TOC_BACKEND_H

#include "DocIterator.h"

#include "support/strfwd.h"

#include <map>
#include <vector>
#include <string>


namespace lyx {

class Buffer;
class FuncRequest;

///
/**
*/
class TocItem
{
	friend class Toc;
	friend class TocBackend;

public:
	/// Default constructor for STL containers.
	TocItem() : dit_(0), depth_(0), output_(false) {}
	///
	TocItem(DocIterator const & dit,
		int depth,
		docstring const & s,
		bool output_active,
		docstring const & t = docstring()
		);
	///
	~TocItem() {}
	///
	int id() const;
	///
	int depth() const;
	///
	docstring const & str() const;
	///
	docstring const & tooltip() const;
	///
	docstring const asString() const;
	///
	DocIterator const & dit() const;
	///
	bool isOutput() const { return output_; }

	/// the action corresponding to the goTo above
	FuncRequest action() const;

protected:
	/// Current position of item.
	DocIterator dit_;
	/// nesting depth
	int depth_;
	/// Full item string
	docstring str_;
	/// The tooltip string
	docstring tooltip_;
	/// Is this item in a note, inactive branch, etc?
	bool output_;
};


///
class Toc : public std::vector<TocItem>
{
public:
	typedef std::vector<TocItem>::const_iterator const_iterator;
	typedef std::vector<TocItem>::iterator iterator;
	const_iterator item(DocIterator const & dit) const;
	/// Look for a TocItem given its depth and string.
	/// \return The first matching item.
	/// \retval end() if no item was found.
	iterator item(int depth, docstring const & str);
};

typedef Toc::const_iterator TocIterator;

/// The ToC list.
/// A class and no typedef because we want to forward declare it.
class TocList : public std::map<std::string, Toc> {};


///
/**
*/
class TocBackend
{
public:
	///
	TocBackend(Buffer const * buffer) : buffer_(buffer) {}
	///
	void setBuffer(Buffer const * buffer) { buffer_ = buffer; }
	///
	void update(bool output_active);
	/// \return true if the item was updated.
	bool updateItem(DocIterator const & pit);

	///
	TocList const & tocs() const { return tocs_; }
	TocList & tocs() { return tocs_; }

	///
	Toc const & toc(std::string const & type) const;
	Toc & toc(std::string const & type);

	/// Return the first Toc Item before the cursor
	TocIterator item(
		std::string const & type, ///< Type of Toc.
		DocIterator const & dit ///< The cursor location in the document.
	) const;

	///
	void writePlaintextTocList(std::string const & type,
	        odocstringstream & os, size_t max_length) const;

private:
	///
	TocList tocs_;
	///
	Buffer const * buffer_;
}; // TocBackend

inline bool operator==(TocItem const & a, TocItem const & b)
{
	return a.id() == b.id() && a.str() == b.str() && a.depth() == b.depth();
}


inline bool operator!=(TocItem const & a, TocItem const & b)
{
	return !(a == b);
}


} // namespace lyx

#endif // TOC_BACKEND_H
