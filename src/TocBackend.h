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
 *
 * TocBackend mainly used in toc.[Ch]
 */

#ifndef TOC_BACKEND_H
#define TOC_BACKEND_H

#include <map>
#include <iosfwd>
#include <vector>
#include <string>

#include "pariterator.h"

class Buffer;
class LyXView;
class Paragraph;
class FuncRequest;
class LCursor;

namespace lyx {

///
/**
*/
class TocBackend
{
public:

	///
	/**
	*/
	class Item
	{
		friend class TocBackend;
		friend bool operator==(Item const & a, Item const & b);

	public:
		///
		Item(
			ParConstIterator const & par_it = ParConstIterator(),
			int d = -1,
			std::string const & s = std::string());
		///
		~Item() {}
		///
		bool const isValid() const;
		///
		int const id() const;
		///
		int const depth() const;
		///
		std::string const & str() const;
		///
		std::string const asString() const;
		/// set cursor in LyXView to this Item
		void goTo(LyXView & lv_) const;
		/// the action corresponding to the goTo above
		FuncRequest action() const;
		
	protected:
		/// Current position of item.
		ParConstIterator par_it_;

		/// nesting depth
		int depth_;

		/// Full item string
		std::string str_;
	};

	///
	typedef std::vector<Item> Toc;
	typedef std::vector<Item>::const_iterator TocIterator;
	///
	typedef std::map<std::string, Toc> TocList;

public:
	///
	TocBackend(Buffer const * buffer = NULL): buffer_(buffer) {}
	///
	~TocBackend() {}
	///
	void setBuffer(Buffer const * buffer)
	{ buffer_ = buffer; }
	///
	bool addType(std::string const & type);
	///
	void update();
	///
	TocList const & tocs()
	{ return tocs_; }
	///
	std::vector<std::string> const & types()
	{ return types_; }
	///
	Toc const & toc(std::string const & type);
	/// Return the first Toc Item before the cursor
	TocIterator const item(std::string const & type, ParConstIterator const &);

	void asciiTocList(std::string const & type, std::ostream & os) const;

private:
	/// 
	TocList tocs_;
	///
	std::vector<std::string> types_;
	///
	Item const invalid_item_;
	///
	Toc const empty_toc_;
	///
	Buffer const * buffer_;

}; // TocBackend

inline
bool operator==(TocBackend::Item const & a, TocBackend::Item const & b)
{
	return a.id() == b.id() && a.str() == b.str();
	// No need to compare depth.
}


inline
bool operator!=(TocBackend::Item const & a, TocBackend::Item const & b)
{
	return !(a == b);
}


} // namespace lyx

#endif // TOC_BACKEND_H
