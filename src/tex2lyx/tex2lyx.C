
/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

#include <config.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

using std::atoi;
using std::cout;
using std::cerr;
using std::endl;
using std::fill;
using std::getline;
using std::ios;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::stack;
using std::string;
using std::vector;
using std::string;
using std::stack;
using std::istringstream;
using std::ostringstream;
using std::ifstream;


namespace {

char const OPEN = '<';
char const CLOSE = '>';

const char * known_languages[] = { "austrian", "babel", "bahasa",
"basque", "breton", "bulgarian", "catalan", "croatian", "czech", "danish",
"dutch", "english", "esperanto", "estonian", "finnish", "francais",
"frenchb", "galician", "germanb", "greek", "hebcal", "hebfont", "hebrew",
"hebrew_newcode", "hebrew_oldcode", "hebrew_p", "hyphen", "icelandic",
"irish", "italian", "latin", "lgrcmr", "lgrcmro", "lgrcmss", "lgrcmtt",
"lgrenc", "lgrlcmss", "lgrlcmtt", "lheclas", "lhecmr", "lhecmss",
"lhecmtt", "lhecrml", "lheenc", "lhefr", "lheredis", "lheshold",
"lheshscr", "lheshstk", "lsorbian", "magyar", "naustrian", "ngermanb",
"ngerman", "norsk", "polish", "portuges", "rlbabel", "romanian",
"russianb", "samin", "scottish", "serbian", "slovak", "slovene", "spanish",
"swedish", "turkish", "ukraineb", "usorbian", "welsh", 0};

const char * known_fontsizes[] = { "10pt", "11pt", "12pt", 0 };


// some ugly stuff
string h_preamble;
string h_textclass               = "FIXME";
string h_options                 = "FIXME";
string h_language                = "FIXME";
string h_inputencoding           = "FIXME";
string h_fontscheme              = "FIXME";
string h_graphics                = "default";
string h_paperfontsize           = "FIXME";
string h_spacing                 = "single";
string h_papersize               = "FIXME";
string h_paperpackage            = "FIXME";
string h_use_geometry            = "0";
string h_use_amsmath             = "0";
string h_use_natbib              = "0";
string h_use_numerical_citations = "0";
string h_paperorientation        = "portrait";
string h_secnumdepth             = "3";
string h_tocdepth                = "3";
string h_paragraph_separation    = "indent";
string h_defskip                 = "medskip";
string h_quotes_language         = "2";
string h_quotes_times            = "1";
string h_papercolumns            = "1";
string h_papersides              = "1";
string h_paperpagestyle          = "default";
string h_tracking_changes        = "0";

// indicates whether we are in the preamble
bool in_preamble = true;

// current stack of nested environments
stack<string> active_environments;



void split(string const & s, vector<string> & result, char delim)
{
	istringstream is(s);	
	string t;
	while (getline(is, t, delim))
		result.push_back(t);
}


string join(vector<string> const & input, char delim)
{
	ostringstream os;
	for (size_t i = 0; i != input.size(); ++i) {
		if (i)
			os << delim;	
		os << input[i]; 
	}
	return os.str();
}


void handle_opt(vector<string> & opts, char const ** what, string & target)
{
	for ( ; what; ++what) {
		vector<string>::iterator it = find(opts.begin(), opts.end(), *what);
		if (it != opts.end()) {
			//cerr << "### found option '" << *what << "'\n";
			target = *what;
			opts.erase(it);
			return;
		}
	}
}


void handle_ert(ostream & os, string const & s)
{
	os << "\n\\begin_inset ERT\nstatus Collapsed\n\n\\layout Standard\n\n";
	os << s;
	os << "\n\\end_inset\n";
}


string wrap(string const & cmd, string const & str)
{
	return OPEN + cmd + ' ' + str + CLOSE;
}


string wrap(string const & cmd, string const & str, string const & str2)
{
	return OPEN + cmd + ' ' + str + ' ' + str2 + CLOSE;
}


enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE};

mode_type asMode(mode_type oldmode, string const & str)
{
	if (str == "mathmode")
		return MATH_MODE;
	if (str == "textmode" || str == "forcetext")
		return TEXT_MODE;
	return oldmode;
}


// These are TeX's catcodes
enum CatCode {
	catEscape,     // 0    backslash
	catBegin,      // 1    {
	catEnd,        // 2    }
	cat,       // 3    $
	catAlign,      // 4    &
	catNewline,    // 5    ^^M
	catParameter,  // 6    #
	catSuper,      // 7    ^
	catSub,        // 8    _
	catIgnore,     // 9
	catSpace,      // 10   space
	catLetter,     // 11   a-zA-Z
	catOther,      // 12   none of the above
	catActive,     // 13   ~
	catComment,    // 14   %
	catInvalid     // 15   <delete>
};

CatCode theCatcode[256];


inline CatCode catcode(unsigned char c)
{
	return theCatcode[c];
}


enum {
	FLAG_BRACE_LAST = 1 << 1,  //  last closing brace ends the parsing
	FLAG_RIGHT      = 1 << 2,  //  next \\right ends the parsing process
	FLAG_END        = 1 << 3,  //  next \\end ends the parsing process
	FLAG_BRACK_LAST = 1 << 4,  //  next closing bracket ends the parsing
	FLAG_TEXTMODE   = 1 << 5,  //  we are in a box
	FLAG_ITEM       = 1 << 6,  //  read a (possibly braced token)
	FLAG_LEAVE      = 1 << 7,  //  leave the loop at the end
	FLAG_SIMPLE     = 1 << 8,  //  next $ leaves the loop
	FLAG_EQUATION   = 1 << 9,  //  next \] leaves the loop
	FLAG_SIMPLE2    = 1 << 10, //  next \) leaves the loop
	FLAG_OPTION     = 1 << 11, //  read [...] style option
	FLAG_BRACED     = 1 << 12  //  read {...} style argument
};


void catInit()
{
	fill(theCatcode, theCatcode + 256, catOther);
	fill(theCatcode + 'a', theCatcode + 'z' + 1, catLetter);
	fill(theCatcode + 'A', theCatcode + 'Z' + 1, catLetter);

	theCatcode['\\'] = catEscape;
	theCatcode['{']  = catBegin;
	theCatcode['}']  = catEnd;
	theCatcode['$']  = cat;
	theCatcode['&']  = catAlign;
	theCatcode['\n'] = catNewline;
	theCatcode['#']  = catParameter;
	theCatcode['^']  = catSuper;
	theCatcode['_']  = catSub;
	theCatcode[''] = catIgnore;
	theCatcode[' ']  = catSpace;
	theCatcode['\t'] = catSpace;
	theCatcode['\r'] = catNewline;
	theCatcode['~']  = catActive;
	theCatcode['%']  = catComment;
}



//
// Helper class for parsing
//

class Token {
public:
	///
	Token() : cs_(), char_(0), cat_(catIgnore) {}
	///
	Token(char c, CatCode cat) : cs_(), char_(c), cat_(cat) {}
	///
	Token(string const & cs) : cs_(cs), char_(0), cat_(catIgnore) {}

	///
	string const & cs() const { return cs_; }
	///
	CatCode cat() const { return cat_; }
	///
	char character() const { return char_; }
	///
	string asString() const { return cs_.size() ? cs_ : string(1, char_); }

private:
	///
	string cs_;
	///
	char char_;
	///
	CatCode cat_;
};

ostream & operator<<(ostream & os, Token const & t)
{
	if (t.cs().size())
		os << '\\' << t.cs();
	else
		os << '[' << t.character() << ',' << t.cat() << ']';
	return os;
}


class Parser {

public:
	///
	Parser(istream & is);

	///
	string parse();
	///
	string parse(unsigned flags, mode_type mode);
	///
	int lineno() const { return lineno_; }
	///
	void putback();
	/// dump contents to screen
	void dump() const;

private:
	///
	string getArg(char left, char right);
	///
	char getChar();
	///
	void error(string const & msg);
	///
	void tokenize(istream & is);
	///
	void tokenize(string const & s);
	///
	void skipSpaceTokens(istream & is, char c);
	///
	void push_back(Token const & t);
	///
	void pop_back();
	///
	Token const & prevToken() const;
	///
	Token const & nextToken() const;
	///
	Token const & getToken();
	/// skips spaces if any
	void skipSpaces();
	///
	void lex(string const & s);
	///
	bool good() const;
	///
	string parse_verbatim_item();
	///
	string parse_verbatim_option();

	///
	int lineno_;
	///
	vector<Token> tokens_;
	///
	unsigned pos_;
};


Parser::Parser(istream & is)
	: lineno_(0), pos_(0)
{
	tokenize(is);
}


void Parser::push_back(Token const & t)
{
	tokens_.push_back(t);
}


void Parser::pop_back()
{
	tokens_.pop_back();
}


Token const & Parser::prevToken() const
{
	static const Token dummy;
	return pos_ > 0 ? tokens_[pos_ - 1] : dummy;
}


Token const & Parser::nextToken() const
{
	static const Token dummy;
	return good() ? tokens_[pos_] : dummy;
}


Token const & Parser::getToken()
{
	static const Token dummy;
	//cerr << "looking at token " << tokens_[pos_] << " pos: " << pos_ << '\n';
	return good() ? tokens_[pos_++] : dummy;
}


void Parser::skipSpaces()
{
	while (nextToken().cat() == catSpace || nextToken().cat() == catNewline)
		getToken();
}


void Parser::putback()
{
	--pos_;
}


bool Parser::good() const
{
	return pos_ < tokens_.size();
}


char Parser::getChar()
{
	if (!good())
		error("The input stream is not well...");
	return tokens_[pos_++].character();
}


string Parser::getArg(char left, char right)
{
	skipSpaces();

	string result;
	char c = getChar();

	if (c != left)
		putback();
	else
		while ((c = getChar()) != right && good())
			result += c;

	return result;
}


void Parser::skipSpaceTokens(istream & is, char c)
{
	// skip trailing spaces
	while (catcode(c) == catSpace || catcode(c) == catNewline)
		if (!is.get(c))
			break;
	//cerr << "putting back: " << c << "\n";
	is.putback(c);
}


void Parser::tokenize(istream & is)
{
	// eat everything up to the next \end_inset or end of stream
	// and store it in s for further tokenization
	string s;
	char c;
	while (is.get(c)) {
		s += c;
		if (s.size() >= 10 && s.substr(s.size() - 10) == "\\end_inset") {
			s = s.substr(0, s.size() - 10);
			break;
		}
	}
	// Remove the space after \end_inset
	if (is.get(c) && c != ' ')
		is.unget();

	// tokenize buffer
	tokenize(s);
}


void Parser::tokenize(string const & buffer)
{
	static bool init_done = false;

	if (!init_done) {
		catInit();
		init_done = true;
	}

	istringstream is(buffer.c_str(), ios::in | ios::binary);

	char c;
	while (is.get(c)) {
		//cerr << "reading c: " << c << "\n";

		switch (catcode(c)) {
			case catNewline: {
				++lineno_;
				is.get(c);
				if (catcode(c) == catNewline)
					push_back(Token("par"));
				else {
					push_back(Token('\n', catNewline));
					is.putback(c);
				}
				break;
			}

/*
			case catComment: {
				while (is.get(c) && catcode(c) != catNewline)
					;
				++lineno_;
				break;
			}
*/

			case catEscape: {
				is.get(c);
				if (!is) {
					error("unexpected end of input");
				} else {
					string s(1, c);
					if (catcode(c) == catLetter) {
						// collect letters
						while (is.get(c) && catcode(c) == catLetter)
							s += c;
						skipSpaceTokens(is, c);
					}
					push_back(Token(s));
				}
				break;
			}

			case catSuper:
			case catSub: {
				push_back(Token(c, catcode(c)));
				is.get(c);
				skipSpaceTokens(is, c);
				break;
			}

			case catIgnore: {
				cerr << "ignoring a char: " << int(c) << "\n";
				break;
			}

			default:
				push_back(Token(c, catcode(c)));
		}
	}

#ifdef FILEDEBUG
	dump();
#endif
}


void Parser::dump() const
{
	cerr << "\nTokens: ";
	for (unsigned i = 0; i < tokens_.size(); ++i) {
		if (i == pos_)
			cerr << " <#> ";
		cerr << tokens_[i];
	}
	cerr << " pos: " << pos_ << "\n";
}


void Parser::error(string const & msg)
{
	cerr << "Line ~" << lineno_ << ":  parse error: " << msg << endl;
	dump();
	//exit(1);
}


string Parser::parse()
{
	skipSpaces();
	return parse(0, UNDECIDED_MODE);
}


string Parser::parse_verbatim_option()
{
	string res;
	if (nextToken().character() == '[') {
		Token t = getToken();
		for (Token t = getToken(); t.character() != ']' && good(); t = getToken()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + parse_verbatim_item() + '}';
			} else
				res += t.asString();
		}
	}
	return res;
}


string Parser::parse_verbatim_item()
{
	string res;
	if (nextToken().cat() == catBegin) {
		Token t = getToken();
		for (Token t = getToken(); t.cat() != catEnd && good(); t = getToken()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + parse_verbatim_item() + '}';
			}
			else
				res += t.asString();
		}
	}
	return res;
}


string Parser::parse(unsigned flags, mode_type mode)
{
	//int limits = 0;

	ostringstream result;
	while (good()) {
		Token const & t = getToken();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
		cell->dump();
		cerr << "\n";
#endif

		if (flags & FLAG_ITEM) {
			if (t.cat() == catSpace)
				continue;

			flags &= ~FLAG_ITEM;
			if (t.cat() == catBegin) {
				// skip the brace and collect everything to the next matching
				// closing brace
				flags |= FLAG_BRACE_LAST;
				continue;
			}

			// handle only this single token, leave the loop if done
			flags |= FLAG_LEAVE;
		}


		if (flags & FLAG_BRACED) {
			if (t.cat() == catSpace)
				continue;

			if (t.cat() != catBegin) {
				error("opening brace expected");
				return result.str();
			}

			// skip the brace and collect everything to the next matching
			// closing brace
			flags = FLAG_BRACE_LAST;
		}


		if (flags & FLAG_OPTION) {
			if (t.cat() == catOther && t.character() == '[') {
				result << parse(FLAG_BRACK_LAST, mode);
			} else {
				// no option found, put back token and we are done
				putback();
			}
			return result.str();
		}

		//
		// cat codes
		//
		if (t.cat() == cat) {
			if (mode != MATH_MODE) {
				// we are inside some text mode thingy, so opening new math is allowed
				Token const & n = getToken();
				if (n.cat() == cat) {
					// TeX's $$...$$ syntax for displayed math
					result << wrap("equation", parse(FLAG_SIMPLE, MATH_MODE));
					getToken(); // skip the second '$' token
				} else {
					// simple $...$  stuff
					putback();
					result << wrap("simple", parse(FLAG_SIMPLE, MATH_MODE));
				}
			}

			else if (flags & FLAG_SIMPLE) {
				// this is the end of the formula
				return result.str();
			}

			else {
				error("something strange in the parser\n");
				break;
			}
		}

		else if (t.cat() == catLetter)
			result << t.character();

		else if (t.cat() == catSpace && mode != MATH_MODE) {
			//if (result.empty() || result[result.size() - 1] != ' ')
				result << t.character();
		}

		else if (t.cat() == catNewline && mode != MATH_MODE)
			result << t.character();

		else if (t.cat() == catParameter) {
			Token const & n	= getToken();
			result << wrap("macroarg", string(1, n.character()));
		}

		else if (t.cat() == catActive)
			result << wrap("active", string(1, t.character()));

		else if (t.cat() == catBegin)
			result << wrap("braced", parse(FLAG_BRACE_LAST, mode));

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return result.str();
			error("found '}' unexpectedly");
			//lyx::Assert(0);
			//add(cell, '}', LM_TC_TEX);
		}

/*
		else if (t.cat() == catAlign) {
			++cellcol;
			//cerr << " column now " << cellcol << " max: " << grid.ncols() << "\n";
			if (cellcol == grid.ncols()) {
				//cerr << "adding column " << cellcol << "\n";
				grid.addCol(cellcol - 1);
			}
			cell = &grid.cell(grid.index(cellrow, cellcol));
		}
*/

		else if (t.character() == ']' && (flags & FLAG_BRACK_LAST)) {
			//cerr << "finished reading option\n";
			return result.str();
		}

		else if (t.cat() == catOther)
			result << string(1, t.character());

		else if (t.cat() == catComment) {
			string s;
			while (good()) {
				Token const & t = getToken();
				if (t.cat() == catNewline)
					break;
				s += t.asString();
			}
			//result << wrap("comment", s);
			skipSpaces();
		}

		//
		// control sequences
		//

		else if (t.cs() == "lyxlock") {
			// ignored;
		}

		else if (t.cs() == "newcommand" || t.cs() == "providecommand") {
			string const name = parse_verbatim_item();
			string const opts = getArg('[', ']');
			string const body = parse_verbatim_item();
			// only non-lyxspecific stuff
			if (name != "noun" && name != "tabularnewline") {
				h_preamble += "\\" + t.cs() + "{" + name + "}";
				if (opts.size()) 
					h_preamble += "[" + opts + "]";
				h_preamble += "{" + body + "}\n";
			}
		}

		else if (t.cs() == "(") 
			result << wrap("simple", parse(FLAG_SIMPLE2, MATH_MODE));

		else if (t.cs() == "[")
			result << wrap("equation", parse(FLAG_EQUATION, MATH_MODE));

		else if (t.cs() == "protect")
			// ignore \\protect, will hopefully be re-added during output
			;

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = getArg('{', '}');
				if (name != active_environments.top())
					error("\\end{" + name + "} does not match \\begin{"
						+ active_environments.top() + "}");
				active_environments.pop();
				return result.str();
			}
			error("found 'end' unexpectedly");
		}

		else if (t.cs() == ")") {
			if (flags & FLAG_SIMPLE2)
				return result.str();
			error("found '\\)' unexpectedly");
		}

		else if (t.cs() == "]") {
			if (flags & FLAG_EQUATION)
				return result.str();
			error("found '\\]' unexpectedly");
		}

/*
		else if (t.cs() == "\\") {
			grid.vcrskip(LyXLength(getArg('[', ']')), cellrow);
			++cellrow;
			cellcol = 0;
			if (cellrow == grid.nrows())
				grid.addRow(cellrow - 1);
			if (grid.asHullstring())
				grid.asHullstring()->numbered(cellrow, numbered);
			cell = &grid.cell(grid.index(cellrow, cellcol));
		}
*/
		else if (t.cs() == "documentclass") {
			vector<string> opts;
			split(getArg('[', ']'), opts, ',');
			handle_opt(opts, known_languages, h_language); 
			handle_opt(opts, known_fontsizes, h_paperfontsize); 
			h_options = join(opts, ',');
			h_textclass = getArg('{', '}');
		}

		else if (t.cs() == "usepackage") {
			string const options = getArg('[', ']');
			string const name = getArg('{', '}');
			if (name == "a4wide") {
				h_papersize = "a4";
				h_paperpackage = "widemarginsa4";
			} else if (name == "ae") 
				h_fontscheme = "ae";
			else if (name == "aecompl") 
				h_fontscheme = "ae";
			else if (name == "amsmath") 
				h_use_amsmath = "1";
			else if (name == "amssymb") 
				h_use_amsmath = "1";
			else if (name == "babel") 
				; // ignore this
			else if (name == "fontenc") 
				; // ignore this
			else if (name == "inputenc") 
				h_inputencoding = options;
			else if (name == "makeidx") 
				; // ignore this
			else if (name == "verbatim") 
				; // ignore this
			else {
				if (options.size())
					h_preamble += "\\usepackage[" + options + "]{" + name + "}\n";
				else
					h_preamble += "\\usepackage{" + name + "}\n";
			}
		}

		else if (t.cs() == "newenvironment") {
			string const name = getArg('{', '}');
			skipSpaces();
			string const begin = parse_verbatim_item();
			skipSpaces();
			string const end = parse_verbatim_item();
			// ignore out mess
			if (name != "lyxcode") 
				result << wrap("newenvironment", begin + end); 
		}

		else if (t.cs() == "def") {
			string const name = getToken().cs();
			string res;
			while (nextToken().cat() != catBegin)
				res += getToken().asString();
			handle_ert(result, "\\def" + res + '{' + parse_verbatim_item() + '}');
		}

		else if (t.cs() == "setcounter") {
			string const name = getArg('{', '}');
			string const content = getArg('{', '}');
			if (name == "secnumdepth") 
				h_secnumdepth = content;
			else if (name == "tocdepth") 
				h_tocdepth = content;
			else
				h_preamble += "\\setcounter{" + name + "}{" + content + "}\n";
		}

		else if (t.cs() == "setlength") {
			string const name = getToken().cs();
			string const content = getArg('{', '}');
			if (name == "parskip")
				h_paragraph_separation = "skip";
			else if (name == "parindent")
				h_paragraph_separation = "skip";
			else
				h_preamble += "\\setcounter{" + name + "}{" + content + "}\n";
		}
	
		else if (t.cs() == "par") {
			if (!active_environments.empty()) 	
				result << "\n\\layout " << active_environments.top() << "\n\n";
		}

		else if (t.cs() == "title")
			result << "\\layout Title\n\n" + parse_verbatim_item();

		else if (t.cs() == "author")
			result << "\\layout Author\n\n" + parse_verbatim_item();

		else if (t.cs() == "abstract")
			result << "\\layout Abstract\n\n" + parse_verbatim_item();

		else if (t.cs() == "begin") {
			string const name = getArg('{', '}');
			active_environments.push(name);
			result << parse(FLAG_END, mode);
		}

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}

	return result.str();
}


} // anonymous namespace


int main(int argc, char * argv[])
{
	if (argc <= 1) {
		cerr << "Usage: " << argv[0] << " <infile.tex>" << endl;
		return 2;
	}

	string t;
	ifstream is(argv[1]);
	Parser p(is);
	//p.dump();
	string s = p.parse();
	cout << "# tex2lyx 0.0.2 created this file\n"
	     << "\\lyxformat 222\n"
	     << "\\textclass " << h_textclass << "\n"
	     << "\\begin_preamble\n" << h_preamble << "\\end_preamble\n"
	     << "\\options " << h_options << "\n"
	     << "\\language " << h_language << "\n"
	     << "\\inputencoding " << h_inputencoding << "\n"
	     << "\\fontscheme " << h_fontscheme << "\n"
	     << "\\graphics " << h_graphics << "\n"
	     << "\\paperfontsize " << h_paperfontsize << "\n"
	     << "\\spacing " << h_spacing << "\n"
	     << "\\papersize " << h_papersize << "\n"
	     << "\\paperpackage " << h_paperpackage << "\n"
	     << "\\use_geometry " << h_use_geometry << "\n"
	     << "\\use_amsmath " << h_use_amsmath << "\n"
	     << "\\use_natbib " << h_use_natbib << "\n"
	     << "\\use_numerical_citations " << h_use_numerical_citations << "\n"
	     << "\\paperorientation " << h_paperorientation << "\n"
	     << "\\secnumdepth " << h_secnumdepth << "\n"
	     << "\\tocdepth " << h_tocdepth << "\n"
	     << "\\paragraph_separation " << h_paragraph_separation << "\n"
	     << "\\defskip " << h_defskip << "\n"
	     << "\\quotes_language " << h_quotes_language << "\n"
	     << "\\quotes_times " << h_quotes_times << "\n"
	     << "\\papercolumns " << h_papercolumns << "\n"
	     << "\\papersides " << h_papersides << "\n"
	     << "\\paperpagestyle " << h_paperpagestyle << "\n"
	     << "\\tracking_changes " << h_tracking_changes << "\n"
	     << s << "\n"
	     << "\\the_end";

	return 0;	
}	
