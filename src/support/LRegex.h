// -*- C++ -*-

/* C++ wrapper around the POSIX regex functions:
   regcomp, regexec, regerror, regfree.
*/

#ifndef LREGEX_H
#define LREGEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <vector>

///
class LRegex {
public:
	///
	explicit
	LRegex(string const & regex);

	///
	~LRegex();

	///
	typedef std::pair<string::size_type, string::size_type> MatchPair;

	///
	typedef std::vector<MatchPair> SubMatches;

	/// Returns all the matches in a vector
	SubMatches const & exec(string const & str) const;

	/// The whole of str matches regex.
	bool exact_match(string const & str) const;

	///
	MatchPair const first_match(string const & str) const;

	///
	string const getError() const;

	///
	int getErrorCode() const;

	/// Will the next operation fail of not.
	bool ok() const;
private:
	///
	struct Impl;

	///
	Impl * impl;
};


// We comment out these, we can comment them in when we need them.
#if 0
// some built in regular expressions

extern const LRegex LRXwhite;          // = "[ \n\t\r\v\f]+"
extern const LRegex LRXint;            // = "-?[0-9]+"
extern const LRegex LRXdouble;         // = "-?\\(\\([0-9]+\\.[0-9]*\\)\\|
                                       //    \\([0-9]+\\)\\|\\(\\.[0-9]+\\)\\)
                                       //    \\([eE][---+]?[0-9]+\\)?"
//extern const LRegex LRXalpha;          // = "[A-Za-z]+"
//extern const LRegex LRXlowercase;      // = "[a-z]+"
//extern const LRegex LRXuppercase;      // = "[A-Z]+"
//extern const LRegex LRXalphanum;       // = "[0-9A-Za-z]+"
extern const LRegex LRXidentifier;     // = "[A-Za-z_][A-Za-z0-9_]*"
#endif
#endif
