/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "Lsstream.h"
#include "tex2lyx.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;


namespace {

struct ColInfo
{
	ColInfo() : rightline(false) {}
	string align;      // column alignment
	string width;      // column width
	bool   rightline;  // a line on the right?
};


struct RowInfo
{
	RowInfo() : topline(false), bottomline(false) {} 
	bool topline;     // horizontal line above
	bool bottomline;  // horizontal line below
};


struct CellInfo
{
	CellInfo()
		: multi(0), leftline(false), rightline(false),
	   topline(false), bottomline(false)
	{}

	string content;    // cell content
	int multi;         // multicolumn flag
	string align;      // cell alignment
	bool leftline;     // do we have a line on the left?
	bool rightline;	   // do we have a line on the right?
	bool topline;	     // do we have a line above?
	bool bottomline;   // do we have a line below?
};


int string2int(string const & s, int deflt = 0)
{
	istringstream is(s);
	int i = deflt;
	is >> i;
	return i;
}


string read_hlines(Parser & p)
{
	ostringstream os;
	p.skipSpaces();
	while (p.good()) {
		if (p.nextToken().cs() == "hline") {
			p.getToken();
			os << "\\hline";
		} else if (p.nextToken().cs() == "cline") {
			p.getToken();
			os << "\\cline{" << p.verbatimItem() << "}";
		} else
			break;
		p.skipSpaces();
	};
	//cerr << "read_hlines(), read: '" << os.str() << "'\n";
	//cerr << "read_hlines(), next token: " << p.nextToken() << "\n";
	return os.str();
}



/* rather brutish way to code table structure in a string:

  \begin{tabular}{ccc}
    1 & 2 & 3\\ \hline
    \multicolumn{2}{c}{4} & 5 //
    6 & 7 \\
  \end{tabular}

 gets "translated" to:

  1 TAB 2 TAB 3 LINE
  \hline HLINE  TAB 5 LINE
  5 TAB 7 LINE
*/

char const TAB   = '\001';
char const LINE  = '\002';
char const HLINE = '\004';


bool handle_colalign(Parser & p, vector<ColInfo> & colinfo)
{
	if (p.getToken().cat() != catBegin)
		cerr << "wrong syntax for table column alignment. '{' expected\n";

	string nextalign = "block";
	bool leftline = false;
	for (Token t = p.getToken(); p.good() && t.cat() != catEnd; t = p.getToken()){
#ifdef FILEDEBUG
		cerr << "t: " << t << "  c: '" << t.character() << "'\n";
#endif

		switch (t.character()) {
			case 'c':
				colinfo.push_back(ColInfo());
				colinfo.back().align = "center";
				break;
			case 'l':
				colinfo.push_back(ColInfo());
				colinfo.back().align = "left";
				break;
			case 'r':
				colinfo.push_back(ColInfo());
				colinfo.back().align = "right";
				break;
			case 'p':
				colinfo.push_back(ColInfo());
				colinfo.back().align = nextalign;
				colinfo.back().width = p.verbatimItem();
				nextalign = "block";
				break;
			case '|':
				if (colinfo.empty())
					leftline = true;
				else
					colinfo.back().rightline = true;
				break;
			case '>': {
				string s = p.verbatimItem();
				if (s == "\\raggedleft ")
					nextalign = "left";
				else if (s == "\\raggedright ")
					nextalign = "right";
				else
					cerr << "unknown '>' column '" << s << "'\n";
				break;
			}
			default:
				cerr << "ignoring special separator '" << t << "'\n";
				break;
			}
	}
	return leftline;
}


} // anonymous namespace


void parse_table(Parser & p, ostream & os, unsigned flags)
{
	string hlines;

	while (p.good()) {
		Token const & t = p.getToken();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
#endif

		//
		// cat codes
		//
		if (t.cat() == catMath) {
				// we are inside some text mode thingy, so opening new math is allowed
			Token const & n = p.getToken();
			if (n.cat() == catMath) {
				// TeX's $$...$$ syntax for displayed math
				os << "\\[";
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << "\\]";
				p.getToken(); // skip the second '$' token
			} else {
				// simple $...$  stuff
				p.putback();
				os << '$';
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << '$';
			}
		}

		else if (t.cat() == catLetter ||
			       t.cat() == catSpace ||
			       t.cat() == catSuper ||
			       t.cat() == catSub ||
			       t.cat() == catOther ||
			       t.cat() == catActive ||
			       t.cat() == catNewline ||
			       t.cat() == catParameter)
			os << t.character();

		else if (t.cat() == catBegin) {
			os << '{';
			parse_table(p, os, FLAG_BRACE_LAST);
			os << '}';
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			cerr << "unexpected '}'\n";
		}

		else if (t.cat() == catAlign) {
			os << TAB;
		}

		else if (t.cs() == "tabularnewline" || t.cs() == "\\") {
			// stuff before the line break
			// and look ahead for stuff after the line break
			os << HLINE << hlines << HLINE << LINE << read_hlines(p) << HLINE;
			hlines.erase();
		}

		else if (t.cs() == "hline")
			hlines += "\\hline";

		else if (t.cs() == "cline")
			hlines += "\\cline{" + p.verbatimItem() + '}';

		else if (t.cat() == catComment)
			handle_comment(p);

		else if (t.cs() == "(") {
			os << "\\(";
			parse_math(p, os, FLAG_SIMPLE2, MATH_MODE);
			os << "\\)";
		}

		else if (t.cs() == "[") {
			os << "\\[";
			parse_math(p, os, FLAG_EQUATION, MATH_MODE);
			os << "\\]";
		}

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			active_environments_push(name);
			parse_table(p, os, FLAG_END);
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != curr_env())
					p.error("\\end{" + name + "} does not match \\begin{"
						+ curr_env() + "}");
				active_environments_pop();
				return;
			}
			p.error("found 'end' unexpectedly");
		}
	}
}


void handle_tabular(Parser & p, ostream & os)
{
	string posopts = p.getOpt();
	if (posopts.size())
		cerr << "vertical tabular positioning '" << posopts << "' ignored\n";

	vector<ColInfo>            colinfo;

	// handle column formatting
	bool leftline = handle_colalign(p, colinfo);

	// handle initial hlines

	// first scan of cells
	// use table mode to keep it minimal-invasive
	// not exactly what's TeX doing...
	vector<string> lines;
	ostringstream ss;
	ss << read_hlines(p) << HLINE; // handle initial hlines
	parse_table(p, ss, FLAG_END);
	split(ss.str(), lines, LINE);

	vector< vector<CellInfo> > cellinfo(lines.size());
	vector<RowInfo> rowinfo(lines.size());
	
	// split into rows
	//cerr << "// split into rows\n";
	for (size_t row = 0; row < rowinfo.size(); ++row) {

		// init row
		vector<CellInfo> & cellinfos = cellinfo[row];
		cellinfos.resize(colinfo.size());

		// split row	
		vector<string> dummy;
		//cerr << "\n########### LINE: " << lines[row] << "########\n";
		split(lines[row], dummy, HLINE);

		// handle horizontal line fragments
		if (dummy.size() != 3) {
			if (dummy.size() != 1)
				cerr << "unexpected dummy size: " << dummy.size()
					<< " content: " << lines[row] << "\n";
			dummy.resize(3);
		}
		lines[row] = dummy[1];

		//cerr << "line: " << row << " above 0: " << dummy[0] << "\n";
		//cerr << "line: " << row << " below 2: " << dummy[2] <<  "\n";
		//cerr << "line: " << row << " cells 1: " << dummy[1] <<  "\n";

		for (int i = 0; i <= 2; i += 2) {	
			//cerr << "   reading from line string '" << dummy[i] << "'\n";
			Parser p1(dummy[i]);
			while (p1.good()) {
				Token t = p1.getToken();
				//cerr << "read token: " << t << "\n";
				if (t.cs() == "hline") {
					if (i == 0) {
						rowinfo[row].topline = true;
						for (size_t col = 0; col < colinfo.size(); ++col)
							cellinfos[col].topline = true;
					} else {
						rowinfo[row].bottomline = true;
						for (size_t col = 0; col < colinfo.size(); ++col)
							cellinfos[col].bottomline = true;
					}
				} else if (t.cs() == "cline") {
					string arg = p1.verbatimItem();
					//cerr << "read cline arg: '" << arg << "'\n";
					vector<string> t;
					split(arg, t, '-');
					t.resize(2);
					size_t from = string2int(t[0]);
					size_t to = string2int(t[1]);
					for (size_t col = from; col < to; ++col) {
						if (i == 0) 
							cellinfos[col].topline = true;
						else	
							cellinfos[col].bottomline = true;
					}
				} else {
					cerr << "unexpected line token: " << t << endl;
				}
			}
		}

		// split into cells
		vector<string> cells;
		split(lines[row], cells, TAB);
		for (size_t col = 0, cell = 0; cell < cells.size() && col < colinfo.size(); ++col, ++cell) {
			//cerr << "cell content: " << cells[cell] << "\n";
			Parser p(cells[cell]);
			p.skipSpaces();	
			//cerr << "handling cell: " << p.nextToken().cs() << " '" <<
			//cells[cell] << "'\n";
			if (p.nextToken().cs() == "multicolumn") {
				// how many cells?
				p.getToken();
				size_t ncells = string2int(p.verbatimItem());

				// special cell properties alignment	
				vector<ColInfo> t;
				bool leftline = handle_colalign(p, t);
				CellInfo & ci = cellinfos[col];
				ci.multi     = 1;
				ci.align     = t.front().align;
				ci.content   = parse_text(p, FLAG_ITEM, false);
				ci.leftline  = leftline;
				ci.rightline = t.front().rightline;

				// add dummy cells for multicol
				for (size_t i = 0; i < ncells - 1 && col < colinfo.size(); ++i) {
					++col;
					cellinfos[col].multi = 2;
					cellinfos[col].align = "center";
				}
			} else {
				cellinfos[col].content = parse_text(p, FLAG_ITEM, false);
			}
		}

		cellinfo.push_back(cellinfos);

		//cerr << "//  handle almost empty last row what we have\n";
		// handle almost empty last row
		if (row && lines[row].empty() && row + 1 == rowinfo.size()) {
			//cerr << "remove empty last line\n";
			if (rowinfo[row].topline);
				rowinfo[row - 1].bottomline = true;
			for (size_t col = 0; col < colinfo.size(); ++col)
				if (cellinfo[row][col].topline)
					cellinfo[row - 1][col].bottomline = true;
			rowinfo.pop_back();
		}

	}

	//cerr << "// output what we have\n";
	// output what we have
	os << "<lyxtabular version=\"3\" rows=\"" << rowinfo.size()
		 << "\" columns=\"" << colinfo.size() << "\">\n"
		 << "<features>\n";

	//cerr << "// after header\n";
	for (size_t col = 0; col < colinfo.size(); ++col) {
		os << "<column alignment=\"" << colinfo[col].align << "\"";
		if (colinfo[col].rightline)
			os << " rightline=\"true\"";
		if (col == 0 && leftline)
			os << " leftline=\"true\"";
		os << " valignment=\"top\"";
		os << " width=\"" << colinfo[col].width << "\"";
		os << ">\n";
	}
	//cerr << "// after cols\n";

	for (size_t row = 0; row < rowinfo.size(); ++row) {
		os << "<row";
		if (rowinfo[row].topline)
			os << " topline=\"true\"";
		if (rowinfo[row].bottomline)
			os << " bottomline=\"true\"";
		os << ">\n";
		for (size_t col = 0; col < colinfo.size(); ++col) {
			CellInfo const & cell = cellinfo[row][col];
			os << "<cell";
			if (cell.multi)
				os << " multicolumn=\"" << cell.multi << "\"";
			if (cell.leftline)
				os << " leftline=\"true\"";
			if (cell.rightline)
				os << " rightline=\"true\"";
			if (cell.topline)
				os << " topline=\"true\"";
			if (cell.bottomline)
				os << " bottomline=\"true\"";
			os << " alignment=\"" << cell.align << "\""
				 << " valignment=\"top\""
				 << " usebox=\"none\""
				 << ">"
			   << "\n\\begin_inset Text"
			   << "\n\n\\layout Standard\n\n"
			   << cell.content
			   << "\n\\end_inset\n\n"
			   << "</cell>\n";
		}
		os << "</row>\n";
	}
			
	os << "</lyxtabular>\n";
}




// }])
