// -*- C++ -*-
/**
 * \file lyxlex_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXLEX_PIMPL_H
#define LYXLEX_PIMPL_H

#include "lyxlex.h"

#include "support/gzstream.h"

#include <boost/utility.hpp>

#include <fstream>
#include <stack>
#include <vector>

///
struct LyXLex::Pimpl : boost::noncopyable {
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
	/// fb__ is only used to open files, the stream is accessed through is.
	std::filebuf fb__;
	/// gz__ is only used to open files, the stream is accessed through is.
	gz::gzstreambuf gz__;

	/// the stream that we use.
	std::istream is;
	///
	string name;
	///
	keyword_item * table;
	///
	int no_items;
	///
	std::vector<char> buff;
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
