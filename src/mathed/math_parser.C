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

#include <config.h>

#include <cctype>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_parser.h"
#include "array.h"
#include "math_inset.h"
#include "math_arrayinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_factory.h"
#include "math_funcinset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_matrixinset.h"
#include "math_rootinset.h"
#include "math_scopeinset.h"
#include "math_sqrtinset.h"
#include "math_scriptinset.h"
#include "math_sqrtinset.h"
#include "debug.h"
#include "support.h"
#include "lyxlex.h"
#include "support/lstrings.h"

using std::istream;
using std::ostream;
using std::ios;
using std::endl;


namespace {

bool stared(string const & s)
{
	unsigned n = s.size();
	return n && s[n - 1] == '*';
}

MathScriptInset * prevScriptInset(MathArray const & array)
{
	MathInset * p = array.back();
	return (p && p->isScriptInset()) ? static_cast<MathScriptInset *>(p) : 0;
}


MathInset * lastScriptInset(MathArray & array, bool up, int limits)
{
	MathScriptInset * p = prevScriptInset(array);
	if (!p) {
		MathInset * b = array.back();
		if (b && b->isScriptable()) {
			p = new MathScriptInset(up, !up, b->clone());
			array.pop_back();	
		} else {
			p = new MathScriptInset(up, !up);
		}
		array.push_back(p);
	}
	if (up)
		p->up(true);
	else
		p->down(true);
	if (limits)
		p->limits(limits);
	return p;
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
	FLAG_BRACE      = 1 << 0,  //  an opening brace needed
	FLAG_BRACE_LAST = 1 << 1,  //  last closing brace ends the parsing process
	FLAG_RIGHT      = 1 << 2,  //  next \\right ends the parsing process
	FLAG_END        = 1 << 3,  //  next \\end ends the parsing process
	FLAG_BRACK_END  = 1 << 4,  //  next closing bracket ends the parsing process
	FLAG_NEWLINE    = 1 << 6,  //  next \\\\ ends the parsing process
	FLAG_ITEM       = 1 << 7,  //  read a (possibly braced token)
	FLAG_BLOCK      = 1 << 8,  //  next block ends the parsing process
	FLAG_LEAVE      = 1 << 9   //  leave the loop at the end
};


void catInit()
{
	for (int i = 0; i <= 255; ++i) 
		theCatcode[i] = catOther;
	for (int i = 'a'; i <= 'z'; ++i) 
		theCatcode[i] = catLetter;
	for (int i = 'A'; i <= 'Z'; ++i) 
		theCatcode[i] = catLetter;

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
	Token(const string & cs) : cs_(cs), char_(0), cat_(catIgnore) {}

	///
	string const & cs() const { return cs_; }
	///
	CatCode cat() const { return cat_; }
	///
	char character() const { return char_; }
	///
	string asString() const;

private:	
	///
	string cs_;
	///
	char char_;
	///
	CatCode cat_;
};

string Token::asString() const
{
	return cs_.size() ? cs_ : string(1, char_);
}

bool operator==(Token const & s, Token const & t)
{
	return s.character() == t.character()
		&& s.cat() == t.cat() && s.cs() == t.cs(); 
}

bool operator!=(Token const & s, Token const & t)
{
	return !(s == t);
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
	MathMacroTemplate * parse_macro();
	///
	MathMatrixInset * parse_normal();
	///
	void parse_into(MathArray & array, unsigned flags);
	///
	int lineno() const { return lineno_; }
	///
	void putback();

private:
	///
	string getArg(char lf, char rf);
	///
	char getChar();
	///
	void error(string const & msg);
	///
	void parse_lines(MathGridInset * p, bool numbered, bool outmost);
	///
	latexkeys const * read_delim();

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
	return good() ? tokens_[pos_++] : dummy;
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

	istringstream is(buffer, ios::in | ios::binary);

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

#if 0
	lyxerr << "\nTokens: ";
	for (unsigned i = 0; i < tokens_.size(); ++i)
		lyxerr << tokens_[i];
	lyxerr << "\n";
#endif
}


void Parser::error(string const & msg) 
{
	lyxerr << "Line ~" << lineno_ << ": Math parse error: " << msg << endl;
}


void Parser::parse_lines(MathGridInset * p, bool numbered, bool outmost)
{
	const int cols = p->ncols();

	// save global variables
	bool   const saved_num   = curr_num_;
	string const saved_label = curr_label_;

	for (int row = 0; true; ++row) {
		// reset global variables
		curr_num_   = numbered;
		curr_label_.erase();

		// reading a row
		for (int col = 0; col < cols; ++col) {
			//lyxerr << "reading cell " << row << " " << col << "\n";
			parse_into(p->cell(col + row * cols), FLAG_BLOCK);

			// no ampersand
			if (prevToken().cat() != catAlign) {
				//lyxerr << "less cells read than normal in row/col: "
				//	<< row << " " << col << "\n";
				break;
			}
		}

		if (outmost) {
			MathMatrixInset * m = static_cast<MathMatrixInset *>(p);
			m->numbered(row, curr_num_);
			m->label(row, curr_label_);
			if (curr_skip_.size()) {
				m->vskip(LyXLength(curr_skip_), row);
				curr_skip_.erase();
			}
		}

		// no newline?
		if (prevToken() != Token("\\")) {
			//lyxerr << "no newline here\n";
			break;
		}

		p->appendRow();
	}

	// restore "global" variables
	curr_num_   = saved_num;
	curr_label_ = saved_label;
}


MathMacroTemplate * Parser::parse_macro()
{
	while (nextToken().cat() == catSpace)
		getToken();

	if (getToken().cs() != "newcommand") {
		lyxerr << "\\newcommand expected\n";
		return 0;
	}

	if (getToken().cat() != catBegin) {
		lyxerr << "'{' expected\n";
		return 0;
	}

	string name = getToken().cs();

	if (getToken().cat() != catEnd) {
		lyxerr << "'}' expected\n";
		return 0;
	}

	string arg  = getArg('[', ']');
	int    narg = arg.empty() ? 0 : atoi(arg.c_str()); 
	//lyxerr << "creating macro " << name << " with " << narg <<  "args\n";
	MathMacroTemplate * p = new MathMacroTemplate(name, narg);
	parse_into(p->cell(0), FLAG_BRACE | FLAG_BRACE_LAST);
	return p;
}


MathMatrixInset * Parser::parse_normal()
{
	Token const & t = getToken();

	if (t.cat() == catMath || t.cs() == "(") {
		MathMatrixInset * p = new MathMatrixInset(LM_OT_SIMPLE);
		parse_into(p->cell(0), 0);
		return p;
	}

	if (!t.cs().size()) {
		lyxerr << "start of math expected, got '" << t << "'\n";
		return 0;
	}

	string const & cs = t.cs();

	if (cs == "[") {
		curr_num_ = 0;
		curr_label_.erase();
		MathMatrixInset * p = new MathMatrixInset(LM_OT_EQUATION);
		parse_into(p->cell(0), 0);
		p->numbered(0, curr_num_);
		p->label(0, curr_label_);
		return p;
	}

	if (cs != "begin") {
		lyxerr << "'begin' of un-simple math expected, got '" << cs << "'\n";
		return 0;
	}

	string const name = getArg('{', '}');

	if (name == "equation" || name == "equation*") {
		curr_num_ = !stared(name);
		curr_label_.erase();
		MathMatrixInset * p = new MathMatrixInset(LM_OT_EQUATION);
		parse_into(p->cell(0), FLAG_END);
		p->numbered(0, curr_num_);
		p->label(0, curr_label_);
		return p;
	}

	if (name == "eqnarray" || name == "eqnarray*") {
		MathMatrixInset * p = new MathMatrixInset(LM_OT_EQNARRAY);
		parse_lines(p, !stared(name), true);
		return p;
	}

	if (name == "align" || name == "align*") {
		MathMatrixInset * p = new MathMatrixInset(LM_OT_ALIGN);
		p->halign(getArg('{', '}'));
		parse_lines(p, !stared(name), true);
		return p;
	}

	if (name == "alignat" || name == "alignat*") {
		MathMatrixInset * p = new MathMatrixInset(LM_OT_ALIGNAT);
		p->halign(getArg('{', '}'));
		parse_lines(p, !stared(name), true);
		return p;
	}

	lyxerr[Debug::MATHED] << "1: unknown math environment: " << name << "\n";
	return 0;
}


latexkeys const * Parser::read_delim()
{
	Token const & t = getToken();
	latexkeys const * l = in_word_set(t.asString());
	return l ? l : in_word_set(".");
}


void Parser::parse_into(MathArray & array, unsigned flags)
{
	MathTextCodes yyvarcode = LM_TC_MIN;

	bool panic  = false;
	int  limits = 0;

	while (good()) {
		Token const & t = getToken();
	
		//lyxerr << "t: " << t << " flags: " << flags << "'\n";
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

		if (flags & FLAG_BRACE) {
			if (t.cat() != catBegin) {
				error("Expected {. Maybe you forgot to enclose an argument in {}");
				panic = true;
				break;
			} else {
				flags &= ~FLAG_BRACE;
				continue;
			}
		}

		if (flags & FLAG_BLOCK) {
			if (t.cat() == catEnd || t.cat() == catAlign || t.cs() == "\\")
				return;
			if (t.cs() == "end") {
				getArg('{', '}');
				return;
			}
		}

		//
		// cat codes
		//
		if (t.cat() == catMath)
			break;

		else if (t.cat() == catLetter)
			array.push_back(new MathCharInset(t.character(), yyvarcode));

		else if (t.cat() == catSpace && yyvarcode == LM_TC_TEXTRM)
			array.push_back(new MathCharInset(' ', yyvarcode));

		else if (t.cat() == catParameter) {
			Token const & n	= getToken();
			MathMacroArgument * p = new MathMacroArgument(n.character() - '0');
			array.push_back(p);
		}

		else if (t.cat() == catBegin) {
			//lyxerr << " creating ScopeInset\n";
			array.push_back(new MathScopeInset);
			parse_into(array.back()->cell(0), FLAG_BRACE_LAST);
		}

		else if (t.cat() == catEnd) {
			if (!(flags & FLAG_BRACE_LAST))
				lyxerr << " ##### unexpected end of block\n";
			return;
		}
		
		else if (t.cat() == catAlign) {
			lyxerr << "found tab unexpectedly, array: '" << array << "'\n";
			return;
		}
		
		else if (t.cat() == catSuper)
			parse_into(lastScriptInset(array, true, limits)->cell(0), FLAG_ITEM);
		
		else if (t.cat() == catSub)
			parse_into(lastScriptInset(array, false, limits)->cell(1), FLAG_ITEM);
		
		else if (t.character() == ']' && (flags & FLAG_BRACK_END))
			return;

		else if (t.cat() == catOther)
			array.push_back(new MathCharInset(t.character(), yyvarcode));
		
		//
		// codesequences
		//	
		else if (t.cs() == "protect") 
			;

		else if (t.cs() == "end")
			break;

		else if (t.cs() == ")")
			break;

		else if (t.cs() == "]")
			break;

		else if (t.cs() == "\\") {
			curr_skip_ = getArg('[', ']');
			if (!(flags & FLAG_NEWLINE))
				lyxerr[Debug::MATHED]
					<< "found newline unexpectedly, array: '" << array << "'\n";
			return;
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
				array.push_back(new MathRootInset);
				parse_into(array.back()->cell(0), FLAG_BRACK_END);
				parse_into(array.back()->cell(1), FLAG_ITEM);
			} else {
				putback();
				array.push_back(new MathSqrtInset);
				parse_into(array.back()->cell(0), FLAG_ITEM);
			}
		}
		
		else if (t.cs() == "left") {
			latexkeys const * l = read_delim();
			MathArray ar;
			parse_into(ar, FLAG_RIGHT);
			latexkeys const * r = read_delim();
			MathDelimInset * dl = new MathDelimInset(l, r);
			dl->cell(0) = ar;
			array.push_back(dl);
		}
		
		else if (t.cs() == "right") {
			if (!(flags & FLAG_RIGHT))
				error("Unmatched right delimiter");
			return;
		}

/*		
		case LM_TK_STY:
		{
			lyxerr[Debug::MATHED] << "LM_TK_STY not implemented\n";
			//MathArray tmp = array;
			//MathSizeInset * p = new MathSizeInset(MathStyles(lval_->id));
			//array.push_back(p);
			//parse_into(p->cell(0), FLAG_BRACE_FONT);
			break; 
		}

		case LM_TK_UNDEF: 
			if (MathMacroTable::hasTemplate(sval_)) {
				MathMacro * m = MathMacroTable::cloneTemplate(sval_);
				for (int i = 0; i < m->nargs(); ++i) 
					parse_into(m->cell(i), FLAG_ITEM);
				array.push_back(m);
				m->metrics(LM_ST_TEXT);
			} else
				array.push_back(new MathFuncInset(sval_));
			break;

		else  LM_TK_SPECIAL:
			array.push_back(new MathCharInset(ival_, LM_TC_SPECIAL));
			break;
*/
		
		else if (t.cs() == "begin") {
			string const name = getArg('{', '}');	
			if (name == "array") {
				string const valign = getArg('[', ']') + 'c';
				string const halign = getArg('{', '}');
				MathArrayInset * m = new MathArrayInset(halign.size(), 1);
				m->valign(valign[0]);
				m->halign(halign);
				parse_lines(m, false, false);
				array.push_back(m);
			} else 
				lyxerr[Debug::MATHED] << "unknow math inset begin '" << name << "'\n";	
		}
	
		else if (t.cs() == "label") {
			MathArray ar;
			parse_into(ar, FLAG_ITEM);
			ostringstream os;
			ar.write(os, true);
			curr_label_ = os.str();
			// was: 
			//curr_label_ = getArg('{', '}');
		}

		else if (t.cs() == "choose" || t.cs() == "over" || t.cs() == "atop") {
			limits = 0;
			MathInset * p = createMathInset(t.cs());
			p->cell(0).swap(array);
			array.push_back(p);
			parse_into(p->cell(1), FLAG_BLOCK);
		}
	
		else if (t.cs().size()) {
			limits = 0;
			latexkeys const * l = in_word_set(t.cs());
			if (l) {
				if (l->token == LM_TK_FONT) {
					//lyxerr << "starting font\n";
					MathArray ar;
					parse_into(ar, FLAG_ITEM);
					for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it)
						(*it)->handleFont(static_cast<MathTextCodes>(l->id));
					array.push_back(ar);
					//lyxerr << "ending font\n";
				}

				else if (l->token == LM_TK_OLDFONT)
					yyvarcode = static_cast<MathTextCodes>(l->id);

				else {
					MathInset * p = createMathInset(t.cs());
					for (int i = 0; i < p->nargs(); ++i) 
						parse_into(p->cell(i), FLAG_ITEM);
					array.push_back(p);
				}
			}

			else {
				MathInset * p = createMathInset(t.cs());
				if (p) {
					for (int i = 0; i < p->nargs(); ++i)
						parse_into(p->cell(i), FLAG_ITEM);
					array.push_back(p);
				} else {
					error("Unrecognized token");
					//lyxerr[Debug::MATHED] << "[" << t << "]\n";
					lyxerr << t << "\n";
				}	
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



MathArray mathed_parse_cell(string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	MathArray ar;
	parser.parse_into(ar, 0);
	return ar;
}



MathMacroTemplate * mathed_parse_macro(string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	return parser.parse_macro();
}

MathMacroTemplate * mathed_parse_macro(istream & is)
{
	Parser parser(is);
	return parser.parse_macro();
}

MathMacroTemplate * mathed_parse_macro(LyXLex & lex)
{
	Parser parser(lex);
	return parser.parse_macro();
}



MathMatrixInset * mathed_parse_normal(string const & str)
{
	istringstream is(str.c_str());
	Parser parser(is);
	return parser.parse_normal();
}

MathMatrixInset * mathed_parse_normal(istream & is)
{
	Parser parser(is);
	return parser.parse_normal();
}

MathMatrixInset * mathed_parse_normal(LyXLex & lex)
{
	Parser parser(lex);
	return parser.parse_normal();
}
