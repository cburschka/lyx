// -*- C++ -*-
/**
 * \file encoding.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ENCODING_H
#define ENCODING_H

#include <map>
#include <string>

#include "support/types.h"

namespace lyx {

namespace support { class FileName; }

///
class Encoding {
public:
	///
	Encoding() {}
	///
	Encoding(std::string const & n, std::string const & l,
	         std::string const & i)
		: Name_(n), LatexName_(l), iconvName_(i)
	{
	}
	///
	std::string const & name() const { return Name_; }
	///
	std::string const & latexName() const { return LatexName_; }
	///
	std::string const & iconvName() const { return iconvName_; }
private:
	///
	std::string Name_;
	///
	std::string LatexName_;
	///
	std::string iconvName_;
};

class Encodings {
public:
	///
	typedef std::map<std::string, Encoding> EncodingList;
	/// iterator to iterate over all encodings.
	/// We hide the fact that our encoding list is implemented as a map.
	class const_iterator : public EncodingList::const_iterator {
		typedef EncodingList::const_iterator base;
	public:
		const_iterator() : base() {}
		const_iterator(base const & b) : base(b) {}
		Encoding const & operator*() const { return base::operator*().second; }
		Encoding const * operator->() const { return &(base::operator*().second); }
	};
	///
	Encodings();
	///
	void read(support::FileName const & filename);
	/// Get encoding from LyX name \p name
	Encoding const * getFromLyXName(std::string const & name) const;
	/// Get encoding from LaTeX name \p name
	Encoding const * getFromLaTeXName(std::string const & name) const;

	///
	const_iterator begin() const { return encodinglist.begin(); }
	///
	const_iterator end() const { return encodinglist.end(); }

	///
	enum Letter_Form {
		///
		FORM_ISOLATED,
		///
		FORM_FINAL,
		///
		FORM_INITIAL,
		///
		FORM_MEDIAL
	};
	///
	static bool isComposeChar_hebrew(char_type c);
	///
	static bool isComposeChar_arabic(char_type c);
	///
	static bool is_arabic_special(char_type c);
	///
	static bool is_arabic(char_type c);
	///
	static char_type transformChar(char_type c, Letter_Form form);

private:
	///
	EncodingList encodinglist;
};

extern Encodings encodings;


} // namespace lyx

#endif
