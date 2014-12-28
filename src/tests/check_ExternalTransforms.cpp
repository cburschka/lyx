#include <config.h>

#include "../insets/ExternalTransforms.h"
#include "../support/debug.h"

#include <iostream>


using namespace lyx;
using namespace std;


void test_sanitizeLatexOption()
{
	using external::sanitizeLatexOption;
	// invalid input
	cout << sanitizeLatexOption("") << endl;
	cout << sanitizeLatexOption(",") << endl;
	cout << sanitizeLatexOption(",,") << endl;
	cout << sanitizeLatexOption("[") << endl;
	cout << sanitizeLatexOption("]") << endl;
	// valid input
	cout << sanitizeLatexOption("[]") << endl;
	cout << sanitizeLatexOption("[[]") << endl;
	cout << sanitizeLatexOption("[]]") << endl;
	cout << sanitizeLatexOption("[[]]") << endl;
	cout << sanitizeLatexOption("[,]") << endl;
	cout << sanitizeLatexOption("[,,]") << endl;
	cout << sanitizeLatexOption("[,,,]") << endl;
	cout << sanitizeLatexOption("[a]") << endl;
	cout << sanitizeLatexOption("[,a]") << endl;
	cout << sanitizeLatexOption("[,,a]") << endl;
	cout << sanitizeLatexOption("[,,,a]") << endl;
	cout << sanitizeLatexOption("[a,b]") << endl;
	cout << sanitizeLatexOption("[a,,b]") << endl;
	cout << sanitizeLatexOption("[a,,,b]") << endl;
	cout << sanitizeLatexOption("[a,]") << endl;
	cout << sanitizeLatexOption("[a,,]") << endl;
	cout << sanitizeLatexOption("[a,,,]") << endl;
}


int main(int, char **)
{
	// Connect lyxerr with cout instead of cerr to catch error output
	lyx::lyxerr.setStream(cout);
	test_sanitizeLatexOption();
}
