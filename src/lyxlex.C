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
#include "error.h"
#include "support/filetools.h"

LyXLex::LyXLex(keyword_item * tab, int num)
	: table(tab), no_items(num)
{
	file = 0;
	owns_file = false;
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
		lyxerr.print("LyXLex error: nothing to pop!");

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
	lyxerr.print(string("\nNumber of tags: ") + tostr(no_items));
	for(int i=0; i<no_items; i++)
		lyxerr.print(string("table[")+ tostr(i) +
                              "]:  tag: `" + table[i].tag +
                              "'  code:" + tostr(table[i].code));
	lyxerr.print(string());
}


void LyXLex::printError(string const & message)
{
	string tmpmsg = subst(message, "$$Token", GetString());
	lyxerr.print("LyX: " + tmpmsg + " [around line " + tostr(lineno) + " of file "
		      + MakeDisplayPath(name) + ']');
}


bool LyXLex::setFile(string const & filename)
{
        if (file) 
		lyxerr.print("Error in LyXLex::setFile: file already set.");
	file = fopen(filename.c_str(), "r");
	name = filename;
	owns_file = true;
	lineno = 0;
	return (file ? true : false);
}


void LyXLex::setFile(FILE * f)
{
        if (file) 
		lyxerr.print("Error in LyXLex::setFile: file already set.");
	file = f;
	owns_file = false;
	lineno = 0; // this is bogus if the file already has been read from
}


int LyXLex::lex()
{
	//NOTE: possible bug.
   if (next() && status==LEX_TOKEN)
       return search_kw(buff);
   else
       return status;
}


int LyXLex::GetInteger()
{
   if (buff[0]>' ')   
       return atoi(buff);
   else {
	printError("Bad integer `$$Token'");
	return -1;
   }
}


float LyXLex::GetFloat()
{
   if (buff[0]>' ')   
       return (float)strtod(buff, (char**)0);
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
		
		lyxerr.debug("LongString: `"+GetString()+'\'', Error::LEX_PARSER);

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
				lyxerr.debug("Prefix = `"+prefix+'\'',
					      Error::LEX_PARSER); 
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


bool LyXLex::GetBool()
{
   if (compare(buff, "true") == 0)
	return true;
   else if (compare(buff, "false") != 0)
	printError("Bad boolean `$$Token'. Use \"false\" or \"true\"");
   return false;
}


bool LyXLex::EatLine()
{
	int i=0;
	int c = '\0'; // getc() returns an int

	while (!feof(file) && c!='\n' && i!=(LEX_MAX_BUFF-1)) {
		c = getc(file);
		if (c != '\r')
			buff[i++] = c;
	}
	if (i==(LEX_MAX_BUFF-1) && c !='\n') {
   		printError("Line too long");
		c = '\n'; // Pretend we had an end of line
		--lineno; // but don't increase line counter (netto effect)
		++i; // and preserve last character read.
	}
	if (c=='\n') {
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
	int m, k=0 , l= 0, r=no_items;

	while (l < r) {
		m = (l+r)/2;

		if (lyxerr.debugging(Error::LEX_PARSER)) {
			string my_l;
			my_l+="LyXLex::search_kw: elem " ;
			my_l+= m; 
			my_l+=" tag "; 
			my_l+=table[m].tag;
			my_l+=" search tag ";
			my_l+= tag;
			lyxerr.print(my_l);
		}

		if (table[m].tag)
			k = compare_no_case(table[m].tag, tag);
		if (k==0)
			return table[m].code;
		else
			if (k<0) l = m+1; else r = m;
	}
	return -1;
}


bool LyXLex::next(bool esc)
{

	if (!esc) {
		int c; // getc() returns an int
		int i;
		
		
		status = 0;
		while (!feof(file) && !status) { 
			c = getc(file);
			if (c=='#') {
				// Read rest of line (fast :-)
				fgets(buff, sizeof(buff), file);
				++lineno;
				continue;
			}
			
			if (c=='\"') {
				i = -1;
				do {
					c = getc(file);
					if (c != '\r')
						buff[++i] = c;
				} while (c!='\"' && c!='\n' && !feof(file) &&
					 i!=(LEX_MAX_BUFF-2));
				
				if (i==(LEX_MAX_BUFF-2)) {
					printError("Line too long");
					c = '\"'; // Pretend we got a "
					++i;
				}
				
				if (c!='\"') {
					printError("Missing quote");
					if (c=='\n')
						++lineno;
				}
				
				buff[i] = '\0';
				status = LEX_DATA;
				break; 
			}
			
			if (c==',')
				continue;              /* Skip ','s */
			
			if (c > ' ' && !feof(file))  {
				i = 0;
				do {
					buff[i++] = c;
					c = getc(file);
				} while (c > ' ' && c != ',' && !feof(file) &&
					 (i != LEX_MAX_BUFF-1) );
				if (i == LEX_MAX_BUFF-1) {
					printError("Line too long");
				}
				buff[i] = '\0';
				status = LEX_TOKEN;
			}
			
			if (c== '\r' && !feof(file)) {
				// The Windows support has lead to the
				// possibility of "\r\n" at the end of
				// a line.  This will stop LyX choking
				// when it expected to find a '\n'
				c = getc(file);
			}

			if (c=='\n')
				++lineno;
			
		}
		if (status) return true;
		
		status = (feof(file)) ? LEX_FEOF: LEX_UNDEF;
		buff[0] = '\0';
		return false;
	} else {
		int c; // getc() returns an int
		int i;
		
		
		status = 0;
		while (!feof(file) && !status) { 
			c = getc(file);

			// skip ','s
			if (c==',') continue;
			
			if (c=='\\') {
				// escape
				i = 0;
				do {
					if (c == '\\') {
						// escape the next char
						c = getc(file);
					}
					buff[i++] = c;
					c = getc(file);
				} while (c > ' ' && c != ',' && !feof(file) &&
					 (i != LEX_MAX_BUFF-1) );
				if (i == LEX_MAX_BUFF-1) {
					printError("Line too long");
				}
				buff[i] = '\0';
				status = LEX_TOKEN;
				continue;
			}
			
			if (c=='#') {
				// Read rest of line (fast :-)
				fgets(buff, sizeof(buff), file);
				++lineno;
				continue;
			}

			// string
			if (c=='\"') {
				i = -1;
				bool escaped = false;
				do {
					escaped = false;
					c = getc(file);
					if (c == '\r') continue;
					if (c == '\\') {
						// escape the next char
						c = getc(file);
						escaped = true;
					}
					buff[++i] = c;
				
					if (!escaped && c == '\"') break;
				} while (c!='\n' && !feof(file) &&
					 i!=(LEX_MAX_BUFF-2));
				
				if (i==(LEX_MAX_BUFF-2)) {
					printError("Line too long");
					c = '\"'; // Pretend we got a "
					++i;
				}
				
				if (c!='\"') {
					printError("Missing quote");
					if (c=='\n')
						++lineno;
				}
				
				buff[i] = '\0';
				status = LEX_DATA;
				break; 
			}
			
			if (c > ' ' && !feof(file))  {
				i = 0;
				do {
					if (c == '\\') {
						// escape the next char
						c = getc(file);
						//escaped = true;
					}
					buff[i++] = c;
					c = getc(file);
				} while (c > ' ' && c != ',' && !feof(file) &&
					 (i != LEX_MAX_BUFF-1) );
				if (i == LEX_MAX_BUFF-1) {
					printError("Line too long");
				}
				buff[i] = '\0';
				status = LEX_TOKEN;
			}

			// new line
			if (c=='\n')
				++lineno;
		}
		
		if (status) return true;
		
		status = (feof(file)) ? LEX_FEOF: LEX_UNDEF;
		buff[0] = '\0';
		return false;	
	}
}


bool LyXLex::nextToken()
{
	int c; // getc() returns an int
	int i;
        
        status = 0;
	while (!feof(file) && !status) { 
		c = getc(file);
	   
		if (c >= ' ' && !feof(file))  {
			i = 0;
			if (c == '\\') { // first char == '\\'
				do {
					buff[i++] = c;
					c = getc(file);
				} while (c > ' ' && c != '\\' && !feof(file) &&
					 i != (LEX_MAX_BUFF-1));
			} else {
				do {
					buff[i++] = c;
					c = getc(file);
				} while (c >= ' ' && c != '\\' && !feof(file)
					 && i != (LEX_MAX_BUFF-1));
			}

			if (i == (LEX_MAX_BUFF-1)) {
				printError("Line too long");
			}

			if (c == '\\') ungetc(c,file); // put it back
			buff[i] = '\0';
		        status = LEX_TOKEN;
		}
		  
		if (c=='\n')
			++lineno;
	
	}
        if (status)  return true;
        
        status = (feof(file)) ? LEX_FEOF: LEX_UNDEF;
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
       for (i = 0; str[i][0] && compare(str[i], buff); i++);
       if (!str[i][0]) {
           if (print_error)
               printError("Unknown argument `$$Token'");
           i = -1;
       }
   }
   return i;
}
