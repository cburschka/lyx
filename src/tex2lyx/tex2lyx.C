/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "Lsstream.h"
#include "tex2lyx.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::stack;
using std::string;
using std::vector;


//namespace {


void handle_comment(Parser & p)
{
	string s;
	while (p.good()) {
		Token const & t = p.getToken();
		if (t.cat() == catNewline)
			break;
		s += t.asString();
	}
	//cerr << "comment: " << s << "\n";
	p.skipSpaces();
}




string const trim(string const & a, char const * p)
{
	// lyx::Assert(p);

	if (a.empty() || !*p)
		return a;

	string::size_type r = a.find_last_not_of(p);
	string::size_type l = a.find_first_not_of(p);

	// Is this the minimal test? (lgb)
	if (r == string::npos && l == string::npos)
		return string();

	return a.substr(l, r - l + 1);
}


void split(string const & s, vector<string> & result, char delim)
{
	//cerr << "split 1: '" << s << "'\n";
	istringstream is(s);
	string t;
	while (getline(is, t, delim))
		result.push_back(t);
	//cerr << "split 2\n";
}


string join(vector<string> const & input, char const * delim)
{
	ostringstream os;
	for (size_t i = 0; i < input.size(); ++i) {
		if (i)
			os << delim;
		os << input[i];
	}
	return os.str();
}


char const ** is_known(string const & str, char const ** what)
{
	for ( ; *what; ++what)
		if (str == *what)
			return what;
	return 0;
}



// current stack of nested environments
stack<string> active_environments;


void active_environments_push(std::string const & name)
{
	active_environments.push(name);
}


void active_environments_pop()
{
	active_environments.pop();
}


bool active_environments_empty()
{
	return active_environments.empty();
}


string curr_env()
{
	return active_environments.empty() ? string() : active_environments.top();
}




//} // anonymous namespace


int main(int argc, char * argv[])
{
	if (argc <= 1) {
		cerr << "Usage: " << argv[0] << " <infile.tex>" << endl;
		return 2;
	}

	ifstream is(argv[1]);
	Parser p(is);
	parse_preamble(p, cout);
	active_environments.push("document");
	parse_text(p, cout, FLAG_END, true);
	cout << "\n\\the_end";

	return 0;
}

// }])
