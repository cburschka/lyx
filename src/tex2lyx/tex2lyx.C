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
#include <string>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::getline;
using std::istream;
using std::ifstream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::stringstream;
using std::vector;


void handle_comment(Parser & p)
{
	string s;
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() == catNewline)
			break;
		s += t.asString();
	}
	//cerr << "comment: " << s << "\n";
	p.skip_spaces();
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
	istringstream is(STRCONV(s));
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
	return STRCONV(os.str());
}


char const ** is_known(string const & str, char const ** what)
{
	for ( ; *what; ++what)
		if (str == *what)
			return what;
	return 0;
}



// current stack of nested environments
vector<string> active_environments;


string active_environment()
{
	return active_environments.empty() ? string() : active_environments.back();
}


void clean_layouts(istream & is, ostream & os)
{
	string last;
	string line;
	bool eating = false;
	while (getline(is, line)) {
		string tline = trim(line, " ");
		if (line.substr(0, 8) == "\\layout ") {
			//cerr << "layout: " << line << "\n";
			last = line;
			eating = true;
		} else if (eating && tline.empty()) {
			//cerr << "eat empty line\n"; 
		} else if (line.substr(0, 13) == "\\begin_deeper") {
			os << line << "\n";
		} else {
			// ordinary line  
			//cerr << "ordinary line\n"; 
			if (eating) {
				eating = false;
				os << last << "\n\n";
			}
			os << line << "\n";
		}
	}
}


int main(int argc, char * argv[])
{
	if (argc <= 1) {
		cerr << "Usage: " << argv[0] << " <infile.tex>" << endl;
		return 2;
	}

	ifstream is(argv[1]);
	Parser p(is);
	//p.dump();

	stringstream ss;
	parse_preamble(p, ss);
	active_environments.push_back("document");
	parse_text(p, ss, FLAG_END, true);
	ss << "\n\\the_end\n";

	ss.seekg(0);
	clean_layouts(ss, cout);

	return 0;
}

// }])
