/**
 * \file Lexer.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Lexer.h"

#include "debug.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/gzstream.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/types.h"
#include "support/unicode.h"

#include <boost/utility.hpp>

#include <functional>
#include <istream>
#include <stack>
#include <sstream>
#include <vector>


namespace lyx {

using support::compare_ascii_no_case;
using support::FileName;
using support::getFormatFromContents;
using support::isStrDbl;
using support::isStrInt;
using support::ltrim;
using support::makeDisplayPath;
using support::prefixIs;
using support::split;
using support::subst;
using support::trim;

using std::endl;
using std::getline;
using std::lower_bound;
using std::sort;
using std::string;
using std::ios;
using std::istream;
using std::ostream;


//////////////////////////////////////////////////////////////////////
//
// Lexer::Pimpl
//
//////////////////////////////////////////////////////////////////////


///
class Lexer::Pimpl : boost::noncopyable {
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
	gz::gzstreambuf gz_;

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



namespace {

class compare_tags
	: public std::binary_function<keyword_item, keyword_item, bool> {
public:
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


Lexer::Pimpl::Pimpl(keyword_item * tab, int num)
	: is(&fb_), table(tab), no_items(num),
	  status(0), lineno(0), commentChar('#')
{
	verifyTable();
}


string const Lexer::Pimpl::getString() const
{
	return buff;
}


docstring const Lexer::Pimpl::getDocString() const
{
	return from_utf8(buff);
}


void Lexer::Pimpl::printError(string const & message) const
{
	string const tmpmsg = subst(message, "$$Token", getString());
	lyxerr << "LyX: " << tmpmsg << " [around line " << lineno
		<< " of file " << to_utf8(makeDisplayPath(name)) << ']' << endl;
}


void Lexer::Pimpl::printTable(ostream & os)
{
	os << "\nNumber of tags: " << no_items << endl;
	for (int i= 0; i < no_items; ++i)
		os << "table[" << i
		   << "]:  tag: `" << table[i].tag
		   << "'  code:" << table[i].code << '\n';
	os.flush();
}


void Lexer::Pimpl::verifyTable()
{
	// Check if the table is sorted and if not, sort it.
	if (table
	    && !lyx::sorted(table, table + no_items, compare_tags())) {
		lyxerr << "The table passed to Lexer is not sorted!\n"
		       << "Tell the developers to fix it!" << endl;
		// We sort it anyway to avoid problems.
		lyxerr << "\nUnsorted:" << endl;
		printTable(lyxerr);

		sort(table, table + no_items, compare_tags());
		lyxerr << "\nSorted:" << endl;
		printTable(lyxerr);
	}
}


void Lexer::Pimpl::pushTable(keyword_item * tab, int num)
{
	pushed_table tmppu(table, no_items);
	pushed.push(tmppu);

	table = tab;
	no_items = num;

	verifyTable();
}


void Lexer::Pimpl::popTable()
{
	if (pushed.empty()) {
		lyxerr << "Lexer error: nothing to pop!" << endl;
		return;
	}

	pushed_table tmp = pushed.top();
	pushed.pop();
	table = tmp.table_elem;
	no_items = tmp.table_siz;
}


bool Lexer::Pimpl::setFile(FileName const & filename)
{
	// Check the format of the file.
	string const format = getFormatFromContents(filename);

	if (format == "gzip" || format == "zip" || format == "compress") {
		LYXERR(Debug::LYXLEX) << "lyxlex: compressed" << endl;
		// The check only outputs a debug message, because it triggers
		// a bug in compaq cxx 6.2, where is_open() returns 'true' for
		// a fresh new filebuf.  (JMarc)
		if (gz_.is_open() || istream::off_type(is.tellg()) > -1)
			lyxerr[Debug::LYXLEX] << "Error in LyXLex::setFile: "
				"file or stream already set." << endl;
		gz_.open(filename.toFilesystemEncoding().c_str(), ios::in);
		is.rdbuf(&gz_);
		name = filename.absFilename();
		lineno = 0;
		return gz_.is_open() && is.good();
	} else {
		LYXERR(Debug::LYXLEX) << "lyxlex: UNcompressed" << endl;

		// The check only outputs a debug message, because it triggers
		// a bug in compaq cxx 6.2, where is_open() returns 'true' for
		// a fresh new filebuf.  (JMarc)
		if (fb_.is_open() || istream::off_type(is.tellg()) > 0)
			LYXERR(Debug::LYXLEX) << "Error in Lexer::setFile: "
				"file or stream already set." << endl;
		fb_.open(filename.toFilesystemEncoding().c_str(), ios::in);
		is.rdbuf(&fb_);
		name = filename.absFilename();
		lineno = 0;
		return fb_.is_open() && is.good();
	}
}


void Lexer::Pimpl::setStream(istream & i)
{
	if (fb_.is_open() || istream::off_type(is.tellg()) > 0)
		LYXERR(Debug::LYXLEX)  << "Error in Lexer::setStream: "
			"file or stream already set." << endl;
	is.rdbuf(i.rdbuf());
	lineno = 0;
}


void Lexer::Pimpl::setCommentChar(char c)
{
	commentChar = c;
}


bool Lexer::Pimpl::next(bool esc /* = false */)
{
	if (!pushTok.empty()) {
		// There can have been a whole line pushed so
		// we extract the first word and leaves the rest
		// in pushTok. (Lgb)
		if (pushTok[0] == '\\' && pushTok.find(' ') != string::npos) {
			buff.clear();
			pushTok = split(pushTok, buff, ' ');
		} else {
			buff = pushTok;
			pushTok.clear();
		}
		status = LEX_TOKEN;
		return true;
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

				LYXERR(Debug::LYXLEX) << "Comment read: `" << c
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

				buff.resize(buff.size()-1);
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
		if (status)
			return true;

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
			if (c == ',')
				continue;

			if (c == commentChar) {
				// Read rest of line (fast :-)
#if 1
				// That is still not fast... (Lgb)
				string dummy;
				getline(is, dummy);

				LYXERR(Debug::LYXLEX) << "Comment read: `" << c
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

					if (!escaped && c == '\"')
						break;
				} while (c != '\n' && is);

				if (c != '\"') {
					printError("Missing quote");
					if (c == '\n')
						++lineno;
				}

				buff.resize(buff.size() -1);
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

		if (status)
			return true;

		status = is.eof() ? LEX_FEOF : LEX_UNDEF;
		buff.clear();
		return false;
	}
}


int Lexer::Pimpl::search_kw(char const * const tag) const
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


int Lexer::Pimpl::lex()
{
	//NOTE: possible bug.
	if (next() && status == LEX_TOKEN)
		return search_kw(getString().c_str());
	return status;
}


bool Lexer::Pimpl::eatLine()
{
	buff.clear();

	unsigned char c = '\0';
	char cc = 0;
	while (is && c != '\n') {
		is.get(cc);
		c = cc;
		//LYXERR(Debug::LYXLEX) << "Lexer::EatLine read char: `"
		//		      << c << '\'' << endl;
		if (c != '\r')
			buff.push_back(c);
	}

	if (c == '\n') {
		++lineno;
		buff.resize(buff.size() - 1);
		status = LEX_DATA;
		return true;
	} else if (buff.length() > 0) { // last line
		status = LEX_DATA;
		return true;
	} else {
		return false;
	}
}


bool Lexer::Pimpl::nextToken()
{
	if (!pushTok.empty()) {
		// There can have been a whole line pushed so
		// we extract the first word and leaves the rest
		// in pushTok. (Lgb)
		if (pushTok[0] == '\\' && pushTok.find(' ') != string::npos) {
			buff.clear();
			pushTok = split(pushTok, buff, ' ');
		} else {
			buff = pushTok;
			pushTok.clear();
		}
		status = LEX_TOKEN;
		return true;
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
	if (status)
		return true;

	status = is.eof() ? LEX_FEOF: LEX_UNDEF;
	buff.clear();
	return false;
}


bool Lexer::Pimpl::inputAvailable()
{
	return is.good();
}


void Lexer::Pimpl::pushToken(string const & pt)
{
	pushTok = pt;
}




//////////////////////////////////////////////////////////////////////
//
// Lexer
//
//////////////////////////////////////////////////////////////////////

Lexer::Lexer(keyword_item * tab, int num)
	: pimpl_(new Pimpl(tab, num))
{}


Lexer::~Lexer()
{
	delete pimpl_;
}


bool Lexer::isOK() const
{
	return pimpl_->inputAvailable();
}


void Lexer::setLineNo(int l)
{
	pimpl_->lineno = l;
}


int Lexer::getLineNo() const
{
	return pimpl_->lineno;
}


istream & Lexer::getStream()
{
	return pimpl_->is;
}


void Lexer::pushTable(keyword_item * tab, int num)
{
	pimpl_->pushTable(tab, num);
}


void Lexer::popTable()
{
	pimpl_->popTable();
}


void Lexer::printTable(ostream & os)
{
	pimpl_->printTable(os);
}


void Lexer::printError(string const & message) const
{
	pimpl_->printError(message);
}


bool Lexer::setFile(support::FileName const & filename)
{
	return pimpl_->setFile(filename);
}


void Lexer::setStream(istream & i)
{
	pimpl_->setStream(i);
}


void Lexer::setCommentChar(char c)
{
	pimpl_->setCommentChar(c);
}


int Lexer::lex()
{
	return pimpl_->lex();
}


int Lexer::getInteger() const
{
	lastReadOk_ = pimpl_->status == LEX_DATA || pimpl_->status == LEX_TOKEN;
	if (!lastReadOk_) {
		pimpl_->printError("integer token missing");
		return -1;
	}

	if (isStrInt(pimpl_->getString()))
		return convert<int>(pimpl_->getString());

	lastReadOk_ = false;
	pimpl_->printError("Bad integer `$$Token'");
	return -1;
}


double Lexer::getFloat() const
{
	// replace comma with dot in case the file was written with
	// the wrong locale (should be rare, but is easy enough to
	// avoid).
	lastReadOk_ = pimpl_->status == LEX_DATA || pimpl_->status == LEX_TOKEN;
	if (!lastReadOk_) {
		pimpl_->printError("float token missing");
		return -1;
	}

	string const str = subst(pimpl_->getString(), ",", ".");
	if (isStrDbl(str))
		return convert<double>(str);

	lastReadOk_ = false;
	pimpl_->printError("Bad float `$$Token'");
	return -1;
}


string const Lexer::getString() const
{
	lastReadOk_ = pimpl_->status == LEX_DATA || pimpl_->status == LEX_TOKEN;

	if (lastReadOk_)
	return pimpl_->getString();

	return string();
}


docstring const Lexer::getDocString() const
{
	lastReadOk_ = pimpl_->status == LEX_DATA || pimpl_->status == LEX_TOKEN;

	if (lastReadOk_)
		return pimpl_->getDocString();

	return docstring();
}


// I would prefer to give a tag number instead of an explicit token
// here, but it is not possible because Buffer::readDocument uses
// explicit tokens (JMarc)
string const Lexer::getLongString(string const & endtoken)
{
	string str, prefix;
	bool firstline = true;

	while (pimpl_->is) { //< eatLine only reads from is, not from pushTok
		if (!eatLine())
			// blank line in the file being read
			continue;

		string const token = trim(getString(), " \t");

		LYXERR(Debug::PARSER) << "LongString: `"
				      << getString() << '\'' << endl;

		// We do a case independent comparison, like search_kw does.
		if (compare_ascii_no_case(token, endtoken) == 0)
			break;

		string tmpstr = getString();
		if (firstline) {
			string::size_type i(tmpstr.find_first_not_of(' '));
			if (i != string::npos)
				prefix = tmpstr.substr(0, i);
			firstline = false;
			LYXERR(Debug::PARSER)
				<< "Prefix = `" << prefix << "\'" << endl;
		}

		// further lines in long strings may have the same
		// whitespace prefix as the first line. Remove it.
		if (prefix.length() && prefixIs(tmpstr, prefix)) {
			tmpstr.erase(0, prefix.length() - 1);
		}

		str += ltrim(tmpstr, "\t") + '\n';
	}

	if (!pimpl_->is) {
		printError("Long string not ended by `" + endtoken + '\'');
	}

	return str;
}


bool Lexer::getBool() const
{
	if (pimpl_->getString() == "true") {
		lastReadOk_ = true;
		return true;
	} else if (pimpl_->getString() != "false") {
		pimpl_->printError("Bad boolean `$$Token'. "
				   "Use \"false\" or \"true\"");
		lastReadOk_ = false;
	}
	lastReadOk_ = true;
	return false;
}


bool Lexer::eatLine()
{
	return pimpl_->eatLine();
}


bool Lexer::next(bool esc)
{
	return pimpl_->next(esc);
}


bool Lexer::nextToken()
{
	return pimpl_->nextToken();
}


void Lexer::pushToken(string const & pt)
{
	pimpl_->pushToken(pt);
}


Lexer::operator void const *() const
{
	// This behaviour is NOT the same as the std::streams which would
	// use fail() here. However, our implementation of getString() et al.
	// can cause the eof() and fail() bits to be set, even though we
	// haven't tried to read 'em.
	return lastReadOk_? this : 0;
}


bool Lexer::operator!() const
{
	return !lastReadOk_;
}


Lexer & Lexer::operator>>(std::string & s)
{
	if (isOK()) {
		next();
		s = getString();
	} else {
		lastReadOk_ = false;
	}
	return *this;
}


Lexer & Lexer::operator>>(docstring & s)
{
	if (isOK()) {
		next();
		s = getDocString();
	} else {
		lastReadOk_ = false;
	}
	return *this;
}


Lexer & Lexer::operator>>(double & s)
{
	if (isOK()) {
		next();
		s = getFloat();
	} else {
		lastReadOk_ = false;
	}
	return *this;
}


Lexer & Lexer::operator>>(int & s)
{
	if (isOK()) {
		next();
		s = getInteger();
	} else {
		lastReadOk_ = false;
	}
	return *this;
}


Lexer & Lexer::operator>>(unsigned int & s)
{
	if (isOK()) {
		next();
		s = getInteger();
	} else {
		lastReadOk_ = false;
	}
	return *this;
}


Lexer & Lexer::operator>>(bool & s)
{
	if (isOK()) {
		next();
		s = getBool();
	} else {
		lastReadOk_ = false;
	}
	return *this;
}


/// quotes a string, e.g. for use in preferences files or as an argument of the "log" dialog
string const Lexer::quoteString(string const & arg)
{
	std::ostringstream os;
	os << '"' << subst(subst(arg, "\\", "\\\\"), "\"", "\\\"") << '"';
	return os.str();
}


} // namespace lyx
