#include <config.h>

#include "../filetools.h"
#include "../FileName.h"

#include <iostream>


using namespace lyx::support;

using namespace std;

namespace lyx {
	docstring const _(string const & s) { return from_ascii(s); }
}

void test_normalizePath()
{
	cout << FileName("/foo/../bar").absFileName() << endl;
	cout << FileName("/foo/./bar").absFileName() << endl;
	cout << FileName("/foo//bar").absFileName() << endl;
}

int main()
{
	test_normalizePath();
}
