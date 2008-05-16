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
	cout << FileName("/foo/../bar").absFilename() << endl;
	cout << FileName("/foo/./bar").absFilename() << endl;
	cout << FileName("/foo//bar").absFilename() << endl;
}

int main()
{
	test_normalizePath();
}
