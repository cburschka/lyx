// -*- C++ -*-

#ifndef LYXLEX_PIMPL_H
#define LYXLEX_PIMPL_H

#include <fstream>

#include "lyxlex.h"

#ifdef __GNUG__
#pragma interface
#endif

///
struct LyXLex::Pimpl {
	///
	enum {
		///
		LEX_MAX_BUFF = 2048
	};
	
	///
	struct pushed_table {
		///
		pushed_table(){
			next = 0;
			table_elem = 0;
		}
		///
		pushed_table * next;
		///
		keyword_item * table_elem;
		///
		int table_siz;
	};
	///
	Pimpl(keyword_item * tab, int num);
	///
	string GetString() const;
	///
	void printError(string const & message) const;
	///
	void printTable(std::ostream & os);
	///
	void pushTable(keyword_item * tab, int num);
	///
	void popTable();
	///
	bool setFile(string const & filename);
	///
	void setStream(std::istream & i);
	///
	bool next(bool esc = false);
	///
	int search_kw(char const * const tag) const;
	///
	int lex();
	///
	bool EatLine();
	///
	bool nextToken();
	///
	void pushToken(string const &);

	/// fb__ is only used to open files, the stream is accessed through is
	std::filebuf fb__;
	/// the stream that we use.
	std::istream is;
	/// 
	string name;
	///
	keyword_item * table;
	///
	int no_items;
	///
	char buff[LEX_MAX_BUFF];
	///
	short status;
	///
	pushed_table * pushed;
	///
	int lineno;
	///
	string pushTok;
};
#endif
