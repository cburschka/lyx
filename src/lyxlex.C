//  Generalized simple lexical analizer.
//  It can be used for simple syntax parsers, like lyxrc,
//  texclass and others to come.   [asierra30/03/96]
//
//   (C) 1996 Lyx Team.

#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation "lyxlex.h"
#endif

#include "lyxlex.h"
#include "debug.h"
#include "support/filetools.h"

using std::ios;

LyXLex::LyXLex(keyword_item * tab, int num)
	: is(&fb__), table(tab), no_items(num)
{
	
	status = 0;
	pushed = 0;
}


void LyXLex::pushTable(keyword_item * tab, int num)
{
	pushed_table * tmppu = new pushed_table;
	tmppu->next = pushed;
	tmppu->table_elem = table;
	tmppu->table_siz = no_items;
	pushed = tmppu;
	table = tab;
	no_items = num;
}


void LyXLex::popTable()
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


void LyXLex::printTable()
{
	lyxerr << "\nNumber of tags: " << no_items << endl;
	for(int i= 0; i < no_items; ++i)
		lyxerr << "table[" << i
		       << "]:  tag: `" << table[i].tag
		       << "'  code:" << table[i].code << endl;
	lyxerr << endl;
}


void LyXLex::printError(string const & message) const
{
	string tmpmsg = subst(message, "$$Token", GetString());
	lyxerr << "LyX: " << tmpmsg << " [around line " << lineno
	       << " of file " << MakeDisplayPath(name) << ']' << endl;
}


bool LyXLex::setFile(string const & filename)
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


void LyXLex::setStream(istream & i)
{
	if (fb__.is_open() || is.rdbuf()->in_avail())
		lyxerr << "Error in LyXLex::setStream: "
			"file or stream already set." << endl;
	is.rdbuf(i.rdbuf());
	lineno = 0;
}


int LyXLex::lex()
{
	//NOTE: possible bug.
	if (next() && status == LEX_TOKEN)
		return search_kw(buff);
	else
		return status;
}


int LyXLex::GetInteger() const
{
	if (buff[0] > ' ')   
		return atoi(buff);
	else {
		printError("Bad integer `$$Token'");
		return -1;
	}
}


float LyXLex::GetFloat() const
{
   if (buff[0] > ' ')   
       return atof(buff);
   else {
	printError("Bad float `$$Token'");
	return -1;
   }
}


string LyXLex::GetString() const
{
	return string(buff);
}


// I would prefer to give a tag number instead of an explicit token
// here, but it is not possible because Buffer::readLyXformat2 uses
// explicit tokens (JMarc) 
string LyXLex::getLongString(string const & endtoken)
{
	string str, prefix;
	bool firstline = true;

	while (IsOK()) {
		if (!EatLine())
			// blank line in the file being read
			continue;
		
		string const token = frontStrip(strip(GetString()), " \t");
		
		lyxerr[Debug::PARSER] << "LongString: `"
				      << GetString() << '\'' << endl;

		// We do a case independent comparison, like search_kw
		// does.
                if (compare_no_case(token, endtoken) != 0) {
			string tmpstr = GetString();
			if (firstline) {
				unsigned int i = 0;
				while(i < tmpstr.length()
				      && tmpstr[i] == ' ') {
					++i;
					prefix += ' ';
				}
				firstline = false;
				lyxerr[Debug::PARSER] << "Prefix = `" << prefix
						      << '\'' << endl;
			} 

			if (!prefix.empty() 
			    && prefixIs(tmpstr, prefix.c_str())) {
				tmpstr.erase(0, prefix.length() - 1);
			}
			str += tmpstr + '\n';
                }
		else // token == endtoken
			break;
	}
	if (!IsOK())
		printError("Long string not ended by `" + endtoken + '\'');

	return str;
}


bool LyXLex::GetBool() const
{
	if (compare(buff, "true") == 0)
		return true;
	else if (compare(buff, "false") != 0)
		printError("Bad boolean `$$Token'. Use \"false\" or \"true\"");
	return false;
}


bool LyXLex::EatLine()
{
	int i = 0;
	unsigned char c = '\0';

	while(is && c != '\n' && i != (LEX_MAX_BUFF - 1)) {
		is.get(c);
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


int LyXLex::search_kw(char const * const tag) const
{
	int m, k = 0 , l = 0, r = no_items;

	while (l < r) {
		m = (l + r) / 2;

		if (lyxerr.debugging(Debug::PARSER)) {
			lyxerr << "LyXLex::search_kw: elem " << m
			       << " tag " << table[m].tag
			       << " search tag " << tag
			       << endl;
		}

		if (table[m].tag)
			k = compare_no_case(table[m].tag, tag);
		if (k == 0)
			return table[m].code;
		else
			if (k < 0) l = m + 1; else r = m;
	}
	return LEX_UNDEF;
}


bool LyXLex::next(bool esc)
{
	if (!esc) {
		unsigned char c; // getc() returns an int
		
		status = 0;
		while (is && !status) { 
			is.get(c);
			if (c == '#') {
				// Read rest of line (fast :-)
				is.get(buff, sizeof(buff));
				lyxerr[Debug::LYXLEX] << "Comment read: " << c << buff << endl;
				++lineno;
				continue;
			}
			
			if (c == '\"') {
				int i = -1;
				do {
					is.get(c);
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
#warning Verify this! (Lgb)
				//if (isalnum(static_cast<unsigned char>(c)) && is) {
				int i = 0;
				do {
					buff[i++] = c;
					is.get(c);
				} while (c > ' ' && c != ',' && is
				//} while (isalnum(static_cast<unsigned char>(c))
					 //&& c != ',' && is
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
				is.get(c);
			}

			if (c == '\n')
				++lineno;
			
		}
		if (status) return true;
		
		status = is.eof() ? LEX_FEOF: LEX_UNDEF;
		buff[0] = '\0';
		return false;
	} else {
		unsigned char c; // getc() returns an int
		
		status = 0;
		while (is && !status) {
			is.get(c);

			// skip ','s
			if (c == ',') continue;
			
			if (c == '\\') {
				// escape
				int i = 0;
				do {
					if (c == '\\') {
						// escape the next char
						is.get(c);
					}
					buff[i++] = c;
					is.get(c);
				} while (c > ' ' && c != ',' && is
				//} while (isalnum(static_cast<unsigned char>(c))
					 //&& c != ',' && is
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
				is.get(buff, sizeof(buff));
				lyxerr[Debug::LYXLEX] << "Comment read: " << c << buff << endl;
				++lineno;
				continue;
			}

			// string
			if (c == '\"') {
				int i = -1;
				bool escaped = false;
				do {
					escaped = false;
					is.get(c);
					if (c == '\r') continue;
					if (c == '\\') {
						// escape the next char
						is.get(c);
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
				//if (isalnum(static_cast<unsigned char>(c)) && is) {
				int i = 0;
				do {
					if (c == '\\') {
						// escape the next char
						is.get(c);
						//escaped = true;
					}
					buff[i++] = c;
					is.get(c);
				} while (c > ' ' && c != ',' && is
				//} while (isalnum(static_cast<unsigned char>(c))
					 //!= ',' && is
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
		
		status = is.eof() ? LEX_FEOF: LEX_UNDEF;
		buff[0] = '\0';
		return false;	
	}
}


bool LyXLex::nextToken()
{
        status = 0;
	while (is && !status) { 
		unsigned char c;
		is.get(c);
	   
		if (c >= ' ' && is) {
			//if (isprint(static_cast<unsigned char>(c)) && is) {
			int i = 0;
			if (c == '\\') { // first char == '\\'
				do {
					buff[i++] = c;
					is.get(c);
				} while (c > ' ' && c != '\\' && is
				//} while (isalnum(static_cast<unsigned char>(c))
				//	 && c != '\\' && is
					 && i != (LEX_MAX_BUFF-1));
			} else {
				do {
					buff[i++] = c;
					is.get(c);
				} while (c >= ' ' && c != '\\' && is
				//} while (isprint(static_cast<unsigned char>(c))
					 // && c != '\\' && is
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


int LyXLex::FindToken(char const * str[])
{  
   int i = -1;
   
   if (next()) {
      if (compare(buff, "default")) {
	 for (i = 0; str[i][0] && compare(str[i], buff); ++i);
	 if (!str[i][0]) {
	    printError("Unknown argument `$$Token'");
	    i = -1;
	 }
      }  
   } else
     printError("file ended while scanning string token");
   return i;
}


int LyXLex::CheckToken(char const * str[], int print_error)
{  
   int i = -1;
   
   if (compare(buff, "default")) {
       for (i = 0; str[i][0] && compare(str[i], buff); ++i);
       if (!str[i][0]) {
           if (print_error)
               printError("Unknown argument `$$Token'");
           i = -1;
       }
   }
   return i;
}
