/** The math parser
    \author André Pönitz (2001)
 */

/*

If someone desperately needs partial "structures" (such as a few
cells of an array inset or similar) (s)he could uses the
following hack as starting point to write some macros:

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
#include "math_envinset.h"
#include "math_extern.h"
#include "math_factory.h"
#include "math_kerninset.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_hullinset.h"
#include "math_parboxinset.h"
#include "math_parinset.h"
#include "math_rootinset.h"
#include "math_scriptinset.h"
#include "math_sizeinset.h"
#include "math_sqrtinset.h"
#include "math_stringinset.h"
#include "math_support.h"
#include "math_xyarrowinset.h"

//#include "insets/insetref.h"
#include "ref_inset.h"

#include "lyxlex.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/lstrings.h"

#include <cctype>
#include <algorithm>

using std::istream;
using std::ostream;
using std::ios;
using std::endl;
using std::fill;
using std::vector;
using std::atoi;


//#define FILEDEBUG


namespace {

MathInset::mode_type asMode(string const & str)
{
	if (str == "mathmode")
		return MathInset::MATH_MODE;
	if (str == "textmode" || str == "forcetext")
		return MathInset::TEXT_MODE;
	if (str == "verbatimmode")
		return MathInset::VERBATIM_MODE;
	return MathInset::UNDECIDED_MODE;
}


bool stared(string const & s)
{
	string::size_type const n = s.size();
	return n && s[n - 1] == '*';
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
	FLAG_OPTION     = 1 << 11  //  read [...] style option
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
		os << "\\" << t.cs();
	else
		os << "[" << t.character() << "," << t.cat() << "]";
	return os;
}


class Parser {

public:
	///
	typedef  MathInset::mode_type mode_type;

	///
	Parser(LyXLex & lex);
	///
	Parser(istream & is);

	///
	bool parse(MathAtom & at);
	///
	void parse(MathArray & array, unsigned flags, mode_type mode);
	///
	int lineno() const { return lineno_; }
	///
	void putback();

private:
	///
	void parse1(MathGridInset & grid, unsigned flags, mode_type mode,
		bool numbered);
	///
	void parse2(MathAtom & at, unsigned flags, mode_type mode, bool numbered);
	/// get arg delimited by 'left' and 'right'
	string getArg(char left, char right);
	///
	char getChar();
	///
	void error(string const & msg);
	/// dump contents to screen
	void dump() const;
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
	//lyxerr << "putting back: " << c << "\n";
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
		//lyxerr << "reading c: " << c << "\n";

		switch (catcode(c)) {
			case catNewline: {
				++lineno_;
				is.get(c);
				if (catcode(c) == catNewline)
					; //push_back(Token("par"));
				else {
					push_back(Token('\n', catNewline));
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
	lyxerr << " pos: " << pos_ << "\n";
}


void Parser::error(string const & msg)
{
	lyxerr << "Line ~" << lineno_ << ": Math parse error: " << msg << endl;
	dump();
	//exit(1);
}


bool Parser::parse(MathAtom & at)
{
	skipSpaces();
	MathArray ar;
	parse(ar, false, MathInset::UNDECIDED_MODE);
	if (ar.size() != 1 || ar.front()->getType() == "none") {
		lyxerr << "unusual contents found: " << ar << endl;
		at.reset(new MathParInset);
		if (at->nargs() > 0)
			at->cell(0) = ar;
		else
			lyxerr << "unusual contents found: " << ar << endl;
		return true;
	}
	at = ar[0];
	return true;
}


void Parser::parse(MathArray & array, unsigned flags, mode_type mode)
{
	MathGridInset grid(1, 1);
	parse1(grid, flags, mode, false);
	array = grid.cell(0);
}


void Parser::parse2(MathAtom & at, unsigned flags, mode_type mode,
	bool numbered)
{
	parse1(*(at->asGridInset()), flags, mode, numbered);
}


void Parser::parse1(MathGridInset & grid, unsigned flags,
	mode_type mode, bool numbered)
{
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


		if (flags & FLAG_OPTION) {
			if (t.cat() == catOther && t.character() == '[') {
				MathArray ar;
				parse(ar, FLAG_BRACK_LAST, mode);
				cell->append(ar);
			} else {
				// no option found, put back token and we are done
				putback();
			}
			return;
		}

		//
		// cat codes
		//
		if (t.cat() == catMath) {
			if (mode != MathInset::MATH_MODE) {
				// we are inside some text mode thingy, so opening new math is allowed
				Token const & n = getToken();
				if (n.cat() == catMath) {
					// TeX's $$...$$ syntax for displayed math
					cell->push_back(MathAtom(new MathHullInset("equation")));
					parse2(cell->back(), FLAG_SIMPLE, MathInset::MATH_MODE, false);
					getToken(); // skip the second '$' token
				} else {
					// simple $...$  stuff
					putback();
					cell->push_back(MathAtom(new MathHullInset("simple")));
					parse2(cell->back(), FLAG_SIMPLE, MathInset::MATH_MODE, false);
				}
			}

			else if (flags & FLAG_SIMPLE) {
				// this is the end of the formula
				return;
			}

			else {
				error("something strange in the parser\n");
				break;
			}
		}

		else if (t.cat() == catLetter)
			cell->push_back(MathAtom(new MathCharInset(t.character())));

		else if (t.cat() == catSpace && mode != MathInset::MATH_MODE)
			cell->push_back(MathAtom(new MathCharInset(t.character())));

		else if (t.cat() == catNewline && mode != MathInset::MATH_MODE)
			cell->push_back(MathAtom(new MathCharInset(t.character())));

		else if (t.cat() == catParameter) {
			Token const & n	= getToken();
			cell->push_back(MathAtom(new MathMacroArgument(n.character()-'0')));
		}

		else if (t.cat() == catActive)
			cell->push_back(MathAtom(new MathCharInset(t.character())));

		else if (t.cat() == catBegin) {
			MathArray ar;
			parse(ar, FLAG_BRACE_LAST, mode);
			// do not create a BraceInset if they were written by LyX
			// this helps to keep the annoyance of  "a choose b"  to a minimum
			if (ar.size() == 1 && ar[0]->extraBraces())
				cell->append(ar);
			else
				cell->push_back(MathAtom(new MathBraceInset(ar)));
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			error("found '}' unexpectedly");
			//lyx::Assert(0);
			//add(cell, '}', LM_TC_TEX);
		}

		else if (t.cat() == catAlign) {
			++cellcol;
			//lyxerr << " column now " << cellcol << " max: " << grid.ncols() << "\n";
			if (cellcol == grid.ncols()) {
				lyxerr << "adding column " << cellcol << "\n";
				grid.addCol(cellcol - 1);
			}
			cell = &grid.cell(grid.index(cellrow, cellcol));
		}

		else if (t.cat() == catSuper || t.cat() == catSub) {
			if (mode == MathInset::VERBATIM_MODE) 
				cell->push_back(MathAtom(new MathStringInset(t.asString())));
			else {
				bool up = (t.cat() == catSuper);
				// we need no new script inset if the last thing was a scriptinset,
				// which has that script already not the same script already
				if (!cell->size())
					cell->push_back(MathAtom(new MathScriptInset(up)));
				else if (cell->back()->asScriptInset() &&
						!cell->back()->asScriptInset()->has(up))
					cell->back()->asScriptInset()->ensure(up);
				else if (cell->back()->asScriptInset())
					cell->push_back(MathAtom(new MathScriptInset(up)));
				else
					cell->back() = MathAtom(new MathScriptInset(cell->back(), up));
				MathScriptInset * p = cell->back()->asScriptInset();
				parse(p->cell(up), FLAG_ITEM, mode);
				p->limits(limits);
				limits = 0;
				// special handling of {}-bases 
				// is this always correct?
				if (p->nuc().size() == 1 && p->nuc().back()->asNestInset() &&
						p->nuc().back()->extraBraces())
					p->nuc() = p->nuc().back()->asNestInset()->cell(0);
			}
		}

		else if (t.character() == ']' && (flags & FLAG_BRACK_LAST)) {
			lyxerr << "finished reading option\n";
			return;
		}

		else if (t.cat() == catOther)
			cell->push_back(MathAtom(new MathCharInset(t.character())));

		//
		// control sequences
		//

		else if (t.cs() == "lyxlock") {
			if (cell->size())
				cell->back()->lock(true);
		}

		else if (t.cs() == "def" || t.cs() == "newcommand") {
			string name;
			int nargs = 0;
			if (t.cs() == "def") {
				// get name
				name = getToken().cs();

				// read parameter
				string pars;
				while (good() && nextToken().cat() != catBegin) {
					pars += getToken().cs();
					++nargs;
				}
				nargs /= 2;
				//lyxerr << "read \\def parameter list '" << pars << "'\n";

			} else { // t.cs() == "newcommand"

				if (getToken().cat() != catBegin) {
					error("'{' in \\newcommand expected (1) \n");
					return;
				}

				name = getToken().cs();

				if (getToken().cat() != catEnd) {
					error("'}' in \\newcommand expected\n");
					return;
				}

				string arg  = getArg('[', ']');
				if (!arg.empty())
					nargs = atoi(arg.c_str());

			}

			MathArray ar1;
			parse(ar1, FLAG_ITEM, MathInset::UNDECIDED_MODE);

			// we cannot handle recursive stuff at all
			//MathArray test;
			//test.push_back(createMathInset(name));
			//if (ar1.contains(test)) {
			//	error("we cannot handle recursive macros at all.\n");
			//	return;
			//}

			// is a version for display attached?
			skipSpaces();
			MathArray ar2;
			if (nextToken().cat() == catBegin) {
				parse(ar2, FLAG_ITEM, MathInset::MATH_MODE);
			}

			cell->push_back(MathAtom(new MathMacroTemplate(name, nargs, ar1, ar2)));
		}

		else if (t.cs() == "(") {
			cell->push_back(MathAtom(new MathHullInset("simple")));
			parse2(cell->back(), FLAG_SIMPLE2, MathInset::MATH_MODE, false);
		}

		else if (t.cs() == "[") {
			cell->push_back(MathAtom(new MathHullInset("equation")));
			parse2(cell->back(), FLAG_EQUATION, MathInset::MATH_MODE, false);
		}

		else if (t.cs() == "protect")
			// ignore \\protect, will hopefully be re-added during output
			;

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				//string const name =
				getArg('{', '}');
				// FIXME: check that we ended the correct environment
				return;
			}
			error("found 'end' unexpectedly");
		}

		else if (t.cs() == ")") {
			if (flags & FLAG_SIMPLE2)
				return;
			error("found '\\)' unexpectedly");
		}

		else if (t.cs() == "]") {
			if (flags & FLAG_EQUATION)
				return;
			error("found '\\]' unexpectedly");
		}

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

#if 1
		else if (t.cs() == "multicolumn") {
			// extract column count and insert dummy cells
			MathArray count;
			parse(count, FLAG_ITEM, mode);
			int cols = 1;
			if (!extractNumber(count, cols)) {
				lyxerr << " can't extract number of cells from " << count << "\n";
			}
			// resize the table if necessary
			for (int i = 0; i < cols; ++i) {
				++cellcol;
				if (cellcol == grid.ncols()) {
					lyxerr << "adding column " << cellcol << "\n";
					grid.addCol(cellcol - 1);
				}
				cell = &grid.cell(grid.index(cellrow, cellcol));
				// mark this as dummy
				grid.cellinfo(grid.index(cellrow, cellcol)).dummy_ = true;
			}
			// the last cell is the real thng, not a dummy
			grid.cellinfo(grid.index(cellrow, cellcol)).dummy_ = false;

			// read special alignment
			MathArray align;
			parse(align, FLAG_ITEM, mode);
			//grid.cellinfo(grid.index(cellrow, cellcol)).align_ = extractString(align);

			// parse the remaining contents into the "real" cell
			parse(*cell, FLAG_ITEM, mode);
		}
#endif

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
			MathArray ar;
			parse(ar, FLAG_OPTION, mode);
			if (ar.size()) {
				cell->push_back(MathAtom(new MathRootInset));
				cell->back()->cell(0) = ar;
				parse(cell->back()->cell(1), FLAG_ITEM, mode);
			} else {
				cell->push_back(MathAtom(new MathSqrtInset));
				parse(cell->back()->cell(0), FLAG_ITEM, mode);
			}
		}

		else if (t.cs() == "ref") {
			cell->push_back(MathAtom(new RefInset));
			parse(cell->back()->cell(1), FLAG_OPTION, mode);
			parse(cell->back()->cell(0), FLAG_ITEM, mode);
		}

		else if (t.cs() == "left") {
			string l = getToken().asString();
			MathArray ar;
			parse(ar, FLAG_RIGHT, mode);
			string r = getToken().asString();
			cell->push_back(MathAtom(new MathDelimInset(l, r, ar)));
		}

		else if (t.cs() == "right") {
			if (flags & FLAG_RIGHT)
				return;
			//lyxerr << "got so far: '" << cell << "'\n";
			error("Unmatched right delimiter");
			return;
		}

		else if (t.cs() == "begin") {
			string const name = getArg('{', '}');
			if (name == "array" || name == "subarray") {
				string const valign = getArg('[', ']') + 'c';
				string const halign = getArg('{', '}');
				cell->push_back(MathAtom(new MathArrayInset(name, valign[0], halign)));
				parse2(cell->back(), FLAG_END, mode, false);
			}

			else if (name == "split" || name == "cases" ||
					 name == "gathered" || name == "aligned") {
				cell->push_back(createMathInset(name));
				parse2(cell->back(), FLAG_END, mode, false);
			}

			else if (name == "math") {
				cell->push_back(MathAtom(new MathHullInset("simple")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, true);
			}

			else if (name == "equation" || name == "equation*"
					|| name == "displaymath") {
				cell->push_back(MathAtom(new MathHullInset("equation")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, (name == "equation"));
			}

			else if (name == "eqnarray" || name == "eqnarray*") {
				cell->push_back(MathAtom(new MathHullInset("eqnarray")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (name == "align" || name == "align*") {
				cell->push_back(MathAtom(new MathHullInset("align")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (name == "alignat" || name == "alignat*") {
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new MathHullInset("alignat")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (name == "xalignat" || name == "xalignat*") {
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new MathHullInset("xalignat")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (name == "xxalignat") {
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new MathHullInset("xxalignat")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (name == "multline" || name == "multline*") {
				cell->push_back(MathAtom(new MathHullInset("multline")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (name == "gather" || name == "gather*") {
				cell->push_back(MathAtom(new MathHullInset("gather")));
				parse2(cell->back(), FLAG_END, MathInset::MATH_MODE, !stared(name));
			}

			else if (latexkeys const * l = in_word_set(name)) {
				if (l->inset == "matrix") {
					cell->push_back(createMathInset(name));
					parse2(cell->back(), FLAG_END, mode, false);
				}
			}

			else {
				// lyxerr << "unknow math inset begin '" << name << "'\n";
				// create generic environment inset
				cell->push_back(MathAtom(new MathEnvInset(name)));
				parse(cell->back()->cell(0), FLAG_END, mode);
			}
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
			MathArray ar;
			parse(ar, FLAG_ITEM, MathInset::VERBATIM_MODE);
			if (grid.asHullInset()) {
				grid.asHullInset()->label(cellrow, asString(ar));
			} else {
				cell->push_back(createMathInset(t.cs()));
				cell->push_back(MathAtom(new MathBraceInset(ar)));
			}
		}

		else if (t.cs() == "choose" || t.cs() == "over" || t.cs() == "atop") {
			MathAtom p = createMathInset(t.cs());
			p->cell(0) = *cell;
			cell->clear();
			parse(p->cell(1), flags, mode);
			cell->push_back(p);
			return;
		}

		else if (t.cs() == "substack") {
			cell->push_back(createMathInset(t.cs()));
			parse2(cell->back(), FLAG_ITEM, mode, false);
		}

		else if (t.cs() == "xymatrix") {
			cell->push_back(createMathInset(t.cs()));
			parse2(cell->back(), FLAG_ITEM, mode, false);
		}

#if 0
		// Disabled
		else if (1 && t.cs() == "ar") {
			MathXYArrowInset * p = new MathXYArrowInset;
			// try to read target
			parse(p->cell(0), FLAG_OTPTION, mode);
			// try to read label
			if (nextToken().cat() == catSuper || nextToken().cat() == catSub) {
				p->up_ = nextToken().cat() == catSuper;
				getToken();
				parse(p->cell(1), FLAG_ITEM, mode);
				//lyxerr << "read label: " << p->cell(1) << "\n";
			}

			cell->push_back(MathAtom(p));
			//lyxerr << "read cell: " << cell << "\n";
		}
#endif

		else if (t.cs().size()) {
			latexkeys const * l = in_word_set(t.cs());
			if (l) {
				if (l->inset == "font") {
					cell->push_back(createMathInset(t.cs()));
					parse(cell->back()->cell(0), FLAG_ITEM, asMode(l->extra));
				}

				else if (l->inset == "oldfont") {
					cell->push_back(createMathInset(t.cs()));
					parse(cell->back()->cell(0), flags, asMode(l->extra));
					return;
				}

				else if (l->inset == "style") {
					cell->push_back(createMathInset(t.cs()));
					parse(cell->back()->cell(0), flags, mode);
					return;
				}

				else if (l->inset == "parbox") {
					// read optional positioning and width
					MathArray pos, width;
					parse(pos, FLAG_OPTION, MathInset::VERBATIM_MODE);
					parse(width, FLAG_ITEM, MathInset::VERBATIM_MODE);
					cell->push_back(createMathInset(t.cs()));
					parse(cell->back()->cell(0), FLAG_ITEM, MathInset::TEXT_MODE);
					cell->back()->asParboxInset()->setPosition(asString(pos));
					cell->back()->asParboxInset()->setWidth(asString(width));
				}

				else {
					MathAtom p = createMathInset(t.cs());
					for (MathInset::idx_type i = 0; i < p->nargs(); ++i)
						parse(p->cell(i), FLAG_ITEM, asMode(l->extra));
					cell->push_back(p);
				}
			}

			else {
				MathAtom p = createMathInset(t.cs());
				MathInset::mode_type m = mode;
				if (m == MathInset::UNDECIDED_MODE)
					m = p->currentMode();
				for (MathInset::idx_type i = 0; i < p->nargs(); ++i)
					parse(p->cell(i), FLAG_ITEM, m);
				cell->push_back(p);
			}
		}


		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
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
	Parser(is).parse(ar, 0, MathInset::MATH_MODE);
}


bool mathed_parse_normal(MathAtom & t, string const & str)
{
	istringstream is(str.c_str());
	return Parser(is).parse(t);
}


bool mathed_parse_normal(MathAtom & t, istream & is)
{
	return Parser(is).parse(t);
}


bool mathed_parse_normal(MathAtom & t, LyXLex & lex)
{
	return Parser(lex).parse(t);
}
