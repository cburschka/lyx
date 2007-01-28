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

#include "support/docstring.h"

#include <set>

namespace lyx {

namespace support { class FileName; }

class LaTeXFeatures;


///
class Encoding {
public:
	///
	Encoding() {}
	///
	Encoding(std::string const & n, std::string const & l,
	         std::string const & i);
	///
	std::string const & name() const { return Name_; }
	///
	std::string const & latexName() const { return LatexName_; }
	///
	std::string const & iconvName() const { return iconvName_; }
	/**
	 * Convert \p c to something that LaTeX can understand.
	 * This is either the character itself (if it is representable
	 * in this encoding), or a LaTeX macro.
	 * If the character is not representable in this encoding, but no
	 * LaTeX macro is known, a warning is given of lyxerr, and the
	 * character is returned.
	 */
	docstring const latexChar(char_type c) const;
	/// Add the preamble snippet needed for the output of latexChar(c)
	/// to \p features.
	void validate(char_type c, LaTeXFeatures & features) const;
private:
	///
	std::string Name_;
	///
	std::string LatexName_;
	///
	std::string iconvName_;
	///
	typedef std::set<char_type> CharSet;
	/// Set of UCS4 characters that we can encode (for singlebyte
	/// encodings only)
	CharSet encodable_;
	/// All code points below this are encodable. This helps us to avoid
	/// lokup of ASCII characters in encodable_ and gives about 1 sec
	/// speedup on export of the Userguide.
	char_type start_encodable_;
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
	/// Read the encodings.
	/// \param encfile encodings definition file
	/// \param symbolsfile unicode->LaTeX mapping file
	void read(support::FileName const & encfile,
	          support::FileName const & symbolsfile);
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
	/// Is this a combining char?
	static bool isCombiningChar(char_type c);

private:
	///
	EncodingList encodinglist;
};

extern Encodings encodings;


} // namespace lyx

#endif
