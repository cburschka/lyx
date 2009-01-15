#include <config.h>

#include "../convert.h"
#include "../docstring.h"

#include <iostream>

using lyx::convert;

using namespace std;


namespace lyx {
	docstring const _(string const & s) { return from_ascii(s); }
}


void convert_int()
{
	cout << convert<int>("123") << '\n'
	     << convert<int>(" 123") << '\n'
	     << convert<int>("123 ") << '\n'
	     << convert<int>(" 123 ") << '\n'

	     << convert<int>("123 123") << '\n'
	     << convert<int>(" 123 123") << '\n'
	     << convert<int>("123 123 ") << '\n'
	     << convert<int>(" 123 123 ") << '\n'

	     << convert<int>("-123") << '\n'
	     << convert<int>(" -123") << '\n'
	     << convert<int>("-123 ") << '\n'
	     << convert<int>(" -123 ") << '\n'

	     << convert<int>("-123 123") << '\n'
	     << convert<int>(" -123 123") << '\n'
	     << convert<int>("-123 123 ") << '\n'
	     << convert<int>(" -123 123 ") << '\n'

	     << convert<int>("") << '\n'
	     << convert<int>("abc") << '\n'
	     << convert<int>(" abc") << '\n'
	     << convert<int>("abc ") << '\n'
	     << convert<int>(" abc ") << '\n'

	     << convert<int>(" 123 abc") << '\n'
	     << convert<int>("123 abc ") << '\n'
	     << convert<int>(" 123 abc ") << '\n'
	     << convert<int>("123  abc") << endl;
}


void convert_unsigned()
{
	cout << convert<unsigned int>(string("123")) << '\n'
	     << convert<unsigned int>(string(" 123")) << '\n'
	     << convert<unsigned int>(string("123 ")) << '\n'
	     << convert<unsigned int>(string(" 123 ")) << '\n'

	     << convert<unsigned int>(string("123 123")) << '\n'
	     << convert<unsigned int>(string(" 123 123")) << '\n'
	     << convert<unsigned int>(string("123 123 ")) << '\n'
	     << convert<unsigned int>(string(" 123 123 ")) << '\n'

	     << convert<unsigned int>(string()) << '\n'
	     << convert<unsigned int>(string("abc")) << '\n'
	     << convert<unsigned int>(string(" abc")) << '\n'
	     << convert<unsigned int>(string("abc ")) << '\n'
	     << convert<unsigned int>(string(" abc ")) << '\n'

	     << convert<unsigned int>(string(" 123 abc")) << '\n'
	     << convert<unsigned int>(string("123 abc ")) << '\n'
	     << convert<unsigned int>(string(" 123 abc ")) << '\n'
	     << convert<unsigned int>(string("123  abc")) << endl;
}


void convert_string()
{
	cout << convert<string>(123) << '\n'
	     << convert<string>(4294967173u) << '\n'
	     << convert<string>(true) << '\n'
	     << convert<string>(false) << '\n'

	     << convert<string>('a') << '\n'
	     << convert<string>(1.0) << '\n'
	     << convert<string>(1.5) << endl;
}

int main()
{
	convert_int();
	convert_unsigned();
	convert_string();
}
