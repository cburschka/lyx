// -*- C++ -*-

#ifndef LYXLEX_PIMPL_H
#define LYXLEX_PIMPL_H

#include <fstream>
#include <stack>
#include <boost/utility.hpp>

#include "lyxlex.h"

#ifdef __GNUG__
#pragma interface
#endif

///
struct LyXLex::Pimpl : boost::noncopyable {
	///
	enum {
		///
		LEX_MAX_BUFF = 2048
	};

	///
	Pimpl(keyword_item * tab, int num);
	///
	string const getString() const;
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
	void setCommentChar(char c);
	///
	bool next(bool esc = false);
	///
	int search_kw(char const * const tag) const;
	///
	int lex();
	///
	bool eatLine();
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
	int status;
	///
	int lineno;
	///
	string pushTok;
	///
	char commentChar;
private:
	///
	void verifyTable();
	///
	struct pushed_table {
		///
		pushed_table()
			: table_elem(0), table_siz(0) {}
		///
		pushed_table(keyword_item * ki, int siz)
			: table_elem(ki), table_siz(siz) {}
		///
		keyword_item * table_elem;
		///
		int table_siz;
	};
	///
	std::stack<pushed_table> pushed;
};
#endif
