/*
 *  File:        math_parser.C
 *  Purpose:     Parser for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Parse LaTeX2e math mode code.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
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
#include "math_casesinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_factory.h"
#include "math_funcinset.h"
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
#include "math_splitinset.h"
#include "math_sqrtinset.h"
#include "math_support.h"
#include "math_xyarrowinset.h"

#include "lyxlex.h"
#include "debug.h"

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


namespace {

bool stared(string const & s)
{
	string::size_type const n = s.size();
	return n && s[n - 1] == '*';
}


void add(MathArray & ar, char c, MathTextCodes code)
{
	ar.push_back(MathAtom(new MathCharInset(c, code)));
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
	FLAG_LEAVE      = 1 << 9   //  leave the loop at the end
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

// Angus' compiler says these are not needed
//bool operator==(Token const & s, Token const & t)
//{
//	return s.character() == t.character()
//		&& s.cat() == t.cat() && s.cs() == t.cs(); 
//}
//
//bool operator!=(Token const & s, Token const & t)
//{
//	return !(s == t);
//}

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
	void parse_into(MathArray & array, unsigned flags, MathTextCodes = LM_TC_MIN);
	///
	int lineno() const { return lineno_; }
	///
	void putback();

private:
	///
	void parse_into1(MathArray & array, unsigned flags, MathTextCodes);
	///
	string getArg(char lf, char rf);
	///
	char getChar();
	///
	void error(string const & msg);
	///
	bool parse_lines(MathAtom & t, bool numbered, bool outmost);
	/// parses {... & ... \\ ... & ... }
	bool parse_lines2(MathAtom & t);
	/// dump contents to screen
	void dump() const;

private:
	///
	void tokenize(istream & is);
	///
	void tokenize(string const & s);
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
	/// skips opening brace
	void skipBegin();
	/// skips closing brace
	void skipEnd();
	/// counts a sequence of hlines
	int readHLines();
	///
	void lex(string const & s);
	///
	bool good() const;

	///
	int lineno_;
	///
	std::vector<Token> tokens_;
	///
	unsigned pos_;
	///
	bool   curr_num_;
	///
	string curr_label_;
	///
	string curr_skip_;
};


Parser::Parser(LyXLex & lexer)
	: lineno_(lexer.getLineNo()), pos_(0), curr_num_(false)
{
	tokenize(lexer.getStream());
	lexer.eatLine();
}


Parser::Parser(istream & is)
	: lineno_(0), pos_(0), curr_num_(false)
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
	//lyxerr << "looking at token " << tokens_[pos_] << '\n';
	return good() ? tokens_[pos_++] : dummy;
}


void Parser::skipSpaces()
{
	while (nextToken().cat() == catSpace)
		getToken();
}


void Parser::skipBegin()
{
	if (nextToken().cat() == catBegin)
		getToken();
	else
		lyxerr << "'{' expected\n";
}


void Parser::skipEnd()
{
	if (nextToken().cat() == catEnd)
		getToken();
	else
		lyxerr << "'}' expected\n";
}


int Parser::readHLines()
{
	int num = 0;
	skipSpaces();
	while (nextToken().cs() == "hline") {
		getToken();
		++num;
		skipSpaces();
	}
	return num;
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
		lyxerr << "The input stream is not well..." << endl;
	return tokens_[pos_++].character();
}


string Parser::getArg(char lf, char rg)
{
	skipSpaces();

	string result;
	char c = getChar();

	if (c != lf)  
		putback();
	else 
		while ((c = getChar()) != rg && good())
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
				string s(1, c);
				if (catcode(c) == catLetter) {
					while (is.get(c) && catcode(c) == catLetter)
						s += c;
					if (catcode(c) == catSpace)
						while (is.get(c) && catcode(c) == catSpace)
							;
					is.putback(c);
				}	
				push_back(Token(s));
				break;
			}

			default:
				push_back(Token(c, catcode(c)));
		}
	}

	//dump();
}


void Parser::dump() const
{
	lyxerr << "\nTokens: ";
	for (unsigned i = 0; i < tokens_.size(); ++i)
		lyxerr << tokens_[i];
	lyxerr << "\n";
}


void Parser::error(string const & msg) 
{
	lyxerr << "Line ~" << lineno_ << ": Math parse error: " << msg << endl;
	dump();
	//exit(1);
}



bool Parser::parse_lines(MathAtom & t, bool numbered, bool outmost)
{	
	MathGridInset * p = t->asGridInset();
	if (!p) {
		dump();
		lyxerr << "error in Parser::parse_lines() 1\n";
		return false;
	}

	// save global variables
	bool   const saved_num   = curr_num_;
	string const saved_label = curr_label_;

	// read initial hlines
	p->rowinfo(0).lines_ = readHLines();

	for (int row = 0; true; ++row) {
		// reset global variables
		curr_num_   = numbered;
		curr_label_.erase();

		// reading a row
		for (MathInset::col_type col = 0; col < p->ncols(); ++col) {
			//lyxerr << "reading cell " << row << " " << col << "\n";
		
			MathArray & ar = p->cell(col + row * p->ncols());
			parse_into(ar, FLAG_BLOCK);
			// remove 'unnecessary' braces:
			if (ar.size() == 1 && ar.back()->asBraceInset())
				ar = ar.back()->asBraceInset()->cell(0);

			// break if cell is not followed by an ampersand
			if (nextToken().cat() != catAlign) {
				//lyxerr << "less cells read than normal in row/col: "
				//	<< row << " " << col << "\n";
				break;
			}
			
			// skip the ampersand
			getToken();
		}

		if (outmost) {
			MathHullInset * m = t->asHullInset();
			if (!m) {
				lyxerr << "error in Parser::parse_lines() 2\n";
				return false;
			}
			m->numbered(row, curr_num_);
			m->label(row, curr_label_);
			if (curr_skip_.size()) {
				m->vcrskip(LyXLength(curr_skip_), row);
				curr_skip_.erase();
			}
		}

		// is a \\ coming?
		if (nextToken().isCR()) {
			// skip the cr-token
			getToken();

			// try to read a length
			//get

			// read hlines for next row
			p->rowinfo(row + 1).lines_ = readHLines();
		}

		// we are finished if the next token is an 'end'
		if (nextToken().cs() == "end") {
			// skip the end-token
			getToken();
			getArg('{','}');

			// leave the 'read a line'-loop
			break;
		}

		// otherwise, we have to start a new row
		p->appendRow();
	}

	// restore "global" variables
	curr_num_   = saved_num;
	curr_label_ = saved_label;

	return true;
}


bool Parser::parse_lines2(MathAtom & t)
{	
	MathGridInset * p = t->asGridInset();
	if (!p) {
		lyxerr << "error in Parser::parse_lines() 1\n";
		return false;
	}

	skipBegin();

	for (int row = 0; true; ++row) {
		// reading a row
		for (MathInset::col_type col = 0; true; ++col) {
			//lyxerr << "reading cell " << row << " " << col << " " << p->ncols() << "\n";
		
			if (col >= p->ncols()) {
				//lyxerr << "adding col " << col << "\n";
				p->addCol(p->ncols());
			}

			parse_into(p->cell(col + row * p->ncols()), FLAG_BLOCK2);
			//lyxerr << "read cell: " << p->cell(col + row * p->ncols()) << "\n";

			// break if cell is not followed by an ampersand
			if (nextToken().cat() != catAlign) {
				//lyxerr << "less cells read than normal in row/col: " << row << " " << col << "\n";
				break;
			}
			
			// skip the ampersand
			getToken();
		}

		// is a \\ coming?
		if (nextToken().isCR()) {
			// skip the cr-token
			getToken();
		}

		// we are finished if the next token is an '}'
		if (nextToken().cat() == catEnd) {
			// skip the end-token
			getToken();
			// leave the 'read a line'-loop
			break;
		}

		// otherwise, we have to start a new row
		p->appendRow();
	}

	return true;
}



bool Parser::parse_macro(string & name)
{
	name = "{error}";
	skipSpaces();

	if (getToken().cs() != "newcommand") {
		lyxerr << "\\newcommand expected\n";
		return false;
	}

	if (getToken().cat() != catBegin) {
		lyxerr << "'{' in \\newcommand expected (1)\n";
		return false;
	}

	name = getToken().cs();

	if (getToken().cat() != catEnd) {
		lyxerr << "'}' expected\n";
		return false;
	}

	string    arg  = getArg('[', ']');
	int       narg = arg.empty() ? 0 : atoi(arg.c_str()); 

	if (getToken().cat() != catBegin) {
		lyxerr << "'{' in \\newcommand expected (2)\n";
		return false;
	}

	MathArray ar;
	parse_into(ar, FLAG_BRACE_LAST);

	// we cannot handle recursive stuff at all
	MathArray test;
	test.push_back(createMathInset(name));
	if (ar.contains(test)) {
		lyxerr << "we cannot handle recursive macros at all.\n";
		return false;
	}

	MathMacroTable::create(name, narg, ar);
	return true;
}


bool Parser::parse_normal(MathAtom & matrix)
{
	skipSpaces();
	Token const & t = getToken();

	if (t.cs() == "(") {
		matrix = MathAtom(new MathHullInset(LM_OT_SIMPLE));
		parse_into(matrix->cell(0), 0);
		return true;
	}

	if (t.cat() == catMath) {
		Token const & n = getToken();
		if (n.cat() == catMath) {
			// TeX's $$...$$ syntax for displayed math
			matrix = MathAtom(new MathHullInset(LM_OT_EQUATION));
			MathHullInset * p = matrix->asHullInset();
			parse_into(p->cell(0), 0);
			p->numbered(0, curr_num_);
			p->label(0, curr_label_);
		} else {
			// simple $...$  stuff
			putback();
			matrix = MathAtom(new MathHullInset(LM_OT_SIMPLE));
			parse_into(matrix->cell(0), 0);
		}
		return true;
	}

	if (!t.cs().size()) {
		lyxerr << "start of math expected, got '" << t << "'\n";
		return false;
	}

	string const & cs = t.cs();

	if (cs == "[") {
		curr_num_ = 0;
		curr_label_.erase();
		matrix = MathAtom(new MathHullInset(LM_OT_EQUATION));
		MathHullInset * p = matrix->asHullInset();
		parse_into(p->cell(0), 0);
		p->numbered(0, curr_num_);
		p->label(0, curr_label_);
		return true;
	}

	if (cs != "begin") {
		lyxerr << "'begin' of un-simple math expected, got '" << cs << "'\n";
		return false;
	}

	string const name = getArg('{', '}');

	if (name == "math") {
		matrix = MathAtom(new MathHullInset(LM_OT_SIMPLE));
		parse_into(matrix->cell(0), 0);
		return true;
	}

	if (name == "equation" || name == "equation*" || name == "displaymath") {
		curr_num_ = (name == "equation");
		curr_label_.erase();
		matrix = MathAtom(new MathHullInset(LM_OT_EQUATION));
		MathHullInset * p = matrix->asHullInset();
		parse_into(p->cell(0), FLAG_END);
		p->numbered(0, curr_num_);
		p->label(0, curr_label_);
		return true;
	}

	if (name == "eqnarray" || name == "eqnarray*") {
		matrix = MathAtom(new MathHullInset(LM_OT_EQNARRAY));
		return parse_lines(matrix, !stared(name), true);
	}

	if (name == "align" || name == "align*") {
		matrix = MathAtom(new MathHullInset(LM_OT_ALIGN));
		return parse_lines(matrix, !stared(name), true);
	}

	if (name == "alignat" || name == "alignat*") {
		int nc = 2 * atoi(getArg('{', '}').c_str());
		matrix = MathAtom(new MathHullInset(LM_OT_ALIGNAT, nc));
		return parse_lines(matrix, !stared(name), true);
	}

	if (name == "xalignat" || name == "xalignat*") {
		int nc = 2 * atoi(getArg('{', '}').c_str());
		matrix = MathAtom(new MathHullInset(LM_OT_XALIGNAT, nc));
		return parse_lines(matrix, !stared(name), true);
	}

	if (name == "xxalignat") {
		int nc = 2 * atoi(getArg('{', '}').c_str());
		matrix = MathAtom(new MathHullInset(LM_OT_XXALIGNAT, nc));
		return parse_lines(matrix, !stared(name), true);
	}

	if (name == "multline" || name == "multline*") {
		matrix = MathAtom(new MathHullInset(LM_OT_MULTLINE));
		return parse_lines(matrix, !stared(name), true);
	}

	if (name == "gather" || name == "gather*") {
		matrix = MathAtom(new MathHullInset(LM_OT_GATHER));
		return parse_lines(matrix, !stared(name), true);
	}

	lyxerr[Debug::MATHED] << "1: unknown math environment: " << name << "\n";
	lyxerr << "1: unknown math environment: " << name << "\n";
	return false;
}


void Parser::parse_into(MathArray & array, unsigned flags, MathTextCodes code)
{
	parse_into1(array, flags, code);
	// remove 'unnecessary' braces:
	if (array.size() == 1 && array.back()->asBraceInset())
		array = array.back()->asBraceInset()->cell(0);
}


void Parser::parse_into1(MathArray & array, unsigned flags, MathTextCodes code)
{
	bool panic  = false;
	int  limits = 0;

	while (good()) {
		Token const & t = getToken();
	
		//lyxerr << "t: " << t << " flags: " << flags << "\n";
		//array.dump(lyxerr);
		//lyxerr << "\n";

		if (flags & FLAG_ITEM) {
			flags &= ~FLAG_ITEM;
			if (t.cat() == catBegin) { 
				// skip the brace and collect everything to the next matching
				// closing brace
				flags |= FLAG_BRACE_LAST;
				continue;
			} else {
				// handle only this single token, leave the loop if done
				flags |= FLAG_LEAVE;
			}
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
			if (flags & FLAG_BOX) {
				// we are inside an mbox, so opening new math is allowed
				array.push_back(MathAtom(new MathHullInset(LM_OT_SIMPLE)));
				parse_into(array.back()->cell(0), 0);
			} else {
				// otherwise this is the end of the formula
				break;
			}
		}

		else if (t.cat() == catLetter)
			add(array, t.character(), code);

		else if (t.cat() == catSpace && code == LM_TC_TEXTRM)
			add(array, t.character(), code);

		else if (t.cat() == catParameter) {
			Token const & n	= getToken();
			array.push_back(MathAtom(new MathMacroArgument(n.character()-'0', code)));
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
       || (ar.size() == 0 && array.size() == 0))
			{
				array.push_back(ar);
			} else {
				array.push_back(MathAtom(new MathBraceInset));
				array.back()->cell(0).swap(ar);
			}
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			lyxerr << "found '}' unexpectedly, array: '" << array << "'\n";
			//lyxerr << "found '}' unexpectedly\n";
			add(array, '}', LM_TC_TEX);
		}
		
		else if (t.cat() == catAlign) {
			lyxerr << "found tab unexpectedly, array: '" << array << "'\n";
			//lyxerr << "found tab unexpectedly\n";
			add(array, '&', LM_TC_TEX);
		}
		
		else if (t.cat() == catSuper || t.cat() == catSub) {
			bool up = (t.cat() == catSuper);
			MathScriptInset * p = 0; 
			if (array.size()) 
				p = array.back()->asScriptInset();
			if (!p || p->has(up)) {
				array.push_back(MathAtom(new MathScriptInset(up)));
				p = array.back()->asScriptInset();
			}
			p->ensure(up);
			parse_into(p->cell(up), FLAG_ITEM);
			p->limits(limits);
			limits = 0;
		}

		else if (t.character() == ']' && (flags & FLAG_BRACK_END))
			return;

		else if (t.cat() == catOther)
			add(array, t.character(), code);
		
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
			curr_skip_ = getArg('[', ']');
			//lyxerr << "found newline unexpectedly, array: '" << array << "'\n";
			lyxerr << "found newline unexpectedly\n";
			array.push_back(createMathInset("\\"));
		}
	
		else if (t.cs() == "limits")
			limits = 1;
		
		else if (t.cs() == "nolimits")
			limits = -1;
		
		else if (t.cs() == "nonumber")
			curr_num_ = false;

		else if (t.cs() == "number")
			curr_num_ = true;

		else if (t.cs() == "sqrt") {
			char c = getChar();
			if (c == '[') {
				array.push_back(MathAtom(new MathRootInset));
				parse_into(array.back()->cell(0), FLAG_BRACK_END);
				parse_into(array.back()->cell(1), FLAG_ITEM);
			} else {
				putback();
				array.push_back(MathAtom(new MathSqrtInset));
				parse_into(array.back()->cell(0), FLAG_ITEM);
			}
		}
		
		else if (t.cs() == "left") {
			string l = getToken().asString();
			MathArray ar;
			parse_into(ar, FLAG_RIGHT);
			string r = getToken().asString();
			MathAtom dl(new MathDelimInset(l, r));
			dl->cell(0) = ar;
			array.push_back(dl);
		}
		
		else if (t.cs() == "right") {
			if (!(flags & FLAG_RIGHT)) {
				//lyxerr << "got so far: '" << array << "'\n";
				error("Unmatched right delimiter");
			}
			return;
		}

		else if (t.cs() == "begin") {
			string const name = getArg('{', '}');	
			if (name == "array") {
				string const valign = getArg('[', ']') + 'c';
				string const halign = getArg('{', '}');
				array.push_back(MathAtom(new MathArrayInset(valign[0], halign)));
				parse_lines(array.back(), false, false);
			} else if (name == "split") {
				array.push_back(MathAtom(new MathSplitInset(1)));
				parse_lines(array.back(), false, false);
			} else if (name == "cases") {
				array.push_back(MathAtom(new MathCasesInset));
				parse_lines(array.back(), false, false);
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
			array.push_back(MathAtom(new MathKernInset(s)));
		}

		else if (t.cs() == "label") {
			curr_label_ = getArg('{', '}');
		}

		else if (t.cs() == "choose" || t.cs() == "over" || t.cs() == "atop") {
			MathAtom p = createMathInset(t.cs());
			array.swap(p->cell(0));
			parse_into(p->cell(1), flags, code);
			array.push_back(p);
			return;
		}

		else if (t.cs() == "xymatrix") {
			array.push_back(createMathInset(t.cs()));
			parse_lines2(array.back());
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

			array.push_back(MathAtom(p));
			//lyxerr << "read array: " << array << "\n";
		}

		else if (t.cs() == "mbox") {
			array.push_back(createMathInset(t.cs()));
			// slurp in the argument of mbox
	
			MathBoxInset * p = array.back()->asBoxInset();
			//lyx::assert(p);
		}
#endif

	
		else if (t.cs().size()) {
			latexkeys const * l = in_word_set(t.cs());
			if (l) {
				if (l->token == LM_TK_FONT) {
					//lyxerr << "starting font\n";
					//CatCode catSpaceSave = theCatcode[' '];
					//if (l->id == LM_TC_TEXTRM) {
					//	// temporarily change catcode	
					//	theCatcode[' '] = catLetter;	
					//}

					MathArray ar;
					parse_into(ar, FLAG_ITEM, static_cast<MathTextCodes>(l->id));
					array.push_back(ar);

					// undo catcode changes
					////theCatcode[' '] = catSpaceSave;
					//lyxerr << "ending font\n";
				}

				else if (l->token == LM_TK_OLDFONT) {
					code = static_cast<MathTextCodes>(l->id);
				}

				else if (l->token == LM_TK_BOX) {
					MathAtom p = createMathInset(t.cs());
					parse_into(p->cell(0), FLAG_ITEM | FLAG_BOX, LM_TC_BOX);
					array.push_back(p);
				}

				else if (l->token == LM_TK_STY) {
					MathAtom p = createMathInset(t.cs());
					parse_into(p->cell(0), flags, code);
					array.push_back(p);
					return;
				}

				else {
					MathAtom p = createMathInset(t.cs());
					for (MathInset::idx_type i = 0; i < p->nargs(); ++i) 
						parse_into(p->cell(i), FLAG_ITEM);
					array.push_back(p);
				}
			}

			else {
				MathAtom p = createMathInset(t.cs());
				for (MathInset::idx_type i = 0; i < p->nargs(); ++i)
					parse_into(p->cell(i), FLAG_ITEM);
				array.push_back(p);
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
