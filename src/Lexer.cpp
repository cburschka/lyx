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
#include "Format.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gzstream.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/types.h"

#include <functional>
#include <istream>
#include <stack>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

//////////////////////////////////////////////////////////////////////
//
// Lexer::Pimpl
//
//////////////////////////////////////////////////////////////////////


///
class Lexer::Pimpl {
public:
	///
	Pimpl(LexerKeyword * tab, int num);
	///
	string const getString() const;
	///
	docstring const getDocString() const;
	///
	void printError(string const & message) const;
	///
	void printTable(ostream & os);
	///
	void pushTable(LexerKeyword * tab, int num);
	///
	void popTable();
	///
	bool setFile(FileName const & filename);
	///
	void setStream(istream & i);
	///
	void setCommentChar(char c);
	///
	bool next(bool esc = false);
	///
	int searchKeyword(char const * const tag) const;
	///
	int lex();
	///
	bool eatLine();
	///
	bool nextToken();
	/// test if there is a pushed token or the stream is ok
	bool inputAvailable();
	///
	void pushToken(string const &);
	/// fb_ is only used to open files, the stream is accessed through is.
	filebuf fb_;

	/// gz_ is only used to open files, the stream is accessed through is.
	gz::gzstreambuf gz_;

	/// the stream that we use.
	istream is;
	///
	string name;
	///
	LexerKeyword * table;
	///
	int no_items;
	///
	string buff;
	///
	int status;
	///
	int lineno;
	///
	string pushTok;
	///
	char commentChar;
	/// used for error messages
	string context;
private:
	/// non-copyable
	Pimpl(Pimpl const &);
	void operator=(Pimpl const &);

	///
	void verifyTable();
	///
	class PushedTable {
	public:
		///
		PushedTable()
			: table_elem(0), table_siz(0) {}
		///
		PushedTable(LexerKeyword * ki, int siz)
			: table_elem(ki), table_siz(siz) {}
		///
		LexerKeyword * table_elem;
		///
		int table_siz;
	};
	///
	stack<PushedTable> pushed;
};



namespace {

class CompareTags
	: public binary_function<LexerKeyword, LexerKeyword, bool> {
public:
	// used by lower_bound, sort and sorted
	bool operator()(LexerKeyword const & a, LexerKeyword const & b) const
	{
		// we use the ascii version, because in turkish, 'i'
		// is not the lowercase version of 'I', and thus
		// turkish locale breaks parsing of tags.
		return compare_ascii_no_case(a.tag, b.tag) < 0;
	}
};

} // end of anon namespace


Lexer::Pimpl::Pimpl(LexerKeyword * tab, int num)
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
		<< " of file " << to_utf8(makeDisplayPath(name))
		<< " current token: '" << getString() << "'"
		<< " context: '" << context << "']" << endl;
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
	    && !lyx::sorted(table, table + no_items, CompareTags())) {
		lyxerr << "The table passed to Lexer is not sorted!\n"
		       << "Tell the developers to fix it!" << endl;
		// We sort it anyway to avoid problems.
		lyxerr << "\nUnsorted:" << endl;
		printTable(lyxerr);

		sort(table, table + no_items, CompareTags());
		lyxerr << "\nSorted:" << endl;
		printTable(lyxerr);
	}
}


void Lexer::Pimpl::pushTable(LexerKeyword * tab, int num)
{
	PushedTable tmppu(table, no_items);
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

	PushedTable tmp = pushed.top();
	pushed.pop();
	table = tmp.table_elem;
	no_items = tmp.table_siz;
}


bool Lexer::Pimpl::setFile(FileName const & filename)
{
	// Check the format of the file.
	if (formats.isZippedFile(filename)) {
		LYXERR(Debug::LYXLEX, "lyxlex: compressed");
		// The check only outputs a debug message, because it triggers
		// a bug in compaq cxx 6.2, where is_open() returns 'true' for
		// a fresh new filebuf.  (JMarc)
		if (gz_.is_open() || istream::off_type(is.tellg()) > -1)
			LYXERR(Debug::LYXLEX, "Error in LyXLex::setFile: "
				"file or stream already set.");
		gz_.open(filename.toFilesystemEncoding().c_str(), ios::in);
		is.rdbuf(&gz_);
		name = filename.absFileName();
		lineno = 0;
		if (!gz_.is_open() || !is.good())
			return false;
	} else {
		LYXERR(Debug::LYXLEX, "lyxlex: UNcompressed");

		// The check only outputs a debug message, because it triggers
		// a bug in compaq cxx 6.2, where is_open() returns 'true' for
		// a fresh new filebuf.  (JMarc)
		if (fb_.is_open() || istream::off_type(is.tellg()) > 0) {
			LYXERR(Debug::LYXLEX, "Error in Lexer::setFile: "
				"file or stream already set.");
		}
		fb_.open(filename.toSafeFilesystemEncoding().c_str(), ios::in);
		is.rdbuf(&fb_);
		name = filename.absFileName();
		lineno = 0;
		if (!fb_.is_open() || !is.good())
			return false;
	}

	// Skip byte order mark.
	if (is.peek() == 0xef) {
		is.get();
		if (is.peek() == 0xbb) {
			is.get();
			LASSERT(is.get() == 0xbf, /**/);
		} else
			is.unget();
	}

	return true;
}


void Lexer::Pimpl::setStream(istream & i)
{
	if (fb_.is_open() || istream::off_type(is.tellg()) > 0) {
		LYXERR(Debug::LYXLEX, "Error in Lexer::setStream: "
			"file or stream already set.");
	}
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

			LYXERR(Debug::LYXLEX, "Comment read: `" << c << dummy << '\'');
#else
			// unfortunately ignore is buggy (Lgb)
			is.ignore(100, '\n');
#endif
			++lineno;
			continue;
		}

		if (c == '\"') {
			buff.clear();

			if (esc) {

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

			} else {

				do {
					is.get(cc);
					c = cc;
					if (c != '\r')
						buff.push_back(c);
				} while (c != '\"' && c != '\n' && is);

			}

			if (c != '\"') {
				printError("Missing quote");
				if (c == '\n')
					++lineno;
			}

			buff.resize(buff.size() - 1);
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
				if (esc && c == '\\') {
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
}


int Lexer::Pimpl::searchKeyword(char const * const tag) const
{
	LexerKeyword search_tag = { tag, 0 };
	LexerKeyword * res =
		lower_bound(table, table + no_items,
			    search_tag, CompareTags());
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
		return searchKeyword(getString().c_str());
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
		//LYXERR(Debug::LYXLEX, "Lexer::EatLine read char: `" << c << '\'');
		if (c != '\r' && is)
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
		if ((c >= ' ' || c == '\t') && is) {
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
				} while ((c >= ' ' || c == '\t') && c != '\\' && is);
			}

			if (c == '\\')
				is.putback(c); // put it back
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

Lexer::Lexer()
	: pimpl_(new Pimpl(0, 0))
{}


void Lexer::init(LexerKeyword * tab, int num)
{
	 pimpl_ = new Pimpl(tab, num);
}


Lexer::~Lexer()
{
	delete pimpl_;
}


bool Lexer::isOK() const
{
	return pimpl_->inputAvailable();
}


void Lexer::setLineNumber(int l)
{
	pimpl_->lineno = l;
}


int Lexer::lineNumber() const
{
	return pimpl_->lineno;
}


istream & Lexer::getStream()
{
	return pimpl_->is;
}


void Lexer::pushTable(LexerKeyword * tab, int num)
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


bool Lexer::setFile(FileName const & filename)
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


string const Lexer::getString(bool trim) const
{
	lastReadOk_ = pimpl_->status == LEX_DATA || pimpl_->status == LEX_TOKEN;

	if (lastReadOk_)
		return trim ? support::trim(pimpl_->getString(), "\t ") : pimpl_->getString();

	return string();
}


docstring const Lexer::getDocString(bool trim) const
{
	lastReadOk_ = pimpl_->status == LEX_DATA || pimpl_->status == LEX_TOKEN;

	if (lastReadOk_)
		return trim ? support::trim(pimpl_->getDocString(), "\t ") : pimpl_->getDocString();

	return docstring();
}


// I would prefer to give a tag number instead of an explicit token
// here, but it is not possible because Buffer::readDocument uses
// explicit tokens (JMarc)
string const Lexer::getLongString(string const & endtoken)
{
	string str;
	string prefix;
	bool firstline = true;

	while (pimpl_->is) { //< eatLine only reads from is, not from pushTok
		if (!eatLine())
			// blank line in the file being read
			continue;

		string const token = trim(getString(), " \t");

		LYXERR(Debug::PARSER, "LongString: `" << getString() << '\'');

		// We do a case independent comparison, like searchKeyword does.
		if (compare_ascii_no_case(token, endtoken) == 0)
			break;

		string tmpstr = getString();
		if (firstline) {
			size_t i = tmpstr.find_first_not_of(' ');
			if (i != string::npos)
				prefix = tmpstr.substr(0, i);
			firstline = false;
			LYXERR(Debug::PARSER, "Prefix = `" << prefix << "\'");
		}

		// further lines in long strings may have the same
		// whitespace prefix as the first line. Remove it.
		if (prefix.length() && prefixIs(tmpstr, prefix))
			tmpstr.erase(0, prefix.length() - 1);

		str += ltrim(tmpstr, "\t") + '\n';
	}

	if (!pimpl_->is)
		printError("Long string not ended by `" + endtoken + '\'');

	return str;
}


bool Lexer::getBool() const
{
	string const s = pimpl_->getString();	
	if (s == "false" || s == "0") {
		lastReadOk_ = true;
		return false;
	}
	if (s == "true" || s == "1") {
		lastReadOk_ = true;
		return true;
	}
	pimpl_->printError("Bad boolean `$$Token'. "
				 "Use \"false\" or \"true\"");
	lastReadOk_ = false;
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
	// This behaviour is NOT the same as the streams which would
	// use fail() here. However, our implementation of getString() et al.
	// can cause the eof() and fail() bits to be set, even though we
	// haven't tried to read 'em.
	return lastReadOk_? this : 0;
}


bool Lexer::operator!() const
{
	return !lastReadOk_;
}


Lexer & Lexer::operator>>(string & s)
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


Lexer & Lexer::operator>>(char & c)
{
	string s;
	operator>>(s);
	if (!s.empty())
		c = s[0];
	return *this;
}


// quotes a string, e.g. for use in preferences files or as an argument
// of the "log" dialog
string Lexer::quoteString(string const & arg)
{
	string res;
	res += '"';
	res += subst(subst(arg, "\\", "\\\\"), "\"", "\\\"");
	res += '"';
	return res;
}


// same for docstring
docstring Lexer::quoteString(docstring const & arg)
{
	docstring res;
	res += '"';
	res += subst(subst(arg, from_ascii("\\"), from_ascii("\\\\")), 
		     from_ascii("\""), from_ascii("\\\""));
	res += '"';
	return res;
}


Lexer & Lexer::operator>>(char const * required)
{
	string token;
	*this >> token;
	if (token != required) {
		LYXERR0("Missing '" << required << "'-tag in " << pimpl_->context 
			<< ". Got " << token << " instead. Line: " << lineNumber());
		pushToken(token);
	}
	return *this;
}


bool Lexer::checkFor(char const * required)
{
	string token;
	*this >> token;
	if (token == required)
		return true;
	pushToken(token);
	return false;
}


void Lexer::setContext(std::string const & str)
{
	pimpl_->context = str;
}


} // namespace lyx
