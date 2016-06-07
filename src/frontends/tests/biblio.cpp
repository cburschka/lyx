#include <config.h>

#include <iostream>
#include <map>

#include "support/regex.h"

using namespace std;

// Escape special chars.
// All characters are literals except: '.|*?+(){}[]^$\'
// These characters are literals when preceded by a "\", which is done here
// This function is unfortunately copied from ../qt4/GuiCitation.cpp, so we
// should try to make sure to keep the two in sync.
string const escape_special_chars(string const & expr)
{
	// Search for all chars '.|*?+(){}[^$]\'
	// Note that '[', ']', and '\' must be escaped.
	lyx::regex reg("[.|*?+(){}^$\\[\\]\\\\]");

	// $& is a ECMAScript format expression that expands to all
	// of the current match
#ifdef LYX_USE_STD_REGEX
	// To prefix a matched expression with a single literal backslash, we
	// need to escape it for the C++ compiler and use:
	return lyx::regex_replace(expr, reg, "\\$&");
#else
	// A backslash in the format string starts an escape sequence in boost.
	// Thus, to prefix a matched expression with a single literal backslash,
	// we need to give two backslashes to the regex engine, and escape both
	// for the C++ compiler and use:
	return lyx::regex_replace(expr, reg, "\\\\$&");
#endif
}


typedef map<string, string> InfoMap;

// A functor for use with find_if, used to ascertain whether a
// data entry matches the required regex_
// This class is unfortunately copied from ../frontend_helpers.cpp, so we should
// try to make sure to keep the two in sync.
class RegexMatch : public unary_function<string, bool>
{
public:
	// re is used to construct an instance of lyx::regex.
	RegexMatch(InfoMap const & m, string const & re)
		: map_(m), regex_(re) {}

	bool operator()(string const & key) const {
		// the data searched is the key + its associated BibTeX/biblio
		// fields
		string data = key;
		InfoMap::const_iterator info = map_.find(key);
		if (info != map_.end())
			data += ' ' + info->second;

		// Attempts to find a match for the current RE
		// somewhere in data.
		return lyx::regex_search(data, regex_);
	}
private:
	InfoMap const map_;
	mutable lyx::regex regex_;
};


void test_escape_special_chars()
{
	cout << escape_special_chars("abcd") << endl;
	cout << escape_special_chars("ab&cd") << endl;
	cout << escape_special_chars(".|*?+(){}[]^$\"") << endl;
	cout << escape_special_chars("..||**??++(()){{}}[[]]^^$$\\\\") << endl;
}


void test_RegexMatch()
{
	InfoMap im;
	im["hello"] = "hei";

	try {
		RegexMatch rm(im, "h.*o");

		cout << rm("hello") << endl;
		cout << rm("hei") << endl;
	}
	catch (lyx::regex_error & regerr) {
		cout << regerr.what() << endl;
	}
}


int main()
{
	test_escape_special_chars();
	test_RegexMatch();
}
