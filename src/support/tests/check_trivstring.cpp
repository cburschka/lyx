#include <config.h>

#include "../trivstring.h"
#include "../docstring.h"

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
		trivstring d = input[i];
		// assignment from trivstring
		string const e = a;
		// assignment from trivstring via C string
		string const f = a.c_str();
		if (a.empty())
			cout << "empty ";
		else
			cout << "not empty ";
		cout << a.length() << endl;
		cout << a << endl;
		cout << b << endl;
		cout << c << endl;
		cout << d << endl;
		cout << e << endl;
		cout << f << endl;
		// swap
		trivstring g("swap");
		cout << g << endl;
		d.swap(g);
		cout << d << endl;
		cout << g << endl;
	}
	// comparison
	trivstring const a;
	trivstring const b("a");
	trivstring const c("b");
	trivstring const d("42");
	cout << (a == a) << ' ' << (a < a) << endl; // equal strings
	cout << (a == b) << ' ' << (a < b) << endl; // different strings, same length
	cout << (b == a) << ' ' << (b < a) << endl; // different strings, same length
	cout << (a == c) << ' ' << (a < c) << endl; // different strings, different length
	cout << (c == a) << ' ' << (c < a) << endl; // different strings, different length
	char const * e = "";
	char const * f = "b";
	char const * g = "42";
	cout << (a == e) << ' ' << (e == a) << endl; // empty strings
	cout << (c == a) << ' ' << (a == c) << endl; // equal strings
	cout << (a == f) << ' ' << (f == a) << endl; // different strings, same length
	cout << (a == g) << ' ' << (g == a) << endl; // different strings, different length
	// operator[]
	cout << d[1] << d[0] << endl;
	// substr()
	cout << d.substr(1) << endl;    // default argument
	cout << d.substr(1, 1) << endl; // maximum length
	cout << d.substr(1, 2) << endl; // length larger than max
	cout << d.substr(2) << endl;    // maximum pos
}

void test_trivdocstring()
{
	docstring const input[] = {
		from_ascii(""),
		from_ascii("a"),
		from_ascii("42"),
		from_ascii("max"), // max. string with sso on 64 bit
		from_ascii("something which does not fit into sso")
	};
	size_t const n = sizeof(input) / sizeof(input[0]);
	for (size_t i = 0; i < n; ++i) {
		// construction from std::string
		trivdocstring const a(input[i]);
		// construction from trivstring
		trivdocstring const b(a);
		// assignment from trivstring
		trivdocstring const c = a;
		// assignment from std::string
		trivdocstring d = input[i];
		// assignment from trivstring
		docstring const e = a;
		// assignment from trivstring via C string
		docstring const f = a.c_str();
		if (a.empty())
			cout << "empty ";
		else
			cout << "not empty ";
		cout << a.length() << endl;
		cout << to_ascii(a) << endl;
		cout << to_ascii(b) << endl;
		cout << to_ascii(c) << endl;
		cout << to_ascii(d) << endl;
		cout << to_ascii(e) << endl;
		cout << to_ascii(f) << endl;
		// swap
		trivdocstring g(from_ascii("swap"));
		cout << to_ascii(g) << endl;
		d.swap(g);
		cout << to_ascii(d) << endl;
		cout << to_ascii(g) << endl;
	}
	// comparison
	trivdocstring const a;
	trivdocstring const b(from_ascii("a"));
	trivdocstring const c(from_ascii("b"));
	trivdocstring const d(from_ascii("42"));
	cout << (a == a) << ' ' << (a < a) << endl; // equal strings
	cout << (a == b) << ' ' << (a < b) << endl; // different strings, same length
	cout << (b == a) << ' ' << (b < a) << endl; // different strings, same length
	cout << (a == c) << ' ' << (a < c) << endl; // different strings, different length
	cout << (c == a) << ' ' << (c < a) << endl; // different strings, different length
	// per character initialization works also if char_type != wchar
	char_type const e[1] = {'\0'};
	char_type const f[2] = {'b', '\0'};
	char_type const g[3] = {'4', '2', '\0'};
	cout << (a == e) << ' ' << (e == a) << endl; // empty strings
	cout << (c == a) << ' ' << (a == c) << endl; // equal strings
	cout << (a == f) << ' ' << (f == a) << endl; // different strings, same length
	cout << (a == g) << ' ' << (g == a) << endl; // different strings, different length
	// operator[]
	cout << static_cast<char>(d[1]) << static_cast<char>(d[0]) << endl;
	// substr()
	cout << to_ascii(d.substr(1)) << endl;    // default argument
	cout << to_ascii(d.substr(1, 1)) << endl; // maximum length
	cout << to_ascii(d.substr(1, 2)) << endl; // length larger than max
	cout << to_ascii(d.substr(2)) << endl;    // maximum pos
}

int main()
{
	test_trivstring();
	test_trivdocstring();
}
