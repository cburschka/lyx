#include "../filetools.h"

#include <iostream>


using namespace lyx::support;

using namespace std;

namespace lyx {
	docstring const _(string const & s) { return from_ascii(s); }
}

void test_normalizePath()
{
	cout << normalizePath("foo/../bar") << endl;
	cout << normalizePath("foo/./bar") << endl;
	cout << normalizePath("./foo/../bar") << endl;
	cout << normalizePath("./foo/./bar") << endl;
	cout << normalizePath("/foo/../bar") << endl;
	cout << normalizePath("/foo/./bar") << endl;
	cout << normalizePath("foo//bar") << endl;
	cout << normalizePath("./foo//bar") << endl;
	cout << normalizePath("/foo//bar") << endl;
}

int main()
{
	test_normalizePath();
}
