
#ifndef PARSER_H
#define PARSER_H

#include "LString.h"
#include <vector>

enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE};

mode_type asMode(mode_type oldmode, string const & str);


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

ostream & operator<<(ostream & os, Token const & t);


//
// Actual parser class
//

class Parser {

public:
	///
	Parser(istream & is);

	///
	int lineno() const { return lineno_; }
	///
	void putback();
	/// dump contents to screen
	void dump() const;

	///
	string getArg(char left, char right);
	///
	char getChar();
	///
	void error(string const & msg);
	///
	void tokenize(istream & is);
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
	string verbatimItem();
	///
	string verbatimOption();

//private:
	///
	int lineno_;
	///
	vector<Token> tokens_;
	///
	unsigned pos_;
};

#endif
