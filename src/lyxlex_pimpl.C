/**
 * \file lyxlex_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxlex_pimpl.h"

#include "debug.h"

#include "support/filetools.h"
#include "support/lyxalgo.h"
#include "support/lstrings.h"

using lyx::support::compare_ascii_no_case;
using lyx::support::getFormatFromContents;
using lyx::support::MakeDisplayPath;
using lyx::support::split;
using lyx::support::subst;

using std::endl;
using std::getline;
using std::lower_bound;
using std::sort;
using std::string;
using std::ios;
using std::istream;
using std::ostream;

namespace {

struct compare_tags
	: public std::binary_function<keyword_item, keyword_item, bool> {
	// used by lower_bound, sort and sorted
	bool operator()(keyword_item const & a, keyword_item const & b) const
	{
		// we use the ascii version, because in turkish, 'i'
		// is not the lowercase version of 'I', and thus
		// turkish locale breaks parsing of tags.
		return compare_ascii_no_case(a.tag, b.tag) < 0;
	}
};

} // end of anon namespace


LyXLex::Pimpl::Pimpl(keyword_item * tab, int num)
	: is(&fb_), table(tab), no_items(num),
	  status(0), lineno(0), commentChar('#')
{
	verifyTable();
}


string const LyXLex::Pimpl::getString() const
{
	return string(buff.begin(), buff.end());
}


void LyXLex::Pimpl::printError(string const & message) const
{
	string const tmpmsg = subst(message, "$$Token", getString());
	lyxerr << "LyX: " << tmpmsg << " [around line " << lineno
	       << " of file " << MakeDisplayPath(name) << ']' << endl;
}


void LyXLex::Pimpl::printTable(ostream & os)
{
	os << "\nNumber of tags: " << no_items << endl;
	for (int i= 0; i < no_items; ++i)
		os << "table[" << i
		   << "]:  tag: `" << table[i].tag
		   << "'  code:" << table[i].code << '\n';
	os.flush();
}


void LyXLex::Pimpl::verifyTable()
{
	// Check if the table is sorted and if not, sort it.
	if (table
	    && !lyx::sorted(table, table + no_items, compare_tags())) {
		lyxerr << "The table passed to LyXLex is not sorted!\n"
		       << "Tell the developers to fix it!" << endl;
		// We sort it anyway to avoid problems.
		lyxerr << "\nUnsorted:" << endl;
		printTable(lyxerr);

		sort(table, table + no_items, compare_tags());
		lyxerr << "\nSorted:" << endl;
		printTable(lyxerr);
	}
}


void LyXLex::Pimpl::pushTable(keyword_item * tab, int num)
{
	pushed_table tmppu(table, no_items);
	pushed.push(tmppu);

	table = tab;
	no_items = num;

	verifyTable();
}


void LyXLex::Pimpl::popTable()
{
	if (pushed.empty()) {
		lyxerr << "LyXLex error: nothing to pop!" << endl;
		return;
	}

	pushed_table tmp = pushed.top();
	pushed.pop();
	table = tmp.table_elem;
	no_items = tmp.table_siz;
}


bool LyXLex::Pimpl::setFile(string const & filename)
{
	// Check the format of the file.
	string const format = getFormatFromContents(filename);

	if (format == "gzip" || format == "zip" || format == "compress") {
		lyxerr[Debug::LYXLEX] << "lyxlex: compressed" << endl;

		// The check only outputs a debug message, because it triggers
		// a bug in compaq cxx 6.2, where is_open() returns 'true' for
		// a fresh new filebuf.  (JMarc)
		if (gz_.is_open() || istream::off_type(is.tellg()) > -1)
			lyxerr[Debug::LYXLEX] << "Error in LyXLex::setFile: "
				"file or stream already set." << endl;
		gz_.open(filename.c_str(), ios::in);
		is.rdbuf(&gz_);
		name = filename;
		lineno = 0;
		return gz_.is_open() && is.good();
	} else {
		lyxerr[Debug::LYXLEX] << "lyxlex: UNcompressed" << endl;

		// The check only outputs a debug message, because it triggers
		// a bug in compaq cxx 6.2, where is_open() returns 'true' for
		// a fresh new filebuf.  (JMarc)
		if (fb_.is_open() || istream::off_type(is.tellg()) > 0)
			lyxerr[Debug::LYXLEX] << "Error in LyXLex::setFile: "
				"file or stream already set." << endl;
		fb_.open(filename.c_str(), ios::in);
		is.rdbuf(&fb_);
		name = filename;
		lineno = 0;
		return fb_.is_open() && is.good();
	}
}


void LyXLex::Pimpl::setStream(istream & i)
{
	if (fb_.is_open() || istream::off_type(is.tellg()) > 0)
		lyxerr[Debug::LYXLEX]  << "Error in LyXLex::setStream: "
			"file or stream already set." << endl;
	is.rdbuf(i.rdbuf());
	lineno = 0;
}


void LyXLex::Pimpl::setCommentChar(char c)
{
	commentChar = c;
}


bool LyXLex::Pimpl::next(bool esc /* = false */)
{
	if (!pushTok.empty()) {
		// There can have been a whole line pushed so
		// we extract the first word and leaves the rest
		// in pushTok. (Lgb)
		if (pushTok.find(' ') != string::npos && pushTok[0] == '\\') {
			string tmp;
			pushTok = split(pushTok, tmp, ' ');
			buff.assign(tmp.begin(), tmp.end());
			return true;
		} else {
			buff.assign(pushTok.begin(), pushTok.end());
			pushTok.erase();
			return true;
		}
	}
	if (!esc) {
		unsigned char c = 0; // getc() returns an int
		char cc = 0;
		status = 0;
		while (is && !status) {
			is.get(cc);
			c = cc;
			if (c == commentChar) {
				// Read rest of line (fast :-)
#if 1
				// That is not fast... (Lgb)
				string dummy;
				getline(is, dummy);

				lyxerr[Debug::LYXLEX] << "Comment read: `" << c
						      << dummy << '\'' << endl;
#else
				// unfortunately ignore is buggy (Lgb)
				is.ignore(100, '\n');
#endif
				++lineno;
				continue;
			}

			if (c == '\"') {
				buff.clear();

				do {
					is.get(cc);
					c = cc;
					if (c != '\r')
						buff.push_back(c);
				} while (c != '\"' && c != '\n' && is);

				if (c != '\"') {
					printError("Missing quote");
					if (c == '\n')
						++lineno;
				}

				buff.pop_back();
				status = LEX_DATA;
				break;
			}

			if (c == ',')
				continue;              /* Skip ','s */

				// using relational operators with chars other
				// than == and != is not safe. And if it is done
				// the type _have_ to be unsigned. It usually a
				// lot better to use the functions from cctype
			if (c > ' ' && is)  {
				buff.clear();

				do {
					buff.push_back(c);
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != ',' && is);

				status = LEX_TOKEN;
			}

			if (c == '\r' && is) {
				// The Windows support has lead to the
				// possibility of "\r\n" at the end of
				// a line.  This will stop LyX choking
				// when it expected to find a '\n'
				is.get(cc);
				c = cc;
			}

			if (c == '\n')
				++lineno;

		}
		if (status) return true;

		status = is.eof() ? LEX_FEOF: LEX_UNDEF;
		buff.clear();
		return false;
	} else {
		unsigned char c = 0; // getc() returns an int
		char cc = 0;

		status = 0;
		while (is && !status) {
			is.get(cc);
			c = cc;

			// skip ','s
			if (c == ',') continue;

			if (c == '\\') {
				// escape
				buff.clear();

				do {
					if (c == '\\') {
						// escape the next char
						is.get(cc);
						c = cc;
					}
					buff.push_back(c);
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != ',' && is);

				status = LEX_TOKEN;
				continue;
			}

			if (c == commentChar) {
				// Read rest of line (fast :-)
#if 1
				// That is still not fast... (Lgb)
				string dummy;
				getline(is, dummy);

				lyxerr[Debug::LYXLEX] << "Comment read: `" << c
						      << dummy << '\'' << endl;
#else
				// but ignore is also still buggy (Lgb)
				// This is fast (Lgb)
				is.ignore(100, '\n');
#endif
				++lineno;
				continue;
			}

			// string
			if (c == '\"') {
				buff.clear();

				bool escaped = false;
				do {
					escaped = false;
					is.get(cc);
					c = cc;
					if (c == '\r') continue;
					if (c == '\\') {
						// escape the next char
						is.get(cc);
						c = cc;
						if (c == '\"' || c == '\\')
							escaped = true;
						else
							buff.push_back('\\');
					}
					buff.push_back(c);

					if (!escaped && c == '\"') break;
				} while (c != '\n' && is);

				if (c != '\"') {
					printError("Missing quote");
					if (c == '\n')
						++lineno;
				}

				buff.pop_back();
				status = LEX_DATA;
				break;
			}

			if (c > ' ' && is) {
				buff.clear();

				do {
					if (c == '\\') {
						// escape the next char
						is.get(cc);
						c = cc;
						//escaped = true;
					}
					buff.push_back(c);
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != ',' && is);

				status = LEX_TOKEN;
			}
			// new line
			if (c == '\n')
				++lineno;
		}

		if (status) return true;

		status = is.eof() ? LEX_FEOF : LEX_UNDEF;
		buff.clear();
		return false;
	}
}


int LyXLex::Pimpl::search_kw(char const * const tag) const
{
	keyword_item search_tag = { tag, 0 };
	keyword_item * res =
		lower_bound(table, table + no_items,
			    search_tag, compare_tags());
	// use the compare_ascii_no_case instead of compare_no_case,
	// because in turkish, 'i' is not the lowercase version of 'I',
	// and thus turkish locale breaks parsing of tags.
	if (res != table + no_items
	    && !compare_ascii_no_case(res->tag, tag))
		return res->code;
	return LEX_UNDEF;
}


int LyXLex::Pimpl::lex()
{
	//NOTE: possible bug.
	if (next() && status == LEX_TOKEN) {
		return search_kw(getString().c_str());
	} else
		return status;
}


bool LyXLex::Pimpl::eatLine()
{
	buff.clear();

	unsigned char c = '\0';
	char cc = 0;
	while (is && c != '\n') {
		is.get(cc);
		c = cc;
		//lyxerr[Debug::LYXLEX] << "LyXLex::EatLine read char: `"
		//		      << c << '\'' << endl;
		if (c != '\r')
			buff.push_back(c);
	}

	if (c == '\n') {
		++lineno;
		buff.pop_back();
		status = LEX_DATA;
		return true;
	} else {
		return false;
	}
}


bool LyXLex::Pimpl::nextToken()
{
	if (!pushTok.empty()) {
		// There can have been a whole line pushed so
		// we extract the first word and leaves the rest
		// in pushTok. (Lgb)
		if (pushTok.find(' ') != string::npos && pushTok[0] == '\\') {
			string tmp;
			pushTok = split(pushTok, tmp, ' ');
			buff.assign(tmp.begin(), tmp.end());
			return true;
		} else {
			buff.assign(pushTok.begin(), pushTok.end());
			pushTok.erase();
			return true;
		}
	}

	status = 0;
	while (is && !status) {
		unsigned char c = 0;
		char cc = 0;
		is.get(cc);
		c = cc;
		if (c >= ' ' && is) {
			buff.clear();

			if (c == '\\') { // first char == '\\'
				do {
					buff.push_back(c);
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != '\\' && is);
			} else {
				do {
					buff.push_back(c);
					is.get(cc);
					c = cc;
				} while (c >= ' ' && c != '\\' && is);
			}

			if (c == '\\') is.putback(c); // put it back
			status = LEX_TOKEN;
		}

		if (c == '\n')
			++lineno;

	}
	if (status)  return true;

	status = is.eof() ? LEX_FEOF: LEX_UNDEF;
	buff.clear();
	return false;
}


void LyXLex::Pimpl::pushToken(string const & pt)
{
	pushTok = pt;
}
