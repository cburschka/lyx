#include <config.h>

#include "../insets/ExternalTransforms.h"
#include "../support/debug.h"

#include <iostream>


using namespace lyx;
using namespace std;


void test_sanitizeLatexOption()
{
	using external::sanitizeLatexOption;
	cout << sanitizeLatexOption("[]") << endl;
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
	lyx::lyxerr.setStream(cerr);
	test_sanitizeLatexOption();
}
