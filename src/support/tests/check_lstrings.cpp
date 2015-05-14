#include <config.h>

#include "../lstrings.h"

#include <iostream>


using namespace lyx::support;
using namespace lyx;

using namespace std;

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

void test_formatFPNumber()
{
	cout << formatFPNumber(0) << endl;
	cout << formatFPNumber(1) << endl;
	cout << formatFPNumber(23.42) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-20) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-19) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-18) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-17) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-16) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-15) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-14) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-13) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-12) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-11) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-10) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-9) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-8) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-7) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-6) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-5) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-4) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-3) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-2) << endl;
	cout << formatFPNumber(1.3754937356458394574047e-1) << endl;
	cout << formatFPNumber(1.3754937356458394574047) << endl;
	cout << formatFPNumber(1.3754937356458394574047e1) << endl;
	cout << formatFPNumber(1.3754937356458394574047e2) << endl;
	cout << formatFPNumber(1.3754937356458394574047e3) << endl;
	cout << formatFPNumber(1.3754937356458394574047e4) << endl;
	cout << formatFPNumber(1.3754937356458394574047e5) << endl;
	cout << formatFPNumber(1.3754937356458394574047e6) << endl;
	cout << formatFPNumber(1.3754937356458394574047e7) << endl;
	cout << formatFPNumber(1.3754937356458394574047e8) << endl;
	cout << formatFPNumber(1.3754937356458394574047e9) << endl;
	cout << formatFPNumber(1.3754937356458394574047e10) << endl;
	cout << formatFPNumber(1.3754937356458394574047e11) << endl;
	cout << formatFPNumber(1.3754937356458394574047e12) << endl;
	cout << formatFPNumber(1.3754937356458394574047e13) << endl;
	cout << formatFPNumber(1.3754937356458394574047e14) << endl;
	cout << formatFPNumber(1.3754937356458394574047e15) << endl;
	cout << formatFPNumber(1.3754937356458394574047e16) << endl;
	cout << formatFPNumber(1.3754937356458394574047e17) << endl;
	cout << formatFPNumber(1.3754937356458394574047e18) << endl;
	cout << formatFPNumber(1.3754937356458394574047e19) << endl;
	cout << formatFPNumber(1.3754937356458394574047e20) << endl;
	cout << formatFPNumber(1e-42) << endl;
	cout << formatFPNumber(1e42) << endl;
}

int main()
{
	test_lowercase();
	test_uppercase();
	test_formatFPNumber();
}
