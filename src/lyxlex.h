// -*- C++ -*-
/**
 * \file lyxlex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

//  Generalized simple lexical analizer.
//  It can be used for simple syntax parsers, like lyxrc,
//  texclass and others to come.

#ifndef LYXLEX_H
#define LYXLEX_H

#include <boost/utility.hpp>

#include <iosfwd>
#include <string>


///
struct keyword_item {
	///
	char const * tag;
	///
	int code;
};

/** Generalized simple lexical analizer.
    It can be used for simple syntax parsers, like lyxrc,
    texclass and others to come.
    @see lyxrc.C for an example of usage.
  */
class LyXLex : boost::noncopyable {
public:
	///
	LyXLex(keyword_item *, int);
	///
	~LyXLex();

	/// Lex basic codes
	enum {
		///
		LEX_UNDEF = -1,
		///
		LEX_FEOF  = -2,
		///
		LEX_DATA  = -3,
		///
		LEX_TOKEN = -4
	};

	/// stream is open and end of stream is not reached
	bool isOK() const;
	/// stream is ok
	operator void const *() const;
	/// stream is not ok
	bool operator!() const;
	/// return true if able to open file, else false
	bool setFile(std::string const & filename);
	///
	void setStream(std::istream & is);
	///
	std::istream & getStream();
	/// Danger! Don't use it unless you know what you are doing.
	void setLineNo(int l);
	/// Change the character that begins a comment. Default is '#'
	void setCommentChar(char c);

	/// returns a lex code
	int lex();

	/** Just read athe next word. If esc is true remember that
	    some chars might be escaped: "\ atleast
	*/
	bool next(bool esc = false);

	/** Read next token. This one is almost the same as next,
	    but it will consider " as a regular character and always
	    split a word if it contains a backslash.
	*/
	bool nextToken();
	/// Push a token, that next token got from lyxlex.
	void pushToken(std::string const &);

	///
	int getLineNo() const;

	///
	int getInteger() const;
	///
	bool getBool() const;
	///
	float getFloat() const;
	///
	std::string const getString() const;

	/** Get a long string, ended by the tag `endtag'.
	    This string can span several lines. The first line
	    serves as a template for how many spaces the lines
	    are indented. This much white space is skipped from
	    each following line. This mechanism does not work
	    perfectly if you use tabs.
	*/
	std::string const getLongString(std::string const & endtag);

	///
	bool eatLine();

	/// Pushes a token list on a stack and replaces it with a new one.
	void pushTable(keyword_item *, int);

	/** Pops a token list into void and replaces it with the one now
	    on top of the stack.
	*/
	void popTable();

	/** Prints an error message with the corresponding line number
	    and file name. If message contains the substring `$$Token',
	    it is replaced with the value of GetString()
	*/
	void printError(std::string const & message) const;

	/// Prints the current token table on the supplied ostream.
	void printTable(std::ostream &);

	/// extract string
	LyXLex & operator>>(std::string &);
	/// extract float
	LyXLex & operator>>(float &);
	/// extract double
	LyXLex & operator>>(double &);
	/// extract integer
	LyXLex & operator>>(int &);
	/// extract unsigned integer
	LyXLex & operator>>(unsigned int &);
	/// extract bool
	LyXLex & operator>>(bool &);

private:
	struct Pimpl;
	///
	Pimpl * pimpl_;
};


/** Use to enable multiple exit points.
    This is needed to ensure that the pop is done upon exit from methods
    with more than one exit point or that can return as a response to
    exceptions.
    @author Lgb
*/
struct pushpophelper {
	///
	pushpophelper(LyXLex & lexrc, keyword_item * i, int s) : lex(lexrc) {
		lex.pushTable(i, s);
	}
	///
	~pushpophelper() {
		lex.popTable();
	}
	///
	LyXLex & lex;
};
/** Avoid wrong usage of pushpophelper.
    To avoid wrong usage:
    pushpophelper(...); // wrong
    pushpophelper pph(...); // right
*/
#define pushpophelper(x, y, z) unnamed_pushpophelper;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal

#endif
