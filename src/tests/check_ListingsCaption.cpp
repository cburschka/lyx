#include <config.h>

#include "../support/debug.h"
#include "../support/regex.h"

#include <iostream>


using namespace lyx;
using namespace std;


// This function is unfortunately copied from ../insets/InsetListing.cpp, so we
// should try to make sure to keep the two in sync.
string test_ListingsCaption(string const & cap)
{
	// convert from
	//     blah1\label{blah2} blah3
	// to
	//     blah1 blah3},label={blah2
	// to form options
	//     caption={blah1 blah3},label={blah2}
	//
	// NOTE that } is not allowed in blah2.
	regex const reg("(.*)\\\\label\\{(.*?)\\}(.*)");
	string const new_cap("$1$3},label={$2");
	return regex_replace(cap, reg, new_cap);
}

void test_test_ListingsCaptions()
{
	cout << test_ListingsCaption("\\label{}") << endl;
	cout << test_ListingsCaption("\\label{blah2}") << endl;
	cout << test_ListingsCaption("\\label{blah2} blah3") << endl;
	cout << test_ListingsCaption("\\label{} blah3") << endl;
	cout << test_ListingsCaption("blah1\\label{}") << endl;
	cout << test_ListingsCaption("blah1\\label{} blah3") << endl;
	cout << test_ListingsCaption("blah1\\label{blah2}") << endl;
	cout << test_ListingsCaption("blah1\\label{blah2} blah3") << endl;
}


int main(int, char **)
{
	// Connect lyxerr with cout instead of cerr to catch error output
	lyx::lyxerr.setStream(cout);
	test_test_ListingsCaptions();
}
