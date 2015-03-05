// -*- C++ -*-
/**
 * \file Lexer.h
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

#ifndef LEXER_H
#define LEXER_H

#include "support/strfwd.h"
#include <string>

namespace lyx {

namespace support { class FileName; }

class PushPopHelper;

/** A helper structure to describe a keyword for the Lexer.
	Usually used bundled in C style arrays and passed to the 
	Lexer using a LexerKeywordTable object.
*/
struct LexerKeyword
{
	/// the string to be recognized
	char const * tag;
	/// a corresponding numerical id
	int code;
};


/** Generalized simple lexical analizer.
	Use the method isOK() to check if there is still data available
	for lexing. Use one of the the operators void* or ! to test if
	the last reading operation was successful.

	Example:

	int readParam(LyxLex & lex)
	{
		int param = 1;    // default value
		if (lex.isOK()) { // the lexer has data to read
			int p;          // temporary variable
			lex >> p;
			if (lex)
				param = p; // only use the input if reading was successful
		}
		return param;
	}

    @see LyXRC.cpp for an example of usage.
  */
class Lexer
{
public:
	/// initialize Lexer with no special keywords.
	Lexer();
	/// initialize Lexer with a bunch of keywords
	template<int N> Lexer(LexerKeyword (&table)[N])
	: pimpl_(0), lastReadOk_(false) { init(table, N); }

	///
	~Lexer();

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
	/// FIXME: test also if pushToken is not empty
	/// FIXME: the method should be renamed to something like
	///        dataAvailable(), in order to reflect the real behavior
	bool isOK() const;
	/// FIXME: The next two operators should be replaced by one method
	///        called e.g. lastReadOk(), in order to reflect the real
	///        behavior
	/// last read operation was successful.
	operator void const *() const;
	/// last read operation was not successful
	bool operator!() const;
	/// return true if able to open file, else false
	bool setFile(support::FileName const & filename);
	///
	void setStream(std::istream & is);
	///
	std::istream & getStream();
	/// Danger! Don't use it unless you know what you are doing.
	void setLineNumber(int l);
	/// Change the character that begins a comment. Default is '#'
	void setCommentChar(char c);

	/// returns a lex code
	int lex();

	/// Read the next string, as delimited by double quotes or 
	/// whitespace. If esc is true, then we remember that some chars 
	/// might be escaped: \" at least.
	bool next(bool esc = false);

	/// Read next token. This one is almost the same as next(),
	/// but it will consider " as a regular character and always
	/// split a word if it contains a backslash.
	bool nextToken();

	/// Puts the rest of the line in the buffer, where it will 
	/// be available via getString() or getDocString().
	bool eatLine();

	/// Push a token, that next token got from lyxlex.
	void pushToken(std::string const &);

	/// return the current line number
	int lineNumber() const;

	///
	int getInteger() const;
	///
	bool getBool() const;
	///
	double getFloat() const;
	///
	std::string const getString() const;
	///
	docstring const getDocString() const;
	/** Get a long string, ended by the tag `endtag'.
	    This string can span several lines. The first line
	    serves as a template for how many spaces the lines
	    are indented. This much white space is skipped from
	    each following line. This mechanism does not work
	    perfectly if you use tabs.
	*/
	std::string const getLongString(std::string const & endtag);

	/// Pushes a token list on a stack and replaces it with a new one.
	template<int N> void pushTable(LexerKeyword (&table)[N])
		{ pushTable(table, N); }

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
	/// Used to dispaly context information in case of errors.
	void setContext(std::string const & functionName);

	/// extract string
	Lexer & operator>>(std::string &);
	/// extract docstring
	Lexer & operator>>(docstring &);
	/// extract double
	Lexer & operator>>(double &);
	/// extract integer
	Lexer & operator>>(int &);
	/// extract unsigned integer
	Lexer & operator>>(unsigned int &);
	/// extract bool
	Lexer & operator>>(bool &);
	/// extract first char of the string
	Lexer & operator>>(char &);

	/// read and check a required token
	Lexer & operator>>(char const * required);
	/// check for an optional token and swallow it if present.
	bool checkFor(char const * required);

	/// Quotes a string so that reading it again with Lexer::next(true)
	/// gets the original string
	static std::string quoteString(std::string const &);
	/// Quotes a docstring so that reading it again with Lexer::next(true)
	/// gets the original string
	static docstring quoteString(docstring const &);

private:
	/// noncopyable
	Lexer(Lexer const &);
	void operator=(Lexer const &);

	///
	friend class PushPopHelper;

	///
	void init(LexerKeyword *, int);
	void pushTable(LexerKeyword *, int);

	///
	class Pimpl;
	///
	Pimpl * pimpl_;
	///
	mutable bool lastReadOk_;
};


/// extract something constructable from a string, i.e. a LaTeX length
template <class T>
Lexer & operator>>(Lexer & lex, T & t)
{
	if (lex.next())
		t = T(lex.getString());
	return lex;
}


/** Use to enable multiple exit points.
    This is needed to ensure that the pop is done upon exit from methods
    with more than one exit point or that can return as a response to
    exceptions.
    @author Lgb
*/
class PushPopHelper
{
public:
	///
	template<int N>
	PushPopHelper(Lexer & l, LexerKeyword (&table)[N])
		: lex(l)
	{
		lex.pushTable(table, N);
	}
	///
	~PushPopHelper()
	{
		lex.popTable();
	}
	///
	Lexer & lex;
};
/** Avoid wrong usage of PushPopHelper.
    To avoid wrong usage:
    PushPopHelper(...); // wrong
    PushPopHelper pph(...); // right
*/
#define PushPopHelper(x, y, z) unnamed_PushPopHelper;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal


} // namespace lyx

#endif
