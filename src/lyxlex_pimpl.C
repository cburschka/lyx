#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif
#include <algorithm>

#include "lyxlex_pimpl.h"
#include "support/lyxalgo.h"
#include "support/filetools.h"
#include "debug.h"

using std::sort;
using std::ostream;
using std::ios;
using std::istream;
using std::endl;
using std::lower_bound;

// namespace {
struct compare_tags {
	// used by lower_bound
	inline
	int operator()(keyword_item const & a, char const * const tag) const {
		return compare_no_case(a.tag, tag) < 0;
	}
	// used by sorted and sort
	inline
	int operator()(keyword_item const & a, keyword_item const & b) const {
		return compare_no_case(a.tag, b.tag) < 0;
	}
};
// } // end of anon namespace


LyXLex::Pimpl::Pimpl(keyword_item * tab, int num) 
	: is(&fb__), table(tab), no_items(num),
	  status(0), pushed(0), lineno(0)
{
	if (table && !sorted(table, table + no_items, compare_tags())) {
		lyxerr << "The table passed to LyXLex is not sorted!!\n"
		       << "Tell the developers to fix it!" << endl;
		// We sort it anyway to avoid problems.
		lyxerr << "\nUnsorted:\n";
		printTable(lyxerr);
		
		sort(table, table + no_items,
		     compare_tags());
		lyxerr << "\nSorted:\n";
		printTable(lyxerr);
	}
}


string const LyXLex::Pimpl::GetString() const
{
	return string(buff);
}


void LyXLex::Pimpl::printError(string const & message) const
{
	string tmpmsg = subst(message, "$$Token", GetString());
	lyxerr << "LyX: " << tmpmsg << " [around line " << lineno
	       << " of file " << MakeDisplayPath(name) << ']' << endl;
}

	
void LyXLex::Pimpl::printTable(ostream & os)
{
	os << "\nNumber of tags: " << no_items << '\n';
	for(int i= 0; i < no_items; ++i)
		os << "table[" << i
		   << "]:  tag: `" << table[i].tag
		   << "'  code:" << table[i].code << '\n';
	os.flush();
}


void LyXLex::Pimpl::pushTable(keyword_item * tab, int num)
{
	pushed_table * tmppu = new pushed_table;
	tmppu->next = pushed;
	tmppu->table_elem = table;
	tmppu->table_siz = no_items;
	pushed = tmppu;
	table = tab;
	no_items = num;
	// Check if the table is sorted and if not, sort it.
	if (table
	    && !sorted(table, table + no_items, compare_tags())) {
		lyxerr << "The table passed to LyXLex is not sorted!!\n"
		       << "Tell the developers to fix it!" << endl;
		// We sort it anyway to avoid problems.
		lyxerr << "\nUnsorted:\n";
		printTable(lyxerr);
		
		sort(table, table + no_items, compare_tags());
		lyxerr << "\nSorted:\n";
		printTable(lyxerr);
	}
}

	
void LyXLex::Pimpl::popTable()
{
	if (pushed == 0)
		lyxerr << "LyXLex error: nothing to pop!" << endl;
	
	pushed_table * tmp;
	tmp = pushed;
	table = tmp->table_elem;
	no_items = tmp->table_siz;
	tmp->table_elem = 0;
	pushed = tmp->next;
	delete tmp;
}


bool LyXLex::Pimpl::setFile(string const & filename)
{
	if (fb__.is_open())
		lyxerr << "Error in LyXLex::setFile: "
			"file or stream already set." << endl;
	fb__.open(filename.c_str(), ios::in);
	is.rdbuf(&fb__);
	name = filename;
	lineno = 0;
	return fb__.is_open() && is.good();
}

	
void LyXLex::Pimpl::setStream(istream & i)
{
	if (fb__.is_open() || is.rdbuf()->in_avail())
		lyxerr << "Error in LyXLex::setStream: "
			"file or stream already set." << endl;
	is.rdbuf(i.rdbuf());
	lineno = 0;
}


bool LyXLex::Pimpl::next(bool esc /* = false */)
{
	if (!esc) {
		unsigned char c = 0; // getc() returns an int
		char cc = 0;
		status = 0;
		while (is && !status) {
			is.get(cc);
			c = cc;
			if (c == '#') {
				// Read rest of line (fast :-)
				is.getline(buff, sizeof(buff));
				lyxerr[Debug::LYXLEX] << "Comment read: `" << c
						      << buff << "'" << endl;
				++lineno;
				continue;
			}
			
			if (c == '\"') {
				int i = -1;
				do {
					is.get(cc);
					c = cc;
					if (c != '\r')
						buff[++i] = c;
				} while (c != '\"' && c != '\n' && is &&
					 i != (LEX_MAX_BUFF - 2));
				
				if (i == (LEX_MAX_BUFF - 2)) {
					printError("Line too long");
					c = '\"'; // Pretend we got a "
					++i;
				}
				
				if (c != '\"') {
					printError("Missing quote");
					if (c == '\n')
						++lineno;
				}
				
				buff[i] = '\0';
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
				int i = 0;
				do {
					buff[i++] = c;
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != ',' && is
					 && (i != LEX_MAX_BUFF - 1) );
				if (i == LEX_MAX_BUFF - 1) {
					printError("Line too long");
				}
				buff[i] = '\0';
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
		buff[0] = '\0';
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
				int i = 0;
				do {
					if (c == '\\') {
						// escape the next char
						is.get(cc);
						c = cc;
					}
					buff[i++] = c;
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != ',' && is
					 && (i != LEX_MAX_BUFF - 1) );
				if (i == LEX_MAX_BUFF - 1) {
					printError("Line too long");
				}
				buff[i] = '\0';
				status = LEX_TOKEN;
				continue;
			}
			
			if (c == '#') {
				// Read rest of line (fast :-)
				is.getline(buff, sizeof(buff));
				lyxerr[Debug::LYXLEX] << "Comment read: `" << c
						      << buff << "'" << endl;
				++lineno;
				continue;
			}
			
			// string
			if (c == '\"') {
				int i = -1;
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
						escaped = true;
					}
					buff[++i] = c;
					
					if (!escaped && c == '\"') break;
				} while (c != '\n' && is &&
					 i != (LEX_MAX_BUFF - 2));
				
				if (i == (LEX_MAX_BUFF - 2)) {
					printError("Line too long");
					c = '\"'; // Pretend we got a "
					++i;
				}
				
				if (c != '\"') {
					printError("Missing quote");
					if (c == '\n')
						++lineno;
				}
				
				buff[i] = '\0';
				status = LEX_DATA;
				break; 
			}
			
			if (c > ' ' && is) {
				int i = 0;
				do {
					if (c == '\\') {
						// escape the next char
						is.get(cc);
						c = cc;
						//escaped = true;
					}
					buff[i++] = c;
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != ',' && is
					 && (i != LEX_MAX_BUFF-1) );
				if (i == LEX_MAX_BUFF-1) {
					printError("Line too long");
				}
				buff[i] = '\0';
				status = LEX_TOKEN;
			}
			// new line
			if (c == '\n')
				++lineno;
		}
		
		if (status) return true;
		
		status = is.eof() ? LEX_FEOF : LEX_UNDEF;
		buff[0] = '\0';
		return false;
	}
}

	///
int LyXLex::Pimpl::search_kw(char const * const tag) const
{
	keyword_item * res =
		lower_bound(table, table + no_items,
			    tag, compare_tags());
	if (res != table + no_items
	    && !compare_no_case(res->tag, tag))
		return res->code;
	return LEX_UNDEF;
}


int LyXLex::Pimpl::lex()
{
	//NOTE: possible bug.
	if (next() && status == LEX_TOKEN)
		return search_kw(buff);
	else
		return status;
}

	
bool LyXLex::Pimpl::EatLine()
{
	int i = 0;
	unsigned char c = '\0';
	char cc = 0;
	while(is && c != '\n' && i != (LEX_MAX_BUFF - 1)) {
		is.get(cc);
		c = cc;
		lyxerr[Debug::LYXLEX] << "LyXLex::EatLine read char: `"
				      << c << "'" << endl;
		if (c != '\r')
			buff[i++] = c;
	}
	if (i == (LEX_MAX_BUFF - 1) && c != '\n') {
		printError("Line too long");
		c = '\n'; // Pretend we had an end of line
		--lineno; // but don't increase line counter (netto effect)
		++i; // and preserve last character read.
	}
	if (c == '\n') {
		++lineno;
		buff[--i] = '\0'; // i can never be 0 here, so no danger
		status = LEX_DATA;
		return true;
	} else {
		buff[i] = '\0';
		return false;
	}
}


bool LyXLex::Pimpl::nextToken()
{
	status = 0;
	while (is && !status) {
		unsigned char c = 0;
		char cc = 0;
		is.get(cc);
		c = cc;
		if (c >= ' ' && is) {
			int i = 0;
			if (c == '\\') { // first char == '\\'
				do {
					buff[i++] = c;
					is.get(cc);
					c = cc;
				} while (c > ' ' && c != '\\' && is
					 && i != (LEX_MAX_BUFF-1));
			} else {
				do {
					buff[i++] = c;
					is.get(cc);
					c = cc;
				} while (c >= ' ' && c != '\\' && is
					 && i != (LEX_MAX_BUFF-1));
			}
			
			if (i == (LEX_MAX_BUFF - 1)) {
				printError("Line too long");
			}
			
			if (c == '\\') is.putback(c); // put it back
			buff[i] = '\0';
			status = LEX_TOKEN;
		}
		
		if (c == '\n')
			++lineno;
		
	}
	if (status)  return true;
	
	status = is.eof() ? LEX_FEOF: LEX_UNDEF;
	buff[0] = '\0';
	return false;
}
