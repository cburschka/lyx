// -*- C++ -*-
/**
 * \file Parser.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <utility>


namespace lyx {


enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE, MATHTEXT_MODE, TABLE_MODE};

mode_type asMode(mode_type oldmode, std::string const & str);


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


CatCode catcode(unsigned char c);


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
	FLAG_BRACED     = 1 << 12, //  read {...} style argument
	FLAG_CELL       = 1 << 13, //  read table cell
	FLAG_TABBING    = 1 << 14  //  We are inside a tabbing environment
};



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
	Token(std::string const & cs, CatCode cat) : cs_(cs), char_(0), cat_(cat) {}

	///
	std::string const & cs() const { return cs_; }
	/// Returns the catcode of the token
	CatCode cat() const { return cat_; }
	///
	char character() const { return char_; }
	/// Returns the token as string
	std::string asString() const;
	/// Returns the token verbatim
	std::string asInput() const;

private:
	///
	std::string cs_;
	///
	char char_;
	///
	CatCode cat_;
};

std::ostream & operator<<(std::ostream & os, Token const & t);


/*!
 * Actual parser class
 *
 * The parser parses every character of the inputstream into a token
 * and classifies the token.
 * The following transformations are done:
 * - Consecutive spaces are combined into one single token with CatCode catSpace
 * - Consecutive newlines are combined into one single token with CatCode catNewline
 * - Comments and %\n combinations are parsed into one token with CatCode catComment
 */

class Parser {

public:
	///
	Parser(std::istream & is);
	///
	Parser(std::string const & s);
	///
	~Parser();

	///
	int lineno() const { return lineno_; }
	///
	void putback();
	/// dump contents to screen
	void dump() const;

	///
	typedef std::pair<bool, std::string> Arg;
	/*!
	 * Get an argument enclosed by \p left and \p right.
	 * \returns wether an argument was found in \p Arg.first and the
	 * argument in \p Arg.second. \see getArg().
	 */
	Arg getFullArg(char left, char right);
	/*!
	 * Get an argument enclosed by \p left and \p right.
	 * \returns the argument (without \p left and \p right) or the empty
	 * string if the next non-space token is not \p left. Use
	 * getFullArg() if you need to know wether there was an empty
	 * argument or no argument at all.
	 */
	std::string getArg(char left, char right);
	/*!
	 * \returns getFullArg('[', ']') including the brackets or the
	 * empty string if there is no such argument.
	 */
	std::string getFullOpt();
	/*!
	 * \returns getArg('[', ']') including the brackets or the
	 * empty string if there is no such argument.
	 */
	std::string getOpt();
	/*!
	 * \returns getFullArg('(', ')') including the parentheses or the
	 * empty string if there is no such argument.
	 */
	std::string getFullParentheseArg();
	/*!
	 * \returns the contents of the environment \p name.
	 * <tt>\begin{name}</tt> must be parsed already, <tt>\end{name}</tt>
	 * is parsed but not returned.
	 */
	std::string const verbatimEnvironment(std::string const & name);
	/*!
	 * Returns the character of the current token and increments
	 * the token position.
	 */
	char getChar();
	///
	void error(std::string const & msg);
	/// Parses one token from \p is 
	void tokenize_one();
	///
	void push_back(Token const & t);
	/// The previous token.
	Token const & prev_token() const;
	/// The current token.
	Token const & curr_token() const;
	/// The next token.
	Token const & next_token();
	/// Make the next token current and return that.
	Token const & get_token();
	/// \return whether the current token starts a new paragraph
	bool isParagraph();
	/// skips spaces (and comments if \p skip_comments is true)
	void skip_spaces(bool skip_comments = false);
	/// puts back spaces (and comments if \p skip_comments is true)
	void unskip_spaces(bool skip_comments = false);
	///
	void lex(std::string const & s);
	///
	bool good();
	///
	std::string verbatim_item();
	///
	std::string verbatimOption();
	/// resets the parser to initial state
	void reset();
	///
	void setCatCode(char c, CatCode cat);
	///
	CatCode getCatCode(char c) const;

private:
	///
	int lineno_;
	///
	std::vector<Token> tokens_;
	///
	unsigned pos_;
	///
	std::istringstream * iss_;
	///
	std::istream & is_;
};



} // namespace lyx

#endif
