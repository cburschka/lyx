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

#include "support/types.h"

# include <boost/iostreams/filtering_streambuf.hpp>
# include <boost/iostreams/filter/gzip.hpp>
# include <boost/iostreams/device/file.hpp>
namespace io = boost::iostreams;

#include <boost/utility.hpp>

#include <istream>
#include <stack>
#include <vector>


namespace lyx {

namespace support { class FileName; }

///
class LyXLex::Pimpl : boost::noncopyable {
public:
	///
	Pimpl(keyword_item * tab, int num);
	///
	std::string const getString() const;
	///
	docstring const getDocString() const;
	///
	void printError(std::string const & message) const;
	///
	void printTable(std::ostream & os);
	///
	void pushTable(keyword_item * tab, int num);
	///
	void popTable();
	///
	bool setFile(support::FileName const & filename);
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
	/// test if there is a pushed token or the stream is ok
	bool inputAvailable();
	///
	void pushToken(std::string const &);
	/// fb_ is only used to open files, the stream is accessed through is.
	std::filebuf fb_;

	/// gz_ is only used to open files, the stream is accessed through is.
	io::filtering_istreambuf gz_;

	/// the stream that we use.
	std::istream is;
	///
	std::string name;
	///
	keyword_item * table;
	///
	int no_items;
	///
	std::string buff;
	///
	int status;
	///
	int lineno;
	///
	std::string pushTok;
	///
	char commentChar;
private:
	///
	void verifyTable();
	///
	class pushed_table {
	public:
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

} // namespace lyx

#endif
