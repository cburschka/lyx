#include "../lstrings.h"

#include <iostream>


using namespace lyx::support;

using namespace std;

namespace lyx {
	docstring const _(string const & s) { return from_ascii(s); }
}

void test_lowercase()
{
	cout << lowercase('A') << endl;
	cout << lowercase("AlLe") << endl;
}

void test_uppercase()
{
	cout << uppercase('a') << endl;
	cout << uppercase("AlLe") << endl;
}

int main()
{
	test_lowercase();
	test_uppercase();
}
