// -*- C++ -*-
//  Generalized simple lexical analizer.
//  It can be used for simple syntax parsers, like lyxrc,
//  texclass and others to come.   [asierra30/03/96]
//
//   Copyright 1996 Lyx Team.
#ifndef LYXLEX_H
#define LYXLEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include <fstream>
using std::filebuf;

#include "support/LIstream.h"

#include "LString.h"

///
struct keyword_item {
	///
	char const * tag;
	///
	short code;
};

/*@Doc:
  Generalized simple lexical analizer.
  It can be used for simple syntax parsers, like lyxrc,
  texclass and others to come.
  See lyxrc.C for an example of usage.
  */
class LyXLex { 
public:
	///
	LyXLex (keyword_item *, int);

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

	/// file is open and end of file is not reached
	bool IsOK() const;
	/// return true if able to open file, else false
	bool setFile(string const & filename);
	///
	void setStream(istream & i);
	///
	istream & getStream() { return is; }
	/// Danger! Don't use it unless you know what you are doing.
	void setLineNo(int l) { lineno = l; }
	/// returns a lex code
	int lex();

	/** Just read athe next word. If esc is true remember that
	  some chars might be escaped: "\ atleast */
	bool next(bool esc = false);

	/** Read next token. This one is almost the same as next,
	  but it will consider " as a regular character and always
	  split a word if it contains a backslash.
	  */
	bool nextToken();
	
	/// 
	int GetLineNo() const { return lineno; }
	///
	int GetInteger() const;
	///
	bool GetBool() const;
	///
	float GetFloat() const;
	///
	string GetString() const;
	
	/// get a long string, ended by the tag `endtag'
	string getLongString(string const & endtag);
	
	///
	bool EatLine();
	///
	int FindToken(char const * str[]);
	///
	int CheckToken(char const * str[], int print_error);

	///
	char const * text() const { return &buff[0]; }

	/** Pushes a token list on a stack and replaces it with a new one.
	 */
	void pushTable(keyword_item *, int);

	/** Pops a token list into void and replaces it with the one now
	  on top of the stack.
	  */
	void popTable();

	/** Prints an error message with the corresponding line number
	  and file name. If message contains the substring `$$Token',
	  it is replaced with the value of GetString()
	  */
	void printError(string const & message) const;

	/**
	  Prints the current token table on the supplied ostream.
	  */
	void printTable(ostream &);
protected:
	///
	enum {
		///
		LEX_MAX_BUFF = 2048
	};

	///
	struct pushed_table {
		///
		pushed_table(){
			next= 0;
			table_elem= 0;
		}
		///
		pushed_table * next;
		///
		keyword_item * table_elem;
		///
		int table_siz;
	};

	/// fb__ is only used to open files, the stream is accessed through is
	filebuf fb__;
	/// the stream that we use.
	istream is;
	/// 
	string name;
	///
	int lineno;
	///
	keyword_item * table;
	///
	int no_items;
	///
	char buff[LEX_MAX_BUFF];
	///
	pushed_table * pushed;
	///
	int search_kw(char const * const) const;
	///
	short status;
};


inline
bool LyXLex::IsOK() const
{
	return is.good();
}


// This is needed to ensure that the pop is done upon exit from methods
// with more than one exit point or that can return as a response to
// exceptions. (Lgb)
struct pushpophelper {
	pushpophelper(LyXLex & lexrc, keyword_item * i, int s) : lex(lexrc) {
		lex.pushTable(i, s);
	}
	~pushpophelper() {
		lex.popTable();
	}
	LyXLex & lex;
};
// To avoid wrong usage:
// pushpophelper(...); // wrong
// pushpophelper pph(...); // right
// we add this macro:
#define pushpophelper(x, y, z) unnamed_pushpophelper;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal

#endif
