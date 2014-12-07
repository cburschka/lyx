#include <config.h>

#include "../trivstring.h"

#include <iostream>


using namespace lyx;

using namespace std;

void test_trivstring()
{
	string const input[] = {
		"",
		"a",
		"42",
		"max sso", // max. string with sso on 64 bit
		"something which does not fit into sso"
	};
	size_t const n = sizeof(input) / sizeof(input[0]);
	for (size_t i = 0; i < n; ++i) {
		// construction from std::string
		trivstring const a(input[i]);
		// construction from trivstring
		trivstring const b(a);
		// assignment from trivstring
		trivstring const c = a;
		// assignment from std::string
		trivstring const d = input[i];
		// assignment from trivstring
		string const e = a.str();
		// assignment from trivstring via C string
		string const f = a.c_str();
		cout << a.length() << endl;
		cout << a.str() << endl;
		cout << b.str() << endl;
		cout << c.str() << endl;
		cout << d.str() << endl;
		cout << e << endl;
		cout << f << endl;
	}
}

int main()
{
	test_trivstring();
}
