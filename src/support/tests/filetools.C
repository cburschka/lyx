#include "../filetools.h"

#include <iostream>

using namespace lyx::support;

using namespace std;

string _(string const & str)
{
	return str;
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
