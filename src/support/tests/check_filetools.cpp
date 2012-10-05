#include <config.h>

#include "../filetools.h"
#include "../FileName.h"

#include <iostream>


using namespace lyx::support;

using namespace std;

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
