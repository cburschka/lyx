#include "../filetools.h"

#include <iostream>

using namespace lyx::support;

using namespace std;

string _(string const & str)
{
	return str;
}

void test_NormalizePath()
{
	cout << NormalizePath("foo/../bar") << endl;
	cout << NormalizePath("foo/./bar") << endl;
	cout << NormalizePath("./foo/../bar") << endl;
	cout << NormalizePath("./foo/./bar") << endl;
	cout << NormalizePath("/foo/../bar") << endl;
	cout << NormalizePath("/foo/./bar") << endl;
	cout << NormalizePath("foo//bar") << endl;	
	cout << NormalizePath("./foo//bar") << endl;	
	cout << NormalizePath("/foo//bar") << endl;	
}

int main()
{
	test_NormalizePath();
}
