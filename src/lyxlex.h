// -*- C++ -*-
//  Generalized simple lexical analizer.
//  It can be used for simple syntax parsers, like lyxrc,
//  texclass and others to come.   [asierra30/03/96]
//
//   (C) 1996 Lyx Team.
#ifndef _LYXLEX_H
#define _LYXLEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include <stdio.h>
#include "LString.h"

///
struct keyword_item {
	///
	char const* tag;
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
	LyXLex (keyword_item*, int);
	///
	~LyXLex() { if (file && owns_file) fclose(file); };

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
	bool IsOK();
	/// return true if able to open file, else false
	bool setFile(LString const & filename);
	/// if file is already read from, line numbers will be wrong.
	// should be removed
	void setFile(FILE *f);
	///
	// should be removed
	FILE *getFile() { return file; }
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
	int GetLineNo() { return lineno; }
	///
	int GetInteger();
	///
	bool GetBool();
	///
	float GetFloat();
	///
	LString GetString() const;
	
	/// get a long string, ended by the tag `endtag'
	LString getLongString(LString const &endtoken);
	
	///
	bool EatLine();
	///
	int FindToken(char const* string[]);
	///
	int CheckToken(char const* string[], int print_error);

	///
	char const *text() const { return &buff[0]; }

	/** Pushes a token list on a stack and replaces it with a new one.
	 */
	void pushTable(keyword_item*, int);

	/** Pops a token list into void and replaces it with the one now
	  on top of the stack.
	  */
	void popTable();

	/** Prints an error message with the corresponding line number
	  and file name. If message contains the substring `$$Token',
	  it is replaced with the value of GetString()
	  */
	void printError(LString const & message);

	/**
	  Prints the current token table on stderr.
	  */
	void printTable();
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
			next=NULL;
			table_elem=NULL;
		}
		///
		pushed_table *next;
		///
		keyword_item *table_elem;
		///
		int table_siz;
	};

	///
	FILE *file;
        ///
        bool owns_file;
	/// 
	LString name;
	///
	int lineno;
	///
	keyword_item *table;
	///
	int no_items;
	///
	char buff[LEX_MAX_BUFF];
	///
	pushed_table *pushed;
	///
	int search_kw(char const * const) const;
	///
	short status;
};


inline
bool LyXLex::IsOK()
{
	return (file && !feof(file));
}

#endif
