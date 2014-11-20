/**
 * \file MathParser.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
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
1
&

  \end{array}\]

*/


#include <config.h>

#include "MathParser.h"

#include "InsetMathArray.h"
#include "InsetMathBig.h"
#include "InsetMathBrace.h"
#include "InsetMathCancelto.h"
#include "InsetMathChar.h"
#include "InsetMathColor.h"
#include "InsetMathComment.h"
#include "InsetMathDelim.h"
#include "InsetMathEnsureMath.h"
#include "InsetMathEnv.h"
#include "InsetMathFrac.h"
#include "InsetMathKern.h"
#include "MathMacro.h"
#include "InsetMathPar.h"
#include "InsetMathRef.h"
#include "InsetMathRoot.h"
#include "InsetMathScript.h"
#include "InsetMathSideset.h"
#include "InsetMathSpace.h"
#include "InsetMathSplit.h"
#include "InsetMathSqrt.h"
#include "InsetMathStackrel.h"
#include "InsetMathString.h"
#include "InsetMathTabular.h"
#include "MathMacroTemplate.h"
#include "MathFactory.h"
#include "MathMacroArgument.h"
#include "MathSupport.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Encoding.h"
#include "Lexer.h"

#include "support/debug.h"
#include "support/convert.h"
#include "support/docstream.h"

#include <sstream>

//#define FILEDEBUG

using namespace std;

namespace lyx {

namespace {

InsetMath::mode_type asMode(InsetMath::mode_type oldmode, docstring const & str)
{
	//lyxerr << "handling mode: '" << str << "'" << endl;
	if (str == "mathmode")
		return InsetMath::MATH_MODE;
	if (str == "textmode" || str == "forcetext")
		return InsetMath::TEXT_MODE;
	return oldmode;
}


bool stared(docstring const & s)
{
	size_t const n = s.size();
	return n && s[n - 1] == '*';
}


docstring const repl(docstring const & oldstr, char_type const c,
		     docstring const & macro, bool textmode = false)
{
	docstring newstr;
	size_t i;
	size_t j;

	for (i = 0, j = 0; i < oldstr.size(); ++i) {
		if (c == oldstr[i]) {
			newstr.append(oldstr, j, i - j);
			newstr.append(macro);
			j = i + 1;
			if (macro.size() > 2 && j < oldstr.size())
				newstr += (textmode && oldstr[j] == ' ' ? '\\' : ' ');
		}
	}

	// Any substitution?
	if (j == 0)
		return oldstr;

	newstr.append(oldstr, j, i - j);
	return newstr;
}


docstring escapeSpecialChars(docstring const & str, bool textmode)
{
	docstring const backslash = textmode ? from_ascii("\\textbackslash")
					     : from_ascii("\\backslash");
	docstring const caret = textmode ? from_ascii("\\textasciicircum")
					 : from_ascii("\\mathcircumflex");
	docstring const tilde = textmode ? from_ascii("\\textasciitilde")
					 : from_ascii("\\sim");

	return repl(repl(repl(repl(repl(repl(repl(repl(repl(repl(str,
			'\\', backslash, textmode),
			'^', caret, textmode),
			'~', tilde, textmode),
			'_', from_ascii("\\_")),
			'$', from_ascii("\\$")),
			'#', from_ascii("\\#")),
			'&', from_ascii("\\&")),
			'%', from_ascii("\\%")),
			'{', from_ascii("\\{")),
			'}', from_ascii("\\}"));
}


/*!
 * Add the row \p cellrow to \p grid.
 * \returns wether the row could be added. Adding a row can fail for
 * environments like "equation" that have a fixed number of rows.
 */
bool addRow(InsetMathGrid & grid, InsetMathGrid::row_type & cellrow,
	    docstring const & vskip, bool allow_newpage_ = true)
{
	++cellrow;
	if (cellrow == grid.nrows()) {
		//lyxerr << "adding row " << cellrow << endl;
		grid.addRow(cellrow - 1);
		if (cellrow == grid.nrows()) {
			// We can't add a row to this grid, so let's
			// append the content of this cell to the previous
			// one.
			// This does not happen in well formed .lyx files,
			// but LyX versions 1.3.x and older could create
			// such files and tex2lyx can still do that.
			--cellrow;
			lyxerr << "ignoring extra row";
			if (!vskip.empty())
				lyxerr << " with extra space " << to_utf8(vskip);
			if (!allow_newpage_)
				lyxerr << " with no page break allowed";
			lyxerr << '.' << endl;
			return false;
		}
	}
	grid.vcrskip(Length(to_utf8(vskip)), cellrow - 1);
	grid.rowinfo(cellrow - 1).allow_newpage_ = allow_newpage_;
	return true;
}


/*!
 * Add the column \p cellcol to \p grid.
 * \returns wether the column could be added. Adding a column can fail for
 * environments like "eqnarray" that have a fixed number of columns.
 */
bool addCol(InsetMathGrid & grid, InsetMathGrid::col_type & cellcol)
{
	++cellcol;
	if (cellcol == grid.ncols()) {
		//lyxerr << "adding column " << cellcol << endl;
		grid.addCol(cellcol);
		if (cellcol == grid.ncols()) {
			// We can't add a column to this grid, so let's
			// append the content of this cell to the previous
			// one.
			// This does not happen in well formed .lyx files,
			// but LyX versions 1.3.x and older could create
			// such files and tex2lyx can still do that.
			--cellcol;
			lyxerr << "ignoring extra column." << endl;
			return false;
		}
	}
	return true;
}


/*!
 * Check whether the last row is empty and remove it if yes.
 * Otherwise the following code
 * \verbatim
\begin{array}{|c|c|}
\hline
1 & 2 \\ \hline
3 & 4 \\ \hline
\end{array}
 * \endverbatim
 * will result in a grid with 3 rows (+ the dummy row that is always present),
 * because the last '\\' opens a new row.
 * Note that this is only needed for inner-hull grid types, such as array
 * or aligned, but not for outer-hull grid types, such as eqnarray or align.
 */
void delEmptyLastRow(InsetMathGrid & grid)
{
	InsetMathGrid::row_type const row = grid.nrows() - 1;
	for (InsetMathGrid::col_type col = 0; col < grid.ncols(); ++col) {
		if (!grid.cell(grid.index(row, col)).empty())
			return;
	}
	// Copy the row information of the empty row (which would contain the
	// last hline in the example above) to the dummy row and delete the
	// empty row.
	grid.rowinfo(row + 1) = grid.rowinfo(row);
	grid.delRow(row);
}


/*!
 * Tell whether the environment name corresponds to an inner-hull grid type.
 */
bool innerHull(docstring const & name)
{
	// For [bB]matrix, [vV]matrix, and pmatrix we can check the suffix only
	return name == "array" || name == "cases" || name == "aligned"
		|| name == "alignedat" || name == "gathered" || name == "split"
		|| name == "subarray" || name == "tabular" || name == "matrix"
		|| name == "smallmatrix" || name.substr(1) == "matrix";
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

CatCode theCatcode[128];


inline CatCode catcode(char_type c)
{
	/* The only characters that are not catOther lie in the pure ASCII
	 * range. Therefore theCatcode has only 128 entries.
	 * TeX itself deals with 8bit characters, so if needed this table
	 * could be enlarged to 256 entries.
	 * Any larger value does not make sense, since the fact that we use
	 * unicode internally does not change Knuth's TeX engine.
	 * Apart from that a table for the full 21bit UCS4 range would waste
	 * too much memory. */
	if (c >= 128)
		return catOther;

	return theCatcode[c];
}


enum {
	FLAG_ALIGN      = 1 << 0,  //  next & or \\ ends the parsing process
	FLAG_BRACE_LAST = 1 << 1,  //  next closing brace ends the parsing
	FLAG_RIGHT      = 1 << 2,  //  next \\right ends the parsing process
	FLAG_END        = 1 << 3,  //  next \\end ends the parsing process
	FLAG_BRACK_LAST = 1 << 4,  //  next closing bracket ends the parsing
	FLAG_TEXTMODE   = 1 << 5,  //  we are in a box
	FLAG_ITEM       = 1 << 6,  //  read a (possibly braced) token
	FLAG_LEAVE      = 1 << 7,  //  leave the loop at the end
	FLAG_SIMPLE     = 1 << 8,  //  next $ leaves the loop
	FLAG_EQUATION   = 1 << 9,  //  next \] leaves the loop
	FLAG_SIMPLE2    = 1 << 10, //  next \) leaves the loop
	FLAG_OPTION     = 1 << 11, //  read [...] style option
	FLAG_BRACED     = 1 << 12  //  read {...} style argument
};


//
// Helper class for parsing
//

class Token {
public:
	///
	Token() : cs_(), char_(0), cat_(catIgnore) {}
	///
	Token(char_type c, CatCode cat) : cs_(), char_(c), cat_(cat) {}
	///
	explicit Token(docstring const & cs) : cs_(cs), char_(0), cat_(catIgnore) {}

	///
	docstring const & cs() const { return cs_; }
	///
	CatCode cat() const { return cat_; }
	///
	char_type character() const { return char_; }
	///
	docstring asString() const { return !cs_.empty() ? cs_ : docstring(1, char_); }
	///
	docstring asInput() const { return !cs_.empty() ? '\\' + cs_ : docstring(1, char_); }

private:
	///
	docstring cs_;
	///
	char_type char_;
	///
	CatCode cat_;
};


ostream & operator<<(ostream & os, Token const & t)
{
	if (!t.cs().empty()) {
		docstring const & cs = t.cs();
		// FIXME: For some strange reason, the stream operator instanciate
		// a new Token before outputting the contents of t.cs().
		// Because of this the line
		//     os << '\\' << cs;
		// below becomes recursive.
		// In order to avoid that we return early:
		if (cs == "\\")
			return os;
		os << '\\' << to_utf8(cs);
	}
	else if (t.cat() == catLetter)
		os << t.character();
	else
		os << '[' << t.character() << ',' << t.cat() << ']';
	return os;
}


class Parser {
public:
	///
	typedef  InsetMath::mode_type mode_type;
	///
	typedef  Parse::flags parse_mode;

	///
	Parser(Lexer & lex, parse_mode mode, Buffer * buf);
	/// Only use this for reading from .lyx file format, for the reason
	/// see Parser::tokenize(istream &).
	Parser(istream & is, parse_mode mode, Buffer * buf);
	///
	Parser(docstring const & str, parse_mode mode, Buffer * buf);

	///
	bool parse(MathAtom & at);
	///
	bool parse(MathData & array, unsigned flags, mode_type mode);
	///
	bool parse1(InsetMathGrid & grid, unsigned flags, mode_type mode,
		bool numbered);
	///
	MathData parse(unsigned flags, mode_type mode);
	///
	int lineno() const { return lineno_; }
	///
	void putback();
	/// store current position
	void pushPosition();
	/// restore previous position
	void popPosition();
	/// forget last saved position
	void dropPosition();

private:
	///
	void parse2(MathAtom & at, unsigned flags, mode_type mode, bool numbered);
	/// get arg delimited by 'left' and 'right'
	docstring getArg(char_type left, char_type right);
	///
	char_type getChar();
	///
	void error(string const & msg);
	void error(docstring const & msg) { error(to_utf8(msg)); }
	/// dump contents to screen
	void dump() const;
	/// Only use this for reading from .lyx file format (see
	/// implementation for reason)
	void tokenize(istream & is);
	///
	void tokenize(docstring const & s);
	///
	void skipSpaceTokens(idocstream & is, char_type c);
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
	void lex(docstring const & s);
	///
	bool good() const;
	///
	docstring parse_verbatim_item();
	///
	docstring parse_verbatim_option();

	///
	int lineno_;
	///
	vector<Token> tokens_;
	///
	unsigned pos_;
	///
	std::vector<unsigned> positions_;
	/// Stack of active environments
	vector<docstring> environments_;
	///
	parse_mode mode_;
	///
	bool success_;
	///
	Buffer * buffer_;
};


Parser::Parser(Lexer & lexer, parse_mode mode, Buffer * buf)
	: lineno_(lexer.lineNumber()), pos_(0), mode_(mode), success_(true),
	  buffer_(buf)
{
	tokenize(lexer.getStream());
	lexer.eatLine();
}


Parser::Parser(istream & is, parse_mode mode, Buffer * buf)
	: lineno_(0), pos_(0), mode_(mode), success_(true), buffer_(buf)
{
	tokenize(is);
}


Parser::Parser(docstring const & str, parse_mode mode, Buffer * buf)
	: lineno_(0), pos_(0), mode_(mode), success_(true), buffer_(buf)
{
	tokenize(str);
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
	//lyxerr << "looking at token " << tokens_[pos_] << " pos: " << pos_ << endl;
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


void Parser::pushPosition()
{
	positions_.push_back(pos_);
}


void Parser::popPosition()
{
	pos_ = positions_.back();
	positions_.pop_back();
}


void Parser::dropPosition()
{
	positions_.pop_back();
}


bool Parser::good() const
{
	return pos_ < tokens_.size();
}


char_type Parser::getChar()
{
	if (!good()) {
		error("The input stream is not well...");
		return 0;
	}
	return tokens_[pos_++].character();
}


docstring Parser::getArg(char_type left, char_type right)
{
	docstring result;
	skipSpaces();

	if (!good())
		return result;

	char_type c = getChar();

	if (c != left)
		putback();
	else
		while ((c = getChar()) != right && good())
			result += c;

	return result;
}


void Parser::skipSpaceTokens(idocstream & is, char_type c)
{
	// skip trailing spaces
	while (catcode(c) == catSpace || catcode(c) == catNewline)
		if (!is.get(c))
			break;
	//lyxerr << "putting back: " << c << endl;
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
	tokenize(from_utf8(s));
}


void Parser::tokenize(docstring const & buffer)
{
	idocstringstream is(mode_ & Parse::VERBATIM
			? escapeSpecialChars(buffer, mode_ & Parse::TEXTMODE)
			: buffer, ios::in | ios::binary);

	char_type c;
	while (is.get(c)) {
		//lyxerr << "reading c: " << c << endl;

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
					if (c == '\n')
						c = ' ';
					docstring s(1, c);
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
				if (!(mode_ & Parse::QUIET))
					lyxerr << "ignoring a char: " << int(c) << endl;
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
	lyxerr << " pos: " << pos_ << endl;
}


void Parser::error(string const & msg)
{
	success_ = false;
	if (!(mode_ & Parse::QUIET)) {
		lyxerr << "Line ~" << lineno_ << ": Math parse error: "
		       << msg << endl;
		dump();
	}
}


bool Parser::parse(MathAtom & at)
{
	skipSpaces();
	MathData ar(buffer_);
	parse(ar, false, InsetMath::UNDECIDED_MODE);
	if (ar.size() != 1 || ar.front()->getType() == hullNone) {
		if (!(mode_ & Parse::QUIET))
			lyxerr << "unusual contents found: " << ar << endl;
		at = MathAtom(new InsetMathPar(buffer_, ar));
		//if (at->nargs() > 0)
		//	at.nucleus()->cell(0) = ar;
		//else
		//	lyxerr << "unusual contents found: " << ar << endl;
		success_ = false;
	} else
		at = ar[0];
	return success_;
}


docstring Parser::parse_verbatim_option()
{
	skipSpaces();
	docstring res;
	if (nextToken().character() == '[') {
		Token t = getToken();
		for (Token t = getToken(); t.character() != ']' && good(); t = getToken()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + parse_verbatim_item() + '}';
			} else
				res += t.asInput();
		}
	}
	return res;
}


docstring Parser::parse_verbatim_item()
{
	skipSpaces();
	docstring res;
	if (nextToken().cat() == catBegin) {
		Token t = getToken();
		for (Token t = getToken(); t.cat() != catEnd && good(); t = getToken()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + parse_verbatim_item() + '}';
			}
			else
				res += t.asInput();
		}
	}
	return res;
}


MathData Parser::parse(unsigned flags, mode_type mode)
{
	MathData ar(buffer_);
	parse(ar, flags, mode);
	return ar;
}


bool Parser::parse(MathData & array, unsigned flags, mode_type mode)
{
	InsetMathGrid grid(buffer_, 1, 1);
	parse1(grid, flags, mode, false);
	array = grid.cell(0);
	return success_;
}


void Parser::parse2(MathAtom & at, const unsigned flags, const mode_type mode,
	const bool numbered)
{
	parse1(*(at.nucleus()->asGridInset()), flags, mode, numbered);
}


bool Parser::parse1(InsetMathGrid & grid, unsigned flags,
	const mode_type mode, const bool numbered)
{
	int limits = 0;
	InsetMathGrid::row_type cellrow = 0;
	InsetMathGrid::col_type cellcol = 0;
	MathData * cell = &grid.cell(grid.index(cellrow, cellcol));
	Buffer * buf = buffer_;

	if (grid.asHullInset())
		grid.asHullInset()->numbered(cellrow, numbered);

	//dump();
	//lyxerr << " flags: " << flags << endl;
	//lyxerr << " mode: " << mode  << endl;
	//lyxerr << "grid: " << grid << endl;

	while (good()) {
		Token const & t = getToken();

#ifdef FILEDEBUG
		lyxerr << "t: " << t << " flags: " << flags << endl;
		lyxerr << "mode: " << mode  << endl;
		cell->dump();
		lyxerr << endl;
#endif

		if (flags & FLAG_ITEM) {

			if (t.cat() == catBegin) {
				// skip the brace and collect everything to the next matching
				// closing brace
				parse1(grid, FLAG_BRACE_LAST, mode, numbered);
				return success_;
			}

			// handle only this single token, leave the loop if done
			flags = FLAG_LEAVE;
		}


		if (flags & FLAG_BRACED) {
			if (t.cat() == catSpace)
				continue;

			if (t.cat() != catBegin) {
				error("opening brace expected");
				return success_;
			}

			// skip the brace and collect everything to the next matching
			// closing brace
			flags = FLAG_BRACE_LAST;
		}


		if (flags & FLAG_OPTION) {
			if (t.cat() == catOther && t.character() == '[') {
				MathData ar;
				parse(ar, FLAG_BRACK_LAST, mode);
				cell->append(ar);
			} else {
				// no option found, put back token and we are done
				putback();
			}
			return success_;
		}

		//
		// cat codes
		//
		if (t.cat() == catMath) {
			if (mode != InsetMath::MATH_MODE) {
				// we are inside some text mode thingy, so opening new math is allowed
				Token const & n = getToken();
				if (n.cat() == catMath) {
					// TeX's $$...$$ syntax for displayed math
					if (mode == InsetMath::UNDECIDED_MODE) {
						cell->push_back(MathAtom(new InsetMathHull(buf, hullEquation)));
						parse2(cell->back(), FLAG_SIMPLE, InsetMath::MATH_MODE, false);
						getToken(); // skip the second '$' token
					} else {
						// This is not an outer hull and display math is
						// not allowed inside text mode environments.
						error("bad math environment $$");
						break;
					}
				} else {
					// simple $...$  stuff
					putback();
					if (mode == InsetMath::UNDECIDED_MODE) {
						cell->push_back(MathAtom(new InsetMathHull(buf, hullSimple)));
						parse2(cell->back(), FLAG_SIMPLE, InsetMath::MATH_MODE, false);
					} else {
						// Don't create nested math hulls (bug #5392)
						cell->push_back(MathAtom(new InsetMathEnsureMath(buf)));
						parse(cell->back().nucleus()->cell(0), FLAG_SIMPLE, InsetMath::MATH_MODE);
					}
				}
			}

			else if (flags & FLAG_SIMPLE) {
				// this is the end of the formula
				return success_;
			}

			else {
				Token const & n = getToken();
				if (n.cat() == catMath) {
					error("something strange in the parser");
					break;
				} else {
					// This is inline math ($...$), but the parser thinks we are
					// already in math mode and latex would issue an error, unless we
					// are inside a text mode user macro. We have no way to tell, so
					// let's play safe by using \ensuremath, as it will work in any case.
					putback();
					cell->push_back(MathAtom(new InsetMathEnsureMath(buf)));
					parse(cell->back().nucleus()->cell(0), FLAG_SIMPLE, InsetMath::MATH_MODE);
				}
			}
		}

		else if (t.cat() == catLetter)
			cell->push_back(MathAtom(new InsetMathChar(t.character())));

		else if (t.cat() == catSpace && mode != InsetMath::MATH_MODE) {
			if (cell->empty() || cell->back()->getChar() != ' ')
				cell->push_back(MathAtom(new InsetMathChar(t.character())));
		}

		else if (t.cat() == catNewline && mode != InsetMath::MATH_MODE) {
			if (cell->empty() || cell->back()->getChar() != ' ')
				cell->push_back(MathAtom(new InsetMathChar(' ')));
		}

		else if (t.cat() == catParameter) {
			Token const & n	= getToken();
			cell->push_back(MathAtom(new MathMacroArgument(n.character()-'0')));
		}

		else if (t.cat() == catActive)
			cell->push_back(MathAtom(new InsetMathSpace(string(1, t.character()), "")));

		else if (t.cat() == catBegin) {
			MathData ar;
			parse(ar, FLAG_BRACE_LAST, mode);
			// do not create a BraceInset if they were written by LyX
			// this helps to keep the annoyance of  "a choose b"  to a minimum
			if (ar.size() == 1 && ar[0]->extraBraces())
				cell->append(ar);
			else
				cell->push_back(MathAtom(new InsetMathBrace(ar)));
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return success_;
			error("found '}' unexpectedly");
			//LASSERT(false, /**/);
			//add(cell, '}', LM_TC_TEX);
		}

		else if (t.cat() == catAlign) {
			//lyxerr << " column now " << (cellcol + 1)
			//       << " max: " << grid.ncols() << endl;
			if (flags & FLAG_ALIGN)
				return success_;
			if (addCol(grid, cellcol))
				cell = &grid.cell(grid.index(cellrow, cellcol));
		}

		else if (t.cat() == catSuper || t.cat() == catSub) {
			bool up = (t.cat() == catSuper);
			// we need no new script inset if the last thing was a scriptinset,
			// which has that script already not the same script already
			if (cell->empty())
				cell->push_back(MathAtom(new InsetMathScript(buf, up)));
			else if (cell->back()->asScriptInset() &&
					!cell->back()->asScriptInset()->has(up))
				cell->back().nucleus()->asScriptInset()->ensure(up);
			else if (cell->back()->asScriptInset())
				cell->push_back(MathAtom(new InsetMathScript(buf, up)));
			else
				cell->back() = MathAtom(new InsetMathScript(buf, cell->back(), up));
			InsetMathScript * p = cell->back().nucleus()->asScriptInset();
			// special handling of {}-bases
			// Here we could remove the brace inset for things
			// like {a'}^2 and add the braces back in
			// InsetMathScript::write().
			// We do not do it, since it is not possible to detect
			// reliably whether the braces are needed because the
			// nucleus contains more than one symbol, or whether
			// they are needed for unknown commands like \xx{a}_0
			// or \yy{a}{b}_0. This was done in revision 14819
			// in an unreliable way. See this thread
			// http://www.mail-archive.com/lyx-devel%40lists.lyx.org/msg104917.html
			// for more details.
			// However, we remove empty braces because they look
			// ugly on screen and we are sure that they were added
			// by the write() method (and will be re-added on save).
			if (p->nuc().size() == 1 &&
			    p->nuc().back()->asBraceInset() &&
			    p->nuc().back()->asBraceInset()->cell(0).empty())
				p->nuc().erase(0);

			parse(p->cell(p->idxOfScript(up)), FLAG_ITEM, mode);
			if (limits) {
				p->limits(limits);
				limits = 0;
			}
		}

		else if (t.character() == ']' && (flags & FLAG_BRACK_LAST)) {
			//lyxerr << "finished reading option" << endl;
			return success_;
		}

		else if (t.cat() == catOther) {
			char_type c = t.character();
			if (isAsciiOrMathAlpha(c)
			    || mode_ & Parse::VERBATIM
			    || !(mode_ & Parse::USETEXT)
			    || mode == InsetMath::TEXT_MODE) {
				cell->push_back(MathAtom(new InsetMathChar(c)));
			} else {
				MathAtom at = createInsetMath("text", buf);
				at.nucleus()->cell(0).push_back(MathAtom(new InsetMathChar(c)));
				while (nextToken().cat() == catOther
				       && !isAsciiOrMathAlpha(nextToken().character())) {
					c = getToken().character();
					at.nucleus()->cell(0).push_back(MathAtom(new InsetMathChar(c)));
				}
				cell->push_back(at);
			}
		}

		else if (t.cat() == catComment) {
			docstring s;
			while (good()) {
				Token const & t = getToken();
				if (t.cat() == catNewline)
					break;
				s += t.asInput();
			}
			cell->push_back(MathAtom(new InsetMathComment(buf, s)));
			skipSpaces();
		}

		//
		// control sequences
		//

		else if (t.cs() == "lyxlock") {
			if (!cell->empty())
				cell->back().nucleus()->lock(true);
		}

		else if ((t.cs() == "global" && nextToken().cs() == "def") ||
			 t.cs() == "def") {
			if (t.cs() == "global")
				getToken();
			
			// get name
			docstring name = getToken().cs();
			
			// read parameters
			int nargs = 0;
			docstring pars;
			while (good() && nextToken().cat() != catBegin) {
				pars += getToken().cs();
				++nargs;
			}
			nargs /= 2;
			
			// read definition
			MathData def;
			parse(def, FLAG_ITEM, InsetMath::UNDECIDED_MODE);
			
			// is a version for display attached?
			skipSpaces();
			MathData display;
			if (nextToken().cat() == catBegin)
				parse(display, FLAG_ITEM, InsetMath::MATH_MODE);
			
			cell->push_back(MathAtom(new MathMacroTemplate(buf,
				name, nargs, 0, MacroTypeDef,
				vector<MathData>(), def, display)));

			if (buf && (mode_ & Parse::TRACKMACRO))
				buf->usermacros.insert(name);
		}
		
		else if (t.cs() == "newcommand" ||
			 t.cs() == "renewcommand" ||
			 t.cs() == "newlyxcommand") {
			// get name
			if (getToken().cat() != catBegin) {
				error("'{' in \\newcommand expected (1) ");
				return success_;
			}
			docstring name = getToken().cs();
			if (getToken().cat() != catEnd) {
				error("'}' in \\newcommand expected");
				return success_;
			}
				
			// get arity
			docstring const arg = getArg('[', ']');
			int nargs = 0;
			if (!arg.empty())
				nargs = convert<int>(arg);
				
			// optional argument given?
			skipSpaces();
			int optionals = 0;
			vector<MathData> optionalValues;
			while (nextToken().character() == '[') {
				getToken();
				optionalValues.push_back(MathData());
				parse(optionalValues[optionals], FLAG_BRACK_LAST, mode);
				++optionals;
			}
			
			MathData def;
			parse(def, FLAG_ITEM, InsetMath::UNDECIDED_MODE);
			
			// is a version for display attached?
			skipSpaces();
			MathData display;
			if (nextToken().cat() == catBegin)
				parse(display, FLAG_ITEM, InsetMath::MATH_MODE);
			
			cell->push_back(MathAtom(new MathMacroTemplate(buf,
				name, nargs, optionals, MacroTypeNewcommand,
				optionalValues, def, display)));

			if (buf && (mode_ & Parse::TRACKMACRO))
				buf->usermacros.insert(name);
		}
		
		else if (t.cs() == "newcommandx" ||
			 t.cs() == "renewcommandx") {
			// \newcommandx{\foo}[2][usedefault, addprefix=\global,1=default]{#1,#2}
			// get name
			docstring name;
			if (nextToken().cat() == catBegin) {
				getToken();
				name = getToken().cs();
				if (getToken().cat() != catEnd) {
					error("'}' in \\newcommandx expected");
					return success_;
				}
			} else
				name = getToken().cs();
				
			// get arity
			docstring const arg = getArg('[', ']');
			if (arg.empty()) {
				error("[num] in \\newcommandx expected");
				return success_;
			}
			int nargs = convert<int>(arg);
			
			// get options
			int optionals = 0;
			vector<MathData> optionalValues;
			if (nextToken().character() == '[') {
				// skip '['
				getToken();
					
				// handle 'opt=value' options, separated by ','.
				skipSpaces();
				while (nextToken().character() != ']' && good()) {
					if (nextToken().character() >= '1'
					    && nextToken().character() <= '9') {
						// optional value -> get parameter number
						int n = getChar() - '0';
						if (n > nargs) {
							error("Arity of \\newcommandx too low "
							      "for given optional parameter.");
							return success_;
						}
						
						// skip '='
						if (getToken().character() != '=') {
							error("'=' and optional parameter value "
							      "expected for \\newcommandx");
							return success_;
						}
						
						// get value
						int optNum = max(size_t(n), optionalValues.size());
						optionalValues.resize(optNum);
						optionalValues[n - 1].clear();
						while (nextToken().character() != ']'
						       && nextToken().character() != ',') {
							MathData data;
							parse(data, FLAG_ITEM, InsetMath::UNDECIDED_MODE);
							optionalValues[n - 1].append(data);
						}
						optionals = max(n, optionals);
					} else if (nextToken().cat() == catLetter) {
						// we in fact ignore every non-optional
						// parameter
						
						// get option name
						docstring opt;
						while (nextToken().cat() == catLetter)
							opt += getChar();
					
						// value?
						skipSpaces();
						MathData value;
						if (nextToken().character() == '=') {
							getToken();
							while (nextToken().character() != ']'
								&& nextToken().character() != ',')
								parse(value, FLAG_ITEM, 
								      InsetMath::UNDECIDED_MODE);
						}
					} else {
						error("option for \\newcommandx expected");
						return success_;
					}
					
					// skip komma
					skipSpaces();
					if (nextToken().character() == ',') {
						getChar();
						skipSpaces();
					} else if (nextToken().character() != ']') {
						error("Expecting ',' or ']' in options "
						      "of \\newcommandx");
						return success_;
					}
				}
				
				// skip ']'
				if (!good())
					return success_;
				getToken();
			}

			// get definition
			MathData def;
			parse(def, FLAG_ITEM, InsetMath::UNDECIDED_MODE);

			// is a version for display attached?
			skipSpaces();
			MathData display;
			if (nextToken().cat() == catBegin)
				parse(display, FLAG_ITEM, InsetMath::MATH_MODE);

			cell->push_back(MathAtom(new MathMacroTemplate(buf,
				name, nargs, optionals, MacroTypeNewcommandx,
				optionalValues, def, display)));

			if (buf && (mode_ & Parse::TRACKMACRO))
				buf->usermacros.insert(name);
		}

		else if (t.cs() == "(") {
			if (mode == InsetMath::UNDECIDED_MODE) {
				cell->push_back(MathAtom(new InsetMathHull(buf, hullSimple)));
				parse2(cell->back(), FLAG_SIMPLE2, InsetMath::MATH_MODE, false);
			} else {
				// Don't create nested math hulls (bug #5392)
				cell->push_back(MathAtom(new InsetMathEnsureMath(buf)));
				parse(cell->back().nucleus()->cell(0), FLAG_SIMPLE2, InsetMath::MATH_MODE);
			}
		}

		else if (t.cs() == "[") {
			if (mode != InsetMath::UNDECIDED_MODE) {
				error("bad math environment [");
				break;
			}
			cell->push_back(MathAtom(new InsetMathHull(buf, hullEquation)));
			parse2(cell->back(), FLAG_EQUATION, InsetMath::MATH_MODE, false);
		}

		else if (t.cs() == "protect")
			// ignore \\protect, will hopefully be re-added during output
			;

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				docstring const name = getArg('{', '}');
				if (environments_.empty())
					error("'found \\end{" + name +
					      "}' without matching '\\begin{" +
					      name + "}'");
				else if (name != environments_.back())
					error("'\\end{" + name +
					      "}' does not match '\\begin{" +
					      environments_.back() + "}'");
				else {
					environments_.pop_back();
					// Delete empty last row in matrix
					// like insets.
					// If you abuse InsetMathGrid for
					// non-matrix like structures you
					// probably need to refine this test.
					// Right now we only have to test for
					// single line hull insets.
					if (grid.nrows() > 1 && innerHull(name))
						delEmptyLastRow(grid);
					return success_;
				}
			} else
				error("found 'end' unexpectedly");
		}

		else if (t.cs() == ")") {
			if (flags & FLAG_SIMPLE2)
				return success_;
			error("found '\\)' unexpectedly");
		}

		else if (t.cs() == "]") {
			if (flags & FLAG_EQUATION)
				return success_;
			error("found '\\]' unexpectedly");
		}

		else if (t.cs() == "\\") {
			if (flags & FLAG_ALIGN)
				return success_;
			bool added = false;
			if (nextToken().asInput() == "*") {
				getToken();
				added = addRow(grid, cellrow, docstring(), false);
			} else {
				// skip "{}" added in front of "[" (the
				// counterpart is in InsetMathGrid::eolString())
				// skip spaces because formula could come from tex2lyx
				bool skipBraces = false;
				pushPosition();
				skipSpaces();
				if (nextToken().cat() == catBegin) {
					getToken();
					skipSpaces();
					if (nextToken().cat() == catEnd) {
						getToken();
						skipSpaces();
						if (nextToken().asInput() == "[")
							skipBraces = true;
					}
				}
				if (skipBraces)
					dropPosition();
				else
					popPosition();
				if (good()) {
					docstring arg;
					if (!skipBraces)
						arg = getArg('[', ']');
					added = addRow(grid, cellrow, arg);
				} else
					error("missing token after \\\\");
			}
			if (added) {
				cellcol = 0;
				if (grid.asHullInset())
					grid.asHullInset()->numbered(
							cellrow, numbered);
				cell = &grid.cell(grid.index(cellrow,
							     cellcol));
			}
		}

#if 0
		else if (t.cs() == "multicolumn") {
			// extract column count and insert dummy cells
			MathData count;
			parse(count, FLAG_ITEM, mode);
			int cols = 1;
			if (!extractNumber(count, cols)) {
				success_ = false;
				lyxerr << " can't extract number of cells from " << count << endl;
			}
			// resize the table if necessary
			for (int i = 0; i < cols; ++i) {
				if (addCol(grid, cellcol)) {
					cell = &grid.cell(grid.index(
							cellrow, cellcol));
					// mark this as dummy
					grid.cellinfo(grid.index(
						cellrow, cellcol)).dummy_ = true;
				}
			}
			// the last cell is the real thing, not a dummy
			grid.cellinfo(grid.index(cellrow, cellcol)).dummy_ = false;

			// read special alignment
			MathData align;
			parse(align, FLAG_ITEM, mode);
			//grid.cellinfo(grid.index(cellrow, cellcol)).align_ = extractString(align);

			// parse the remaining contents into the "real" cell
			parse(*cell, FLAG_ITEM, mode);
		}
#endif

		else if (t.cs() == "limits" || t.cs() == "nolimits") {
			CatCode const cat = nextToken().cat();
			if (cat == catSuper || cat == catSub)
				limits = t.cs() == "limits" ? 1 : -1;
			else {
				MathAtom at = createInsetMath(t.cs(), buf);
				cell->push_back(at);
			}
		}

		// \notag is the same as \nonumber if amsmath is used
		else if ((t.cs() == "nonumber" || t.cs() == "notag") &&
		         grid.asHullInset())
			grid.asHullInset()->numbered(cellrow, false);

		else if (t.cs() == "number" && grid.asHullInset())
			grid.asHullInset()->numbered(cellrow, true);

		else if (t.cs() == "hline") {
			grid.rowinfo(cellrow).lines_ ++;
		}

		else if (t.cs() == "sqrt") {
			MathData ar;
			parse(ar, FLAG_OPTION, mode);
			if (!ar.empty()) {
				cell->push_back(MathAtom(new InsetMathRoot(buf)));
				cell->back().nucleus()->cell(0) = ar;
				parse(cell->back().nucleus()->cell(1), FLAG_ITEM, mode);
			} else {
				cell->push_back(MathAtom(new InsetMathSqrt(buf)));
				parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
			}
		}

		else if (t.cs() == "cancelto") {
			MathData ar;
			parse(ar, FLAG_ITEM, mode);
				cell->push_back(MathAtom(new InsetMathCancelto(buf)));
				cell->back().nucleus()->cell(1) = ar;
				parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
		}

		else if (t.cs() == "unit") {
			// Allowed formats \unit[val]{unit}
			MathData ar;
			parse(ar, FLAG_OPTION, mode);
			if (!ar.empty()) {
				cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::UNIT)));
				cell->back().nucleus()->cell(0) = ar;
				parse(cell->back().nucleus()->cell(1), FLAG_ITEM, mode);
			} else {
				cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::UNIT, 1)));
				parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
			}
		}

		else if (t.cs() == "unitfrac") {
			// Here allowed formats are \unitfrac[val]{num}{denom}
			MathData ar;
			parse(ar, FLAG_OPTION, mode);
			if (!ar.empty()) {
				cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::UNITFRAC, 3)));
				cell->back().nucleus()->cell(2) = ar;
			} else {
				cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::UNITFRAC)));
			}
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
			parse(cell->back().nucleus()->cell(1), FLAG_ITEM, mode);
		}

		else if (t.cs() == "cfrac") {
			// allowed formats are \cfrac[pos]{num}{denom}
			docstring const arg = getArg('[', ']');
			//lyxerr << "got so far: '" << arg << "'" << endl;				
				if (arg == "l")
					cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::CFRACLEFT)));
				else if (arg == "r")
					cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::CFRACRIGHT)));
				else if (arg.empty() || arg == "c")
					cell->push_back(MathAtom(new InsetMathFrac(buf, InsetMathFrac::CFRAC)));
				else {
					error("found invalid optional argument");
					break;
				}
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
			parse(cell->back().nucleus()->cell(1), FLAG_ITEM, mode);
		}

		else if (t.cs() == "sideset") {
			// Here allowed formats are \sideset{_{bl}^{tl}}{_{br}^{tr}}{operator}
			MathData ar[2];
			InsetMathScript * script[2] = {0, 0};
			for (int i = 0; i < 2; ++i) {
				parse(ar[i], FLAG_ITEM, mode);
				if (ar[i].size() == 1)
					script[i] = ar[i][0].nucleus()->asScriptInset();
			}
			bool const hasscript[2] = {script[0] ? true : false, script[1] ? true : false};
			cell->push_back(MathAtom(new InsetMathSideset(buf, hasscript[0], hasscript[1])));
			if (hasscript[0]) {
				if (script[0]->hasDown())
					cell->back().nucleus()->cell(1) = script[0]->down();
				if (script[0]->hasUp())
					cell->back().nucleus()->cell(2) = script[0]->up();
			} else
				cell->back().nucleus()->cell(1) = ar[0];
			if (hasscript[1]) {
				if (script[1]->hasDown())
					cell->back().nucleus()->cell(2 + hasscript[0]) = script[1]->down();
				if (script[1]->hasUp())
					cell->back().nucleus()->cell(3 + hasscript[0]) = script[1]->up();
			} else
				cell->back().nucleus()->cell(2 + hasscript[0]) = ar[1];
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
		}

		else if (t.cs() == "stackrel") {
			// Here allowed formats are \stackrel[subscript]{superscript}{operator}
			MathData ar;
			parse(ar, FLAG_OPTION, mode);
			cell->push_back(MathAtom(new InsetMathStackrel(buf, !ar.empty())));
			if (!ar.empty())
				cell->back().nucleus()->cell(2) = ar;
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
			parse(cell->back().nucleus()->cell(1), FLAG_ITEM, mode);
		}

		else if (t.cs() == "xrightarrow" || t.cs() == "xleftarrow") {
			cell->push_back(createInsetMath(t.cs(), buf));
			parse(cell->back().nucleus()->cell(1), FLAG_OPTION, mode);
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
		}

		else if (t.cs() == "xhookrightarrow" || t.cs() == "xhookleftarrow" ||
			     t.cs() == "xRightarrow" || t.cs() == "xLeftarrow" ||
				 t.cs() == "xleftrightarrow" || t.cs() == "xLeftrightarrow" ||
				 t.cs() == "xrightharpoondown" || t.cs() == "xrightharpoonup" ||
				 t.cs() == "xleftharpoondown" || t.cs() == "xleftharpoonup" ||
				 t.cs() == "xleftrightharpoons" || t.cs() == "xrightleftharpoons" ||
				 t.cs() == "xmapsto") {
			cell->push_back(createInsetMath(t.cs(), buf));
			parse(cell->back().nucleus()->cell(1), FLAG_OPTION, mode);
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
		}

		else if (t.cs() == "ref" || t.cs() == "eqref" || t.cs() == "prettyref"
			  || t.cs() == "pageref" || t.cs() == "vpageref" || t.cs() == "vref") {
			cell->push_back(MathAtom(new InsetMathRef(buf, t.cs())));
			docstring const opt = parse_verbatim_option();
			docstring const ref = parse_verbatim_item();
			if (!opt.empty()) {
				cell->back().nucleus()->cell(1).push_back(
					MathAtom(new InsetMathString(opt)));
			}
			cell->back().nucleus()->cell(0).push_back(
					MathAtom(new InsetMathString(ref)));
		}

		else if (t.cs() == "left") {
			skipSpaces();
			Token const & tl = getToken();
			// \| and \Vert are equivalent, and InsetMathDelim
			// can't handle \|
			// FIXME: fix this in InsetMathDelim itself!
			docstring const l = tl.cs() == "|" ? from_ascii("Vert") : tl.asString();
			MathData ar;
			parse(ar, FLAG_RIGHT, mode);
			if (!good())
				break;
			skipSpaces();
			Token const & tr = getToken();
			docstring const r = tr.cs() == "|" ? from_ascii("Vert") : tr.asString();
			cell->push_back(MathAtom(new InsetMathDelim(buf, l, r, ar)));
		}

		else if (t.cs() == "right") {
			if (flags & FLAG_RIGHT)
				return success_;
			//lyxerr << "got so far: '" << cell << "'" << endl;
			error("Unmatched right delimiter");
			return success_;
		}

		else if (t.cs() == "begin") {
			docstring const name = getArg('{', '}');
			
			if (name.empty()) {
				success_ = false;
				error("found invalid environment");
				return success_;
			}
			
			environments_.push_back(name);

			if (name == "array" || name == "subarray") {
				docstring const valign = parse_verbatim_option() + 'c';
				docstring const halign = parse_verbatim_item();
				cell->push_back(MathAtom(new InsetMathArray(buf, name,
					InsetMathGrid::guessColumns(halign), 1, (char)valign[0], halign)));
				parse2(cell->back(), FLAG_END, mode, false);
			}

			else if (name == "tabular") {
				docstring const valign = parse_verbatim_option() + 'c';
				docstring const halign = parse_verbatim_item();
				cell->push_back(MathAtom(new InsetMathTabular(buf, name,
					InsetMathGrid::guessColumns(halign), 1, (char)valign[0], halign)));
				parse2(cell->back(), FLAG_END, InsetMath::TEXT_MODE, false);
			}

			else if (name == "split" || name == "cases") {
				cell->push_back(createInsetMath(name, buf));
				parse2(cell->back(), FLAG_END, mode, false);
			}

			else if (name == "alignedat") {
				docstring const valign = parse_verbatim_option() + 'c';
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new InsetMathSplit(buf, name, (char)valign[0])));
				parse2(cell->back(), FLAG_END, mode, false);
			}

			else if (name == "math") {
				if (mode == InsetMath::UNDECIDED_MODE) {
					cell->push_back(MathAtom(new InsetMathHull(buf, hullSimple)));
					parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, false);
				} else {
					// Don't create nested math hulls (bug #5392)
					cell->push_back(MathAtom(new InsetMathEnsureMath(buf)));
					parse(cell->back().nucleus()->cell(0), FLAG_END, InsetMath::MATH_MODE);
				}
			}

			else if (name == "equation" || name == "equation*"
					|| name == "displaymath") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				cell->push_back(MathAtom(new InsetMathHull(buf, hullEquation)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, (name == "equation"));
			}

			else if (name == "eqnarray" || name == "eqnarray*") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				cell->push_back(MathAtom(new InsetMathHull(buf, hullEqnArray)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (name == "align" || name == "align*") {
				if (mode == InsetMath::UNDECIDED_MODE) {
					cell->push_back(MathAtom(new InsetMathHull(buf, hullAlign)));
					parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
				} else {
					cell->push_back(MathAtom(new InsetMathSplit(buf, name,
							'c', !stared(name))));
					parse2(cell->back(), FLAG_END, mode, !stared(name));
				}
			}

			else if (name == "flalign" || name == "flalign*") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				cell->push_back(MathAtom(new InsetMathHull(buf, hullFlAlign)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (name == "alignat" || name == "alignat*") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new InsetMathHull(buf, hullAlignAt)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (name == "xalignat" || name == "xalignat*") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new InsetMathHull(buf, hullXAlignAt)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (name == "xxalignat") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				// ignore this for a while
				getArg('{', '}');
				cell->push_back(MathAtom(new InsetMathHull(buf, hullXXAlignAt)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (name == "multline" || name == "multline*") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				cell->push_back(MathAtom(new InsetMathHull(buf, hullMultline)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (name == "gather" || name == "gather*") {
				if (mode != InsetMath::UNDECIDED_MODE) {
					error("bad math environment " + name);
					break;
				}
				cell->push_back(MathAtom(new InsetMathHull(buf, hullGather)));
				parse2(cell->back(), FLAG_END, InsetMath::MATH_MODE, !stared(name));
			}

			else if (latexkeys const * l = in_word_set(name)) {
				if (l->inset == "matrix") {
					cell->push_back(createInsetMath(name, buf));
					parse2(cell->back(), FLAG_END, mode, false);
				} else if (l->inset == "split") {
					docstring const valign = parse_verbatim_option() + 'c';
					cell->push_back(MathAtom(
						new InsetMathSplit(buf, name, (char)valign[0])));
					parse2(cell->back(), FLAG_END, mode, false);
				} else {
					success_ = false;
					if (!(mode_ & Parse::QUIET)) {
						dump();
						lyxerr << "found math environment `"
						       << to_utf8(name)
						       << "' in symbols file with unsupported inset `"
						       << to_utf8(l->inset)
						       << "'." << endl;
					}
					// create generic environment inset
					cell->push_back(MathAtom(new InsetMathEnv(buf, name)));
					parse(cell->back().nucleus()->cell(0), FLAG_END, mode);
				}
			}

			else {
				success_ = false;
				if (!(mode_ & Parse::QUIET)) {
					dump();
					lyxerr << "found unknown math environment '"
					       << to_utf8(name) << "'" << endl;
				}
				// create generic environment inset
				cell->push_back(MathAtom(new InsetMathEnv(buf, name)));
				parse(cell->back().nucleus()->cell(0), FLAG_END, mode);
			}
		}

		else if (t.cs() == "kern") {
			// FIXME: A hack...
			docstring s;
			int num_tokens = 0;
			while (true) {
				Token const & t = getToken();
				++num_tokens;
				if (!good()) {
					s.clear();
					while (num_tokens--)
						putback();
					break;
				}
				s += t.character();
				if (isValidLength(to_utf8(s)))
					break;
			}
			if (s.empty())
				cell->push_back(MathAtom(new InsetMathKern));
			else
				cell->push_back(MathAtom(new InsetMathKern(s)));
		}

		else if (t.cs() == "label") {
			// FIXME: This is swallowed in inline formulas
			docstring label = parse_verbatim_item();
			MathData ar;
			asArray(label, ar);
			if (grid.asHullInset()) {
				grid.asHullInset()->label(cellrow, label);
			} else {
				cell->push_back(createInsetMath(t.cs(), buf));
				cell->push_back(MathAtom(new InsetMathBrace(ar)));
			}
		}

		else if (t.cs() == "choose" || t.cs() == "over"
				|| t.cs() == "atop" || t.cs() == "brace"
				|| t.cs() == "brack") {
			MathAtom at = createInsetMath(t.cs(), buf);
			at.nucleus()->cell(0) = *cell;
			cell->clear();
			parse(at.nucleus()->cell(1), flags, mode);
			cell->push_back(at);
			return success_;
		}

		else if (t.cs() == "color") {
			docstring const color = parse_verbatim_item();
			cell->push_back(MathAtom(new InsetMathColor(buf, true, color)));
			parse(cell->back().nucleus()->cell(0), flags, mode);
			return success_;
		}

		else if (t.cs() == "textcolor") {
			docstring const color = parse_verbatim_item();
			cell->push_back(MathAtom(new InsetMathColor(buf, false, color)));
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, InsetMath::TEXT_MODE);
		}

		else if (t.cs() == "normalcolor") {
			cell->push_back(createInsetMath(t.cs(), buf));
			parse(cell->back().nucleus()->cell(0), flags, mode);
			return success_;
		}

		else if (t.cs() == "substack") {
			cell->push_back(createInsetMath(t.cs(), buf));
			parse2(cell->back(), FLAG_ITEM, mode, false);
			// Delete empty last row if present
			InsetMathGrid & subgrid =
				*(cell->back().nucleus()->asGridInset());
			if (subgrid.nrows() > 1)
				delEmptyLastRow(subgrid);
		}

		else if (t.cs() == "xymatrix") {
			odocstringstream os;
			while (good() && nextToken().cat() != catBegin)
				os << getToken().asInput();
			cell->push_back(createInsetMath(t.cs() + os.str(), buf));
			parse2(cell->back(), FLAG_ITEM, mode, false);
			// Delete empty last row if present
			InsetMathGrid & subgrid =
				*(cell->back().nucleus()->asGridInset());
			if (subgrid.nrows() > 1)
				delEmptyLastRow(subgrid);
		}

		else if (t.cs() == "Diagram") {
			odocstringstream os;
			while (good() && nextToken().cat() != catBegin)
				os << getToken().asInput();
			cell->push_back(createInsetMath(t.cs() + os.str(), buf));
			parse2(cell->back(), FLAG_ITEM, mode, false);
		}

		else if (t.cs() == "framebox" || t.cs() == "makebox") {
			cell->push_back(createInsetMath(t.cs(), buf));
			parse(cell->back().nucleus()->cell(0), FLAG_OPTION, InsetMath::TEXT_MODE);
			parse(cell->back().nucleus()->cell(1), FLAG_OPTION, InsetMath::TEXT_MODE);
			parse(cell->back().nucleus()->cell(2), FLAG_ITEM, InsetMath::TEXT_MODE);
		}

		else if (t.cs() == "tag") {
			if (nextToken().character() == '*') {
				getToken();
				cell->push_back(createInsetMath(t.cs() + '*', buf));
			} else
				cell->push_back(createInsetMath(t.cs(), buf));
			parse(cell->back().nucleus()->cell(0), FLAG_ITEM, InsetMath::TEXT_MODE);
		}

		else if (t.cs() == "hspace") {
			bool const prot =  nextToken().character() == '*';
			if (prot)
				getToken();
			docstring const name = t.cs();
			docstring const arg = parse_verbatim_item();
			Length length;
			if (prot && arg == "\\fill")
				cell->push_back(MathAtom(new InsetMathSpace("hspace*{\\fill}", "")));
			else if (isValidLength(to_utf8(arg), &length))
				cell->push_back(MathAtom(new InsetMathSpace(length, prot)));
			else {
				// Since the Length class cannot use length variables
				// we must not create an InsetMathSpace.
				cell->push_back(MathAtom(new MathMacro(buf, name)));
				MathData ar;
				mathed_parse_cell(ar, '{' + arg + '}', mode_);
				cell->append(ar);
			}
		}

		else if (t.cs() == "smash") {
			skipSpaces();
			if (nextToken().asInput() == "[") {
				// Since the phantom inset cannot handle optional arguments
				// other than b and t, we must not create an InsetMathPhantom
				// if opt is different from b and t (bug 8967).
				docstring const opt = parse_verbatim_option();
				if (opt == "t" || opt == "b") {
					cell->push_back(createInsetMath(t.cs() + opt, buf));
					parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
				} else {
					docstring const arg = parse_verbatim_item();
					cell->push_back(MathAtom(new MathMacro(buf, t.cs())));
					MathData ar;
					mathed_parse_cell(ar, '[' + opt + ']', mode_);
					cell->append(ar);
					ar = MathData();
					mathed_parse_cell(ar, '{' + arg + '}', mode_);
					cell->append(ar);
				}
			}
			else {
				cell->push_back(createInsetMath(t.cs(), buf));
				parse(cell->back().nucleus()->cell(0), FLAG_ITEM, mode);
			}
		}

#if 0
		else if (t.cs() == "infer") {
			MathData ar;
			parse(ar, FLAG_OPTION, mode);
			cell->push_back(createInsetMath(t.cs(), buf));
			parse2(cell->back(), FLAG_ITEM, mode, false);
		}

		// Disabled
		else if (1 && t.cs() == "ar") {
			auto_ptr<InsetMathXYArrow> p(new InsetMathXYArrow);
			// try to read target
			parse(p->cell(0), FLAG_OTPTION, mode);
			// try to read label
			if (nextToken().cat() == catSuper || nextToken().cat() == catSub) {
				p->up_ = nextToken().cat() == catSuper;
				getToken();
				parse(p->cell(1), FLAG_ITEM, mode);
				//lyxerr << "read label: " << p->cell(1) << endl;
			}

			cell->push_back(MathAtom(p.release()));
			//lyxerr << "read cell: " << cell << endl;
		}
#endif

		else if (t.cs() == "lyxmathsym") {
			skipSpaces();
			if (getToken().cat() != catBegin) {
				error("'{' expected in \\" + t.cs());
				return success_;
			}
			int count = 0;
			docstring cmd;
			CatCode cat = nextToken().cat();
			while (good() && (count || cat != catEnd)) {
				if (cat == catBegin)
					++count;
				else if (cat == catEnd)
					--count;
				cmd += getToken().asInput();
				cat = nextToken().cat();
			}
			if (getToken().cat() != catEnd) {
				error("'}' expected in \\" + t.cs());
				return success_;
			}
			bool termination;
			docstring rem;
			do {
				cmd = Encodings::fromLaTeXCommand(cmd,
					Encodings::MATH_CMD | Encodings::TEXT_CMD,
					termination, rem);
				for (size_t i = 0; i < cmd.size(); ++i)
					cell->push_back(MathAtom(new InsetMathChar(cmd[i])));
				if (!rem.empty()) {
					char_type c = rem[0];
					cell->push_back(MathAtom(new InsetMathChar(c)));
					cmd = rem.substr(1);
					rem.clear();
				} else
					cmd.clear();
			} while (!cmd.empty());
		}

		else if (!t.cs().empty()) {
			bool const no_mhchem =
				(t.cs() == "ce" || t.cs() == "cf")
				&& buf && buf->params().use_package("mhchem") ==
						BufferParams::package_off;

			bool const is_user_macro = no_mhchem ||
				(buf && (mode_ & Parse::TRACKMACRO
					 ? buf->usermacros.count(t.cs()) != 0
					 : buf->getMacro(t.cs(), false) != 0));

			latexkeys const * l = in_word_set(t.cs());
			if (l && !is_user_macro) {
				if (l->inset == "big") {
					skipSpaces();
					docstring const delim = getToken().asInput();
					if (InsetMathBig::isBigInsetDelim(delim))
						cell->push_back(MathAtom(
							new InsetMathBig(t.cs(), delim)));
					else {
						cell->push_back(createInsetMath(t.cs(), buf));
						putback();
					}
				}

				else if (l->inset == "font") {
					cell->push_back(createInsetMath(t.cs(), buf));
					parse(cell->back().nucleus()->cell(0),
						FLAG_ITEM, asMode(mode, l->extra));
				}

				else if (l->inset == "oldfont") {
					cell->push_back(createInsetMath(t.cs(), buf));
					parse(cell->back().nucleus()->cell(0),
						flags | FLAG_ALIGN, asMode(mode, l->extra));
					if (prevToken().cat() != catAlign &&
					    prevToken().cs() != "\\")
						return success_;
					putback();
				}

				else if (l->inset == "style") {
					cell->push_back(createInsetMath(t.cs(), buf));
					parse(cell->back().nucleus()->cell(0),
						flags | FLAG_ALIGN, mode);
					if (prevToken().cat() != catAlign &&
					    prevToken().cs() != "\\")
						return success_;
					putback();
				}

				else {
					MathAtom at = createInsetMath(t.cs(), buf);
					for (InsetMath::idx_type i = 0; i < at->nargs(); ++i)
						parse(at.nucleus()->cell(i),
							FLAG_ITEM, asMode(mode, l->extra));
					cell->push_back(at);
				}
			}

			else {
				bool is_unicode_symbol = false;
				if (mode == InsetMath::TEXT_MODE && !is_user_macro) {
					int num_tokens = 0;
					docstring cmd = prevToken().asInput();
					CatCode cat = nextToken().cat();
					if (cat == catBegin) {
						int count = 0;
						while (good() && (count || cat != catEnd)) {
							cat = nextToken().cat();
							cmd += getToken().asInput();
							++num_tokens;
							if (cat == catBegin)
								++count;
							else if (cat == catEnd)
								--count;
						}
					}
					bool is_combining;
					bool termination;
					char_type c = Encodings::fromLaTeXCommand(cmd,
						Encodings::MATH_CMD | Encodings::TEXT_CMD,
						is_combining, termination);
					if (is_combining) {
						if (cat == catLetter)
							cmd += '{';
						cmd += getToken().asInput();
						++num_tokens;
						if (cat == catLetter)
							cmd += '}';
						c = Encodings::fromLaTeXCommand(cmd,
							Encodings::MATH_CMD | Encodings::TEXT_CMD,
							is_combining, termination);
					}
					if (c) {
						if (termination) {
							if (nextToken().cat() == catBegin) {
								getToken();
								if (nextToken().cat() == catEnd) {
									getToken();
									num_tokens += 2;
								} else
									putback();
							} else {
								while (nextToken().cat() == catSpace) {
									getToken();
									++num_tokens;
								}
							}
						}
						is_unicode_symbol = true;
						cell->push_back(MathAtom(new InsetMathChar(c)));
					} else {
						while (num_tokens--)
							putback();
					}
				}
				if (!is_unicode_symbol) {
					MathAtom at = is_user_macro ?
						MathAtom(new MathMacro(buf, t.cs()))
						: createInsetMath(t.cs(), buf);
					InsetMath::mode_type m = mode;
					//if (m == InsetMath::UNDECIDED_MODE)
					//lyxerr << "default creation: m1: " << m << endl;
					if (at->currentMode() != InsetMath::UNDECIDED_MODE)
						m = at->currentMode();
					//lyxerr << "default creation: m2: " << m << endl;
					InsetMath::idx_type start = 0;
					// this fails on \bigg[...\bigg]
					//MathData opt;
					//parse(opt, FLAG_OPTION, InsetMath::VERBATIM_MODE);
					//if (!opt.empty()) {
					//	start = 1;
					//	at.nucleus()->cell(0) = opt;
					//}
					for (InsetMath::idx_type i = start; i < at->nargs(); ++i) {
						parse(at.nucleus()->cell(i), FLAG_ITEM, m);
						if (mode == InsetMath::MATH_MODE)
							skipSpaces();
					}
					cell->push_back(at);
				}
			}
		}


		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
	return success_;
}



} // anonymous namespace


bool mathed_parse_cell(MathData & ar, docstring const & str, Parse::flags f)
{
	return Parser(str, f, ar.buffer()).parse(ar, 0, f & Parse::TEXTMODE ?
				InsetMath::TEXT_MODE : InsetMath::MATH_MODE);
}


bool mathed_parse_cell(MathData & ar, istream & is, Parse::flags f)
{
	return Parser(is, f, ar.buffer()).parse(ar, 0, f & Parse::TEXTMODE ?
				InsetMath::TEXT_MODE : InsetMath::MATH_MODE);
}


bool mathed_parse_normal(Buffer * buf, MathAtom & t, docstring const & str,
			 Parse::flags f)
{
	return Parser(str, f, buf).parse(t);
}


bool mathed_parse_normal(Buffer * buf, MathAtom & t, Lexer & lex,
			 Parse::flags f)
{
	return Parser(lex, f, buf).parse(t);
}


bool mathed_parse_normal(InsetMathGrid & grid, docstring const & str,
			 Parse::flags f)
{
	return Parser(str, f, &grid.buffer()).parse1(grid, 0, f & Parse::TEXTMODE ?
			InsetMath::TEXT_MODE : InsetMath::MATH_MODE, false);
}


void initParser()
{
	fill(theCatcode, theCatcode + 128, catOther);
	fill(theCatcode + 'a', theCatcode + 'z' + 1, catLetter);
	fill(theCatcode + 'A', theCatcode + 'Z' + 1, catLetter);

	theCatcode[int('\\')] = catEscape;
	theCatcode[int('{')]  = catBegin;
	theCatcode[int('}')]  = catEnd;
	theCatcode[int('$')]  = catMath;
	theCatcode[int('&')]  = catAlign;
	theCatcode[int('\n')] = catNewline;
	theCatcode[int('#')]  = catParameter;
	theCatcode[int('^')]  = catSuper;
	theCatcode[int('_')]  = catSub;
	theCatcode[int(0x7f)] = catIgnore;
	theCatcode[int(' ')]  = catSpace;
	theCatcode[int('\t')] = catSpace;
	theCatcode[int('\r')] = catNewline;
	theCatcode[int('~')]  = catActive;
	theCatcode[int('%')]  = catComment;
}


} // namespace lyx
