//  Generalized simple lexical analizer.
//  It can be used for simple syntax parsers, like lyxrc,
//  texclass and others to come.   [asierra30/03/96]
//
//   (C) 1996 Lyx Team.

#include <config.h>
//#include "definitions.h"

#include <stdlib.h>
#include <string.h>

#ifdef __GNUG__
#pragma implementation "lyxlex.h"
#endif

#include "lyxlex.h"
#include "error.h"
#include "filetools.h"

// 	$Id: lyxlex.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: lyxlex.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $";
#endif /* lint */


LyXLex::LyXLex(keyword_item* tab, int num)
	: table(tab), no_items(num)
{
	file = NULL;
	owns_file = false;
	status = 0;
	pushed = NULL;
}


void LyXLex::pushTable(keyword_item* tab, int num)
{
	pushed_table *tmppu = new pushed_table;
	tmppu->next = pushed;
	tmppu->table_elem = table;
	tmppu->table_siz = no_items;
	pushed = tmppu;
	table = tab;
	no_items = num;
}


void LyXLex::popTable()
{
	if (pushed == NULL)
		lyxerr.print("LyXLex error: nothing to pop!");

	pushed_table *tmp;
	tmp = pushed;
	table = tmp->table_elem;
	no_items = tmp->table_siz;
	tmp->table_elem = NULL;
	pushed = tmp->next;
	delete tmp;
}


void LyXLex::printTable()
{
	lyxerr.print(LString("\nNumber of tags: ")+no_items);
	for(int i=0; i<no_items; i++)
		lyxerr.print(LString("table[")+i+
                              "]:  tag: `"+table[i].tag+
                              "'  code:"+(long)table[i].code);
	lyxerr.print(LString());
}


void LyXLex::printError(LString const & message)
{
	LString tmpmsg = message;
	tmpmsg.subst("$$Token",GetString());
	lyxerr.print("LyX: "+tmpmsg+" [around line "+lineno+" of file "
		      +MakeDisplayPath(name)+']');
}


bool LyXLex::setFile(LString const & filename)
{
        if (file) 
		lyxerr.print("Error in LyXLex::setFile: file already set.");
	file = fopen(filename.c_str(), "r");
	name = filename;
	owns_file = true;
	lineno = 0;
	return (file ? true : false);
}


void LyXLex::setFile(FILE *f)
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
       return (float)strtod(buff, (char**)NULL);
   else {
	printError("Bad float `$$Token'");
	return -1;
   }
}


LString LyXLex::GetString() const
{
	return LString(buff);
}


// I would prefer to give a tag number instead of an explicit token
// here, but it is not possible because Buffer::readLyXformat2 uses
// explicit tokens (JMarc) 
LString LyXLex::getLongString(LString const &endtoken)
{
	LString str, prefix;
	bool firstline = true;

	while (IsOK()) {
		if (!EatLine())
			// blank line in the file being read
			continue;
		
		LString const token = GetString().strip().frontStrip();
		
		lyxerr.debug("LongString: `"+GetString()+'\'', Error::LEX_PARSER);

		// We do a case independent comparison, like search_kw
		// does.
                if (strcasecmp(token.c_str(), endtoken.c_str()) != 0) {
			LString tmpstr = GetString();
			if (firstline) {
				int i = 0;
				while(i < tmpstr.length()
				      && tmpstr[i] == ' ') {
					i++;
					prefix += ' ';
				}
				firstline = false;
				lyxerr.debug("Prefix = `"+prefix+'\'',
					      Error::LEX_PARSER); 
			} 

			if (!prefix.empty() 
			    && tmpstr.prefixIs(prefix.c_str())) {
				tmpstr.substring(prefix.length(), 
						 tmpstr.length()-1);
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
   if (strcmp(buff, "true") == 0)
	return true;
   else if (strcmp(buff, "false") != 0)
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
		lineno--; // but don't increase line counter (netto effect)
		i++; // and preserve last character read.
	}
	if (c=='\n') {
		lineno++;
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
			LString my_l;
			my_l+="LyXLex::search_kw: elem " ;
			my_l+= m; 
			my_l+=" tag "; 
			my_l+=table[m].tag;
			my_l+=" search tag ";
			my_l+= tag;
			lyxerr.print(my_l);
		}

		if (table[m].tag)
			k = strcasecmp(table[m].tag, tag);
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
				lineno++;
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
					i++;
				}
				
				if (c!='\"') {
					printError("Missing quote");
					if (c=='\n')
						lineno++;
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
			
			if (c=='\r' && !feof(file)) {
				// The Windows support has lead to the
				// possibility of "\r\n" at the end of
				// a line.  This will stop LyX choking
				// when it expected to find a '\n'
				c = getc(file);
			}

			if (c=='\n')
				lineno++;
			
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
				lineno++;
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
					i++;
				}
				
				if (c!='\"') {
					printError("Missing quote");
					if (c=='\n')
						lineno++;
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
				lineno++;
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
			lineno++;
	
	}
        if (status)  return true;
        
        status = (feof(file)) ? LEX_FEOF: LEX_UNDEF;
        buff[0] = '\0';
        return false;
}


int LyXLex::FindToken(char const* string[])
{  
   int i = -1;
   
   if (next()) {
      if (strcmp(buff, "default")) {
	 for (i=0; string[i][0] && strcmp(string[i], buff); i++);
	 if (!string[i][0]) {
	    printError("Unknown argument `$$Token'");
	    i = -1;
	 }
      }  
   } else
     printError("file ended while scanning string token");
   return i;
}


int LyXLex::CheckToken(char const* string[], int print_error)
{  
   int i = -1;
   
   if (strcmp(buff, "default")) {
       for (i=0; string[i][0] && strcmp(string[i], buff); i++);
       if (!string[i][0]) {
           if (print_error)
               printError("Unknown argument `$$Token'");
           i = -1;
       }
   }
   return i;
}
