/** The math parser
    \author André Pönitz (2001)
 */

/*

If someone desperately needs partial "structures" (such as a few cells of
an array inset or similar) (s)he could uses the following hack as starting
point to write some macros:

  \newif\ifcomment
  \commentfalse
  \ifcomment
	  \def\makeamptab{\catcode`\&=4\relax}
	  \def\makeampletter{\catcode`\&=11\relax}
    \def\b{\makeampletter\expandafter\makeamptab\bi}
    \long\def\bi#1\e{}
  \else
    \def\b{}\def\e{}
  \fi

  ...

  \[\begin{array}{ccc}
   1 & 2\b & 3^2\\
   4 & 5\e & 6\\
   7 & 8 & 9
  \end{array}\]

*/


#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_parser.h"
#include "math_inset.h"
#include "math_arrayinset.h"
#include "math_braceinset.h"
#include "math_boxinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_factory.h"
#include "math_kerninset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_hullinset.h"
#include "math_rootinset.h"
#include "math_sizeinset.h"
#include "math_sqrtinset.h"
#include "math_scriptinset.h"
#include "math_specialcharinset.h"
#include "math_sqrtinset.h"
#include "math_support.h"
#include "math_xyarrowinset.h"

#include "lyxlex.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/lstrings.h"

#include <cctype>
#include <stack>
#include <algorithm>

using std::istream;
using std::ostream;
using std::ios;
using std::endl;
using std::stack;
using std::fill;
using std::vector;
using std::atoi;

//#define FILEDEBUG


namespace {

bool stared(string const & s)
{
	string::size_type const n = s.size();
	return n && s[n - 1] == '*';
}


void add(MathArray & ar, char c)
{
	ar.push_back(MathAtom(new MathCharInset(c)));
}


// These are TeX's catcodes
enum CatCode {
	catEscape,     // 0    backslash
	catBegin,      // 1    {
	catEnd,        // 2    }
	catMath,       // 3    $
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
	FLAG_BRACE_LAST = 1 << 1,  //  last closing brace ends the parsing process
	FLAG_RIGHT      = 1 << 2,  //  next \\right ends the parsing process
	FLAG_END        = 1 << 3,  //  next \\end ends the parsing process
	FLAG_BRACK_END  = 1 << 4,  //  next closing bracket ends the parsing process
	FLAG_BOX        = 1 << 5,  //  we are in a box
	FLAG_ITEM       = 1 << 6,  //  read a (possibly braced token)
	FLAG_BLOCK      = 1 << 7,  //  next block ends the parsing process
	FLAG_BLOCK2     = 1 << 8,  //  next block2 ends the parsing process
	FLAG_LEAVE      = 1 << 9,  //  leave the loop at the end
	FLAG_SIMPLE     = 1 << 10, //  next $ leaves the loop
	FLAG_EQUATION   = 1 << 11, //  next \] leaves the loop
	FLAG_SIMPLE2    = 1 << 12  //  next \) leaves the loop
};


void catInit()
{
	fill(theCatcode, theCatcode + 256, catOther);
	fill(theCatcode + 'a', theCatcode + 'z' + 1, catLetter);
	fill(theCatcode + 'A', theCatcode + 'Z' + 1, catLetter);

	theCatcode['\\'] = catEscape;
	theCatcode['{']  = catBegin;
	theCatcode['}']  = catEnd;
	theCatcode['$']  = catMath;
	theCatcode['&']  = catAlign;
	theCatcode['\n'] = catNewline;
	theCatcode['#']  = catParameter;
	theCatcode['^']  = catSuper;
	theCatcode['_']  = catSub;
	theCatcode[''] = catIgnore;
	theCatcode[' ']  = catSpace;
	theCatcode['\t'] = catSpace;
	theCatcode['\r'] = catSpace;
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
	string asString() const;
	///
	bool isCR() const;

private:
	///
	string cs_;
	///
	char char_;
	///
	CatCode cat_;
};

bool Token::isCR() const
{
	return cs_ == "\\" || cs_ == "cr" || cs_ == "crcr";
}

string Token::asString() const
{
	return cs_.size() ? cs_ : string(1, char_);
}

ostream & operator<<(ostream & os, Token const & t)
{
	if (t.cs().size())
		os << "\\" << t.cs();
	else
		os << "[" << t.character() << "," << t.cat() << "]";
	return os;
}


class Parser {

public:
	///
	Parser(LyXLex & lex);
	///
	Parser(istream & is);

	///
	bool parse_macro(string & name);
	///
	bool parse_normal(MathAtom &);
	///
	void parse_into(MathArray & array, unsigned flags);
	///
	int lineno() const { return lineno_; }
	///
	void putback();

private:
	///
	void parse_into1(MathGridInset & grid, unsigned flags, bool numbered);
	///
	void parse_into2(MathAtom & at, unsigned flags, bool numbered);
	/// get arg delimited by 'left' and 'right'
	string getArg(char left, char right);
	///
	char getChar();
	///
	void error(string const & msg);
	/// dump contents to screen
	void dump() const;

private:
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
	int lineno_;
	///
	vector<Token> tokens_;
	///
	unsigned pos_;
};


Parser::Parser(LyXLex & lexer)
	: lineno_(lexer.getLineNo()), pos_(0)
{
	tokenize(lexer.getStream());
	lexer.eatLine();
}


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
	//lyxerr << "looking at token " << tokens_[pos_] << " pos: " << pos_ << '\n';
	return good() ? tokens_[pos_++] : dummy;
}


void Parser::skipSpaces()
{
	while (nextToken().cat() == catSpace)
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
	if (!good()) {
		lyxerr << "The input stream is not well..." << endl;
		dump();
	}
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

	// tokenize buffer
	tokenize(s);
}


void Parser::skipSpaceTokens(istream & is, char c)
{
	// skip trailing spaces
	while (catcode(c) == catSpace || catcode(c) == catNewline)
		if (!is.get(c))
			break;
	//lyxerr << "putting back: " << c << "\n";
	is.putback(c);
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
		//lyxerr << "reading c: " << c << "\n";

		switch (catcode(c)) {
			case catNewline: {
				++lineno_;
				is.get(c);
				if (catcode(c) == catNewline)
					; //push_back(Token("par"));
				else {
					push_back(Token(' ', catSpace));
					is.putback(c);
				}
				break;
			}

			case catComment: {
				while (is.get(c) && catcode(c) != catNewline)
					;
				++lineno_;
				break;
			}

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
				lyxerr << "ignoring a char: " << int(c) << "\n";
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
	lyxerr << "\nTokens: ";
	for (unsigned i = 0; i < tokens_.size(); ++i) {
		if (i == pos_)
			lyxerr << " <#> ";
		lyxerr << tokens_[i];
	}
	lyxerr << "\n";
}


void Parser::error(string const & msg)
{
	lyxerr << "Line ~" << lineno_ << ": Math parse error: " << msg << endl;
	dump();
	//exit(1);
}


bool Parser::parse_macro(string & name)
{
	int nargs = 0;
	name = "{error}";
	skipSpaces();

	if (nextToken().cs() == "def") {

		getToken();
		name = getToken().cs();

		string pars;
		while (good() && nextToken().cat() != catBegin)
			pars += getToken().cs();

		if (!good()) {
			lyxerr << "bad stream in parse_macro\n";
			dump();
			return false;
		}

		//lyxerr << "read \\def parameter list '" << pars << "'\n";
		if (!pars.empty()) {
			lyxerr << "can't handle non-empty parameter lists\n";
			dump();
			return false;
		}

	} else if (nextToken().cs() == "newcommand") {

		getToken();

		if (getToken().cat() != catBegin) {
			lyxerr << "'{' in \\newcommand expected (1) \n";
			dump();
			return false;
		}

		name = getToken().cs();

		if (getToken().cat() != catEnd) {
			lyxerr << "'}' expected\n";
			return false;
		}

		string arg  = getArg('[', ']');
		if (!arg.empty())
			nargs = atoi(arg.c_str());

	} else {
		lyxerr << "\\newcommand or \\def  expected\n";
		return false;
	}


	if (getToken().cat() != catBegin) {
		lyxerr << "'{' in macro definition expected (2)\n";
		return false;
	}

	MathArray ar1;
	parse_into(ar1, FLAG_BRACE_LAST);

	// we cannot handle recursive stuff at all
	MathArray test;
	test.push_back(createMathInset(name));
	if (ar1.contains(test)) {
		lyxerr << "we cannot handle recursive macros at all.\n";
		return false;
	}

	// is a version for display attached?
	MathArray ar2;
	parse_into(ar2, FLAG_ITEM);

	MathMacroTable::create(name, nargs, ar1, ar2);
	return true;
}
 

bool Parser::parse_normal(MathAtom & matrix)
{
	skipSpaces();
	Token const & t = getToken();

	if (t.cs() == "(") {
		matrix = MathAtom(new MathHullInset(LM_OT_SIMPLE));
		parse_into2(matrix, FLAG_SIMPLE2, true);
		return true;
	}

	if (t.cat() == catMath) {
		Token const & n = getToken();
		if (n.cat() == catMath) {
			// TeX's $$...$$ syntax for displayed math
			matrix = MathAtom(new MathHullInset(LM_OT_EQUATION));
			parse_into2(matrix, FLAG_SIMPLE, false);
			getToken(); // skip the second '$' token
		} else {
			// simple $...$  stuff
			putback();
			matrix = MathAtom(new MathHullInset(LM_OT_SIMPLE));
			parse_into2(matrix, FLAG_SIMPLE, false);
		}
		return true;
	}

	if (!t.cs().size()) {
		lyxerr << "start of math expected, got '" << t << "'\n";
		return false;
	}

	string const & cs = t.cs();

	if (cs == "[") {
		matrix = MathAtom(new MathHullInset(LM_OT_EQUATION));
		parse_into2(matrix, FLAG_EQUATION, true);
		return true;
	}

	if (cs != "begin") {
		lyxerr[Debug::MATHED]
			<< "'begin' of un-simple math expected, got '" << cs << "'\n";
		return false;
	}

	string const name = getArg('{', '}');

	if (name == "math") {
		matrix = MathAtom(new MathHullInset(LM_OT_SIMPLE));
		parse_into2(matrix, FLAG_SIMPLE, true);
		return true;
	}

	if (name == "equation" || name == "equation*" || name == "displaymath") {
		matrix = MathAtom(new MathHullInset(LM_OT_EQUATION));
		parse_into2(matrix, FLAG_END, (name == "equation"));
		return true;
	}

	if (name == "eqnarray" || name == "eqnarray*") {
		matrix = MathAtom(new MathHullInset(LM_OT_EQNARRAY));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	if (name == "align" || name == "align*") {
		matrix = MathAtom(new MathHullInset(LM_OT_ALIGN));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	if (name == "alignat" || name == "alignat*") {
		// ignore this for a while
		getArg('{', '}');
		matrix = MathAtom(new MathHullInset(LM_OT_ALIGNAT));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	if (name == "xalignat" || name == "xalignat*") {
		// ignore this for a while
		getArg('{', '}');
		matrix = MathAtom(new MathHullInset(LM_OT_XALIGNAT));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	if (name == "xxalignat") {
		// ignore this for a while
		getArg('{', '}');
		matrix = MathAtom(new MathHullInset(LM_OT_XXALIGNAT));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	if (name == "multline" || name == "multline*") {
		matrix = MathAtom(new MathHullInset(LM_OT_MULTLINE));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	if (name == "gather" || name == "gather*") {
		matrix = MathAtom(new MathHullInset(LM_OT_GATHER));
		parse_into2(matrix, FLAG_END, !stared(name));
		return true;
	}

	lyxerr[Debug::MATHED] << "1: unknown math environment: " << name << "\n";
	lyxerr << "1: unknown math environment: " << name << "\n";
	return false;
}


void Parser::parse_into(MathArray & array, unsigned flags)
{
	MathGridInset grid(1, 1);
	parse_into1(grid, flags, false);
	array = grid.cell(0);
	// remove 'unnecessary' braces:
	if (array.size() == 1 && array.back()->asBraceInset()) {
		lyxerr << "extra braces removed\n";
		array = array.back()->asBraceInset()->cell(0);
	}
}


void Parser::parse_into2(MathAtom & at, unsigned flags, bool numbered)
{
	parse_into1(*(at->asGridInset()), flags, numbered);
}


void Parser::parse_into1(MathGridInset & grid, unsigned flags, bool numbered)
{
	bool panic  = false;
	int  limits = 0;
	MathGridInset::row_type cellrow = 0;
	MathGridInset::col_type cellcol = 0;
	MathArray * cell = &grid.cell(grid.index(cellrow, cellcol));

	if (grid.asHullInset())
		grid.asHullInset()->numbered(cellrow, numbered);

	//dump();
	//lyxerr << "grid: " << grid << endl;

	while (good()) {
		Token const & t = getToken();

#ifdef FILEDEBUG
		lyxerr << "t: " << t << " flags: " << flags << "\n";
		//cell->dump();
		lyxerr << "\n";
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

		if (flags & FLAG_BLOCK) {
			if (t.cat() == catAlign || t.isCR() || t.cs() == "end") {
				putback();
				return;
			}
		}

		if (flags & FLAG_BLOCK2) {
			if (t.cat() == catAlign || t.isCR() || t.cs() == "end"
					|| t.cat() == catEnd) {
				putback();
				return;
			}
		}

		//
		// cat codes
		//
		if (t.cat() == catMath) {
			if (flags & FLAG_SIMPLE) {
				// this is the end of the formula
				return;
			}
				
			if (flags & FLAG_BOX) {
				// we are inside an mbox, so opening new math is allowed
				push_back(t);
				cell->push_back(MathAtom(new MathHullInset(LM_OT_SIMPLE)));
				parse_normal(cell->back());
			} else {
				lyxerr << "somthing strange in the parser\n";
				break;
			}
		}

		else if (t.cat() == catLetter)
			add(*cell, t.character());

		else if (t.cat() == catSpace) //&& code == LM_TC_TEXTRM
			add(*cell, t.character());

		else if (t.cat() == catParameter) {
			Token const & n	= getToken();
			cell->push_back(MathAtom(new MathMacroArgument(n.character()-'0')));
		}

		else if (t.cat() == catBegin) {
			MathArray ar;
			parse_into(ar, FLAG_BRACE_LAST);
#ifndef WITH_WARNINGS
#warning this might be wrong in general!
#endif
			// ignore braces around simple items
			if ((ar.size() == 1 && !ar.front()->needsBraces()
       || (ar.size() == 2 && !ar.front()->needsBraces()
					    && ar.back()->asScriptInset()))
       || (ar.size() == 0 && cell->size() == 0))
			{
				cell->push_back(ar);
			} else {
				cell->push_back(MathAtom(new MathBraceInset));
				cell->back()->cell(0).swap(ar);
			}
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			lyxerr << "found '}' unexpectedly, cell: '" << cell << "'\n";
			dump();
			//lyxerr << "found '}' unexpectedly\n";
			//lyx::Assert(0);
			//add(cell, '}', LM_TC_TEX);
		}

		else if (t.cat() == catAlign) {
			++cellcol;
			lyxerr << " column now " << cellcol << " max: " << grid.ncols() << "\n";
			if (cellcol == grid.ncols()) { 
				lyxerr << "adding column " << cellcol << "\n";
				grid.addCol(cellcol - 1);
			}
			cell = &grid.cell(grid.index(cellrow, cellcol));
		}

		else if (t.cat() == catSuper || t.cat() == catSub) {
			bool up = (t.cat() == catSuper);
			MathScriptInset * p = 0;
			if (cell->size())
				p = cell->back()->asScriptInset();
			if (!p || p->has(up)) {
				cell->push_back(MathAtom(new MathScriptInset(up)));
				p = cell->back()->asScriptInset();
			}
			p->ensure(up);
			parse_into(p->cell(up), FLAG_ITEM);
			p->limits(limits);
			limits = 0;
		}

		else if (t.character() == ')' && (flags & FLAG_SIMPLE2))
			return;

		else if (t.character() == ']' && (flags & FLAG_BRACK_END))
			return;

		else if (t.cat() == catOther)
			add(*cell, t.character());

		//
		// control sequences
		//
		else if (t.cs() == "protect")
			// ignore \\protect, will be re-added during output
			;

		else if (t.cs() == "end")
			break;

		else if (t.cs() == ")")
			break;

		else if (t.cs() == "]")
			break;

		else if (t.cs() == "\\") {
			grid.vcrskip(LyXLength(getArg('[', ']')), cellrow);
			++cellrow;
			cellcol = 0;
			if (cellrow == grid.nrows())
				grid.addRow(cellrow - 1);
			if (grid.asHullInset())
				grid.asHullInset()->numbered(cellrow, numbered);
			cell = &grid.cell(grid.index(cellrow, cellcol));
		}

		else if (t.cs() == "limits")
			limits = 1;

		else if (t.cs() == "nolimits")
			limits = -1;

		else if (t.cs() == "nonumber") {
			if (grid.asHullInset())
				grid.asHullInset()->numbered(cellrow, false);
		}

		else if (t.cs() == "number") {
			if (grid.asHullInset())
				grid.asHullInset()->numbered(cellrow, true);
		}

		else if (t.cs() == "hline") {
			if (grid.asHullInset())
				grid.asHullInset()->rowinfo(cellrow + 1);
		}

		else if (t.cs() == "sqrt") {
			char c = getChar();
			if (c == '[') {
				cell->push_back(MathAtom(new MathRootInset));
				parse_into(cell->back()->cell(0), FLAG_BRACK_END);
				parse_into(cell->back()->cell(1), FLAG_ITEM);
			} else {
				putback();
				cell->push_back(MathAtom(new MathSqrtInset));
				parse_into(cell->back()->cell(0), FLAG_ITEM);
			}
		}

		else if (t.cs() == "left") {
			string l = getToken().asString();
			MathArray ar;
			parse_into(ar, FLAG_RIGHT);
			string r = getToken().asString();
			MathAtom dl(new MathDelimInset(l, r));
			dl->cell(0) = ar;
			cell->push_back(dl);
		}

		else if (t.cs() == "right") {
			if (!(flags & FLAG_RIGHT)) {
				//lyxerr << "got so far: '" << cell << "'\n";
				error("Unmatched right delimiter");
			}
			return;
		}

		else if (t.cs() == "begin") {
			string const name = getArg('{', '}');
			if (name == "array" || name == "subarray") {
				string const valign = getArg('[', ']') + 'c';
				string const halign = getArg('{', '}');
				cell->push_back(MathAtom(new MathArrayInset(name, valign[0], halign)));
				parse_into2(cell->back(), FLAG_END, false);
			} else if (name == "split" || name == "cases" ||
					 name == "gathered" || name == "aligned") {
				cell->push_back(createMathInset(name));
				parse_into2(cell->back(), FLAG_END, false);
			} else if (name == "matrix"  || name == "pmatrix" || name == "bmatrix" ||
					 name == "vmatrix" || name == "Vmatrix") {
				cell->push_back(createMathInset(name));
				parse_into2(cell->back(), FLAG_END, false);
			} else
				lyxerr << "unknow math inset begin '" << name << "'\n";
		}

		else if (t.cs() == "kern") {
#ifdef WITH_WARNINGS
#warning A hack...
#endif
			string s;
			while (1) {
				Token const & t = getToken();
				if (!good()) {
					putback();
					break;
				}
				s += t.character();
				if (isValidLength(s))
					break;
			}
			cell->push_back(MathAtom(new MathKernInset(s)));
		}

		else if (t.cs() == "label") {
			if (grid.asHullInset())
				grid.asHullInset()->label(cellrow, getArg('{', '}'));
		}

		else if (t.cs() == "choose" || t.cs() == "over" || t.cs() == "atop") {
			MathAtom p = createMathInset(t.cs());
			cell->swap(p->cell(0));
			parse_into(p->cell(1), flags);
			cell->push_back(p);
			return;
		}

		else if (t.cs() == "substack") {
			cell->push_back(createMathInset(t.cs()));
			parse_into2(cell->back(), FLAG_ITEM, false);
		}

		else if (t.cs() == "xymatrix") {
			cell->push_back(createMathInset(t.cs()));
			parse_into2(cell->back(), FLAG_ITEM, false);
		}

#if 0
		// Disabled
		else if (1 && t.cs() == "ar") {
			MathXYArrowInset * p = new MathXYArrowInset;

			// try to read target
			char c = getChar();
			if (c == '[') {
				parse_into(p->cell(0), FLAG_BRACK_END);
				//lyxerr << "read target: " << p->cell(0) << "\n";
			} else {
				putback();
			}

			// try to read label
			if (nextToken().cat() == catSuper || nextToken().cat() == catSub) {
				p->up_ = nextToken().cat() == catSuper;
				getToken();
				parse_into(p->cell(1), FLAG_ITEM);
				//lyxerr << "read label: " << p->cell(1) << "\n";
			}

			cell->push_back(MathAtom(p));
			//lyxerr << "read cell: " << cell << "\n";
		}
#endif

#if 0
		else if (t.cs() == "mbox" || t.cs() == "text") {
			//array.push_back(createMathInset(t.cs()));
			array.push_back(MathAtom(new MathBoxInset(t.cs())));
			// slurp in the argument of mbox

			MathBoxInset * p = cell->back()->asBoxInset();
			//lyx::assert(p);
		}
#endif


		else if (t.cs().size()) {
			latexkeys const * l = in_word_set(t.cs());
			if (l) {
				if (l->inset == "font") {
					lyxerr << "starting font " << t.cs() << "\n";
					//CatCode catSpaceSave = theCatcode[' '];
					//if (l->id == LM_TC_TEXTRM) {
					//	// temporarily change catcode
					//	theCatcode[' '] = catLetter;
					//}

					MathAtom p = createMathInset(t.cs());
					parse_into(p->cell(0), FLAG_ITEM);
					cell->push_back(p);

					// undo catcode changes
					//theCatcode[' '] = catSpaceSave;
					//lyxerr << "ending font\n";
				}

				else if (l->inset == "oldfont") {
					MathAtom p = createMathInset(t.cs());
					parse_into(p->cell(0), flags);
					cell->push_back(p);
					return;
				}

				else if (l->inset == "box") {
					MathAtom p = createMathInset(t.cs());
					parse_into(p->cell(0), FLAG_ITEM | FLAG_BOX);
					cell->push_back(p);
				}

				else if (l->inset == "style") {
					MathAtom p = createMathInset(t.cs());
					parse_into(p->cell(0), flags);
					cell->push_back(p);
					return;
				}

				else {
					MathAtom p = createMathInset(t.cs());
					for (MathInset::idx_type i = 0; i < p->nargs(); ++i)
						parse_into(p->cell(i), FLAG_ITEM);
					cell->push_back(p);
				}
			}

			else {
				MathAtom p = createMathInset(t.cs());
				for (MathInset::idx_type i = 0; i < p->nargs(); ++i)
					parse_into(p->cell(i), FLAG_ITEM);
				cell->push_back(p);
			}
		}


		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}

	if (panic) {
		lyxerr << " Math Panic, expect problems!\n";
		//   Search for the end command.
		Token t;
		do {
			t = getToken();
		} while (good() && t.cs() != "end");
	}
}



} // anonymous namespace


void mathed_parse_cell(MathArray & ar, string const & str)
{
	istringstream is(str.c_str());
	mathed_parse_cell(ar, is);
}


void mathed_parse_cell(MathArray & ar, istream & is)
{
	Parser(is).parse_into(ar, 0);
}



bool mathed_parse_macro(string & name, string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	return parser.parse_macro(name);
}

bool mathed_parse_macro(string & name, istream & is)
{
	Parser parser(is);
	return parser.parse_macro(name);
}

bool mathed_parse_macro(string & name, LyXLex & lex)
{
	Parser parser(lex);
	return parser.parse_macro(name);
}



bool mathed_parse_normal(MathAtom & t, string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	return parser.parse_normal(t);
}

bool mathed_parse_normal(MathAtom & t, istream & is)
{
	Parser parser(is);
	return parser.parse_normal(t);
}

bool mathed_parse_normal(MathAtom & t, LyXLex & lex)
{
	Parser parser(lex);
	return parser.parse_normal(t);
}
