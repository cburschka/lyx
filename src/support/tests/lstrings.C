#include <config.h>

#include "../lstrings.h"

#include <iostream>


using namespace lyx::support;
using namespace lyx;

using namespace std;

namespace lyx {
	docstring const _(string const & s) { return from_ascii(s); }
}

void test_lowercase()
{
	cout << to_ascii(docstring(1, lowercase(char_type('A')))) << endl;
	cout << to_ascii(lowercase(from_ascii("AlLe"))) << endl;
	cout << lowercase('A') << endl;
	cout << ascii_lowercase("AlLe") << endl;
}

void test_uppercase()
{
	cout << to_ascii(docstring(1, uppercase(char_type('a')))) << endl;
	cout << to_ascii(uppercase(from_ascii("AlLe"))) << endl;
	cout << uppercase('a') << endl;
}

int main()
{
	test_lowercase();
	test_uppercase();
}
