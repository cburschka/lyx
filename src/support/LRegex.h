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

#endif
