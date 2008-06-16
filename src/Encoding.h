// -*- C++ -*-
/**
 * \file Encoding.h
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
#include "support/types.h"

#include <map>
#include <set>
#include <vector>

namespace lyx {

namespace support { class FileName; }

class LaTeXFeatures;

class EncodingException : public std::exception {
public:
	EncodingException(char_type c);
	virtual ~EncodingException() throw() {}
	virtual const char * what() const throw();
 
	char_type failed_char;
	int par_id;
	pos_type pos;
};


///
class Encoding {
public:
	/// Which LaTeX package handles this encoding?
	enum Package {
		none,
		inputenc,
		CJK
	};
	///
	Encoding() {}
	///
	Encoding(std::string const & n, std::string const & l,
		 std::string const & i, bool f, Package p);
	///
	void init() const;
	///
	std::string const & name() const { return name_; }
	///
	std::string const & latexName() const { return latexName_; }
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
	docstring latexChar(char_type c) const;
	/// Which LaTeX package handles this encoding?
	Package package() const { return package_; }
	/// A list of all characters usable in this encoding
	std::vector<char_type> symbolsList() const;
private:
	///
	std::string name_;
	///
	std::string latexName_;
	///
	std::string iconvName_;
	/// Is this a fixed width encoding?
	bool fixedwidth_;
	///
	typedef std::set<char_type> CharSet;
	/// Set of UCS4 characters that we can encode (for singlebyte
	/// encodings only)
	mutable CharSet encodable_;
	/// All code points below this are encodable. This helps us to avoid
	/// lokup of ASCII characters in encodable_ and gives about 1 sec
	/// speedup on export of the Userguide.
	mutable char_type start_encodable_;
	/// Which LaTeX package handles this encoding?
	Package package_;
	/**
	 * If this is true the stored information about the encoding covers
	 * all encodable characters. We set this to false initially so that
	 * we only need to query iconv for the actually used encodings.
	 * This is needed especially for the multibyte encodings, if we
	 * complete all encoding info on startup it takes 2-3 minutes.
	 */
	mutable bool complete_;
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
	Encoding const * fromLyXName(std::string const & name) const;
	/// Get encoding from LaTeX name \p name
	Encoding const * fromLaTeXName(std::string const & name) const;

	///
	const_iterator begin() const { return encodinglist.begin(); }
	///
	const_iterator end() const { return encodinglist.end(); }

	///
	enum LetterForm {
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
	static bool isHebrewComposeChar(char_type c);
	///
	static bool isArabicComposeChar(char_type c);
	///
	static bool isArabicSpecialChar(char_type c);
	///
	static bool isArabicChar(char_type c);
	///
	static char_type transformChar(char_type c, LetterForm form);
	/// Is this a combining char?
	static bool isCombiningChar(char_type c);
	/**
	 * Is this a known char from some language?
	 * If \p preamble is empty and code point \p c is known to belong
	 * to a supported script, true is returned and \p preamble is set
	 * to the corresponding entry in the unicodesymbols file.
	 * If \p preamble is not empty, a check is made whether code point
	 * \p c is a known character matching the preamble entry.
	 */
	static bool isKnownScriptChar(char_type const c, std::string & preamble);
	/**
	 * Convert \p c to something that LaTeX can understand in math mode.
	 * \return whether \p command is a math mode command
	 */
	static bool latexMathChar(char_type c, docstring & command);

	/**
	 * Convert the LaTeX command in \p cmd to the corresponding unicode
	 * point and set \p combining to true if it is a combining symbol
	 */
	static char_type fromLaTeXCommand(docstring const & cmd, bool & combining);
	/**
	 * Convert the LaTeX commands in \p cmd and \return a docstring
	 * of corresponding unicode points. The conversion stops at the
	 * first command which could not be converted, and the remaining
	 * unconverted commands are returned in \p rem
	 */
	static docstring fromLaTeXCommand(docstring const & cmd, docstring & rem);
	/**
	 * Add the preamble snippet needed for the output of \p c to
	 * \p features.
	 * This does not depend on the used encoding, since the inputenc
	 * package only maps the code point \p c to a command, it does not
	 * make this command available.
	 */
	static void validate(char_type c, LaTeXFeatures & features, bool for_mathed = false);

private:
	///
	EncodingList encodinglist;
};

extern Encodings encodings;


} // namespace lyx

#endif
