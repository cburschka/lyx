#include <config.h>

#include <sys/types.h>

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#include "lyxregex.h"
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include "LRegex.h"

using std::make_pair;

///
struct LRegex::Impl {
	///
	regex_t * preg;

	///
	int error_code;

	///
	mutable LRegex::SubMatches matches;

	///
	Impl(string const & regex)
		: preg(new regex_t), error_code(0)
	{
		error_code = regcomp(preg, regex.c_str(), REG_EXTENDED);
	}

	///
	~Impl()
	{
		regfree(preg);
		delete preg;
	}

	///
	bool exact_match(string const & str) const
	{
		regmatch_t tmp;
		if (!regexec(preg, str.c_str(), 1, &tmp, 0)) {
			if (tmp.rm_so == 0 &&
			    tmp.rm_eo == static_cast<signed int>(str.length()))
				return true;
		}
		// no match
		return false;
	}

	///
	LRegex::MatchPair const first_match(string const & str) const
	{
		regmatch_t tmp;
		regexec(preg, str.c_str(), 1, &tmp, 0);
		string::size_type const first = tmp.rm_so != -1 ?
			tmp.rm_so : string::npos;
		string::size_type const second = tmp.rm_eo != -1 ?
			tmp.rm_eo : string::npos;
		return make_pair(first, second - first);
	}

	///
	string const getError() const
	{
		size_t nr = regerror(error_code, preg, 0, 0);
		char * tmp = new char[nr];
		regerror(error_code, preg, tmp, nr);
		string const ret(tmp);
		delete [] tmp;
		return ret;
	}

	///
	LRegex::SubMatches const & exec(string const & str) const
	{
		// Some room for improvement in this func. I realize
		// that it is double as expensive as needed, but that
		// is something I am willing to pay to get the nice
		// interface. One thing that can be done is to only put
		// valid submatches into matches. That will not make this
		// func much faster, but client code will be simpler,
		// because then it will only be needed to scan through
		// all the entries in matches.
		size_t const subs =
			(preg->re_nsub != 0 ? (preg->re_nsub + 1) : 1);
		regmatch_t * mat = new regmatch_t[subs];
		string::size_type first = 0;
		string::size_type second = 0;
		matches.erase(matches.begin(), matches.end());
		if (!regexec(preg, str.c_str(), subs, mat, 0)) { // some match
			matches.reserve(subs);
			for (size_t i = 0; i < subs; ++i) {
				first = mat[i].rm_so != -1 ?
					mat[i].rm_so : string::npos;
				second = mat[i].rm_eo != -1 ?
					mat[i].rm_eo : string::npos;
				matches.push_back(make_pair(first,
							    second - first));
			}
		}
		delete[] mat;
		return matches;
	}
};


LRegex::LRegex(string const & regex)
	: impl(new Impl(regex)) {}


LRegex::~LRegex()
{
	delete impl;
}


LRegex::SubMatches const & LRegex::exec(string const & str) const
{
	return impl->exec(str);
}


bool LRegex::exact_match(string const & str) const
{
	return impl->exact_match(str);
}


LRegex::MatchPair const LRegex::first_match(string const & str) const
{
	return impl->first_match(str);
}


string const LRegex::getError() const
{
	return impl->getError();
}


int LRegex::getErrorCode() const
{
	return impl->error_code;
}


bool LRegex::ok() const {
	return impl->error_code == 0;
}


#if 0
// some built in regular expressions

// this is good
const LRegex LRXwhite("[ \n\t\r\v\f]+");
// this is good
const LRegex LRXint("-?[0-9]+");
// this is good
const LRegex LRXdouble("-?(([0-9]+.[0-9]*)|"
		     "([0-9]+)|(.[0-9]+))"
		     "([eE][---+]?[0-9]+)?");
// not usable
// const LRegex LRXalpha("[A-Za-z]+");
// not usable (only ascii)
// const LRegex LRXlowercase("[a-z]+");
// not usable (only ascii)
// const LRegex LRXuppercase("[A-Z]+");
// not usable (only ascii)
// const LRegex LRXalphanum("[0-9A-Za-z]+");
// this is good
const LRegex LRXidentifier("[A-Za-z_][A-Za-z0-9_]*");
#endif
