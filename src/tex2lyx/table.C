/**
 * \file table.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#include <config.h>

#include "tex2lyx.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

using std::cerr;
using std::endl;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;

#include "mathed/math_gridinfo.h"

// filled in preamble.C
std::map<char, int> special_columns;


namespace {

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
	p.skip_spaces();
	while (p.good()) {
		if (p.next_token().cs() == "hline") {
			p.get_token();
			os << "\\hline";
		} else if (p.next_token().cs() == "cline") {
			p.get_token();
			os << "\\cline{" << p.verbatim_item() << "}";
		} else
			break;
		p.skip_spaces();
	};
	//cerr << "read_hlines(), read: '" << os.str() << "'\n";
	//cerr << "read_hlines(), next token: " << p.next_token() << "\n";
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


void handle_colalign(Parser & p, vector<ColInfo> & colinfo)
{
	if (p.get_token().cat() != catBegin)
		cerr << "wrong syntax for table column alignment. '{' expected\n";

	char nextalign = 'b';
	bool leftline = false;
	for (Token t=p.get_token(); p.good() && t.cat() != catEnd; t = p.get_token()){
#ifdef FILEDEBUG
		cerr << "t: " << t << "  c: '" << t.character() << "'\n";
#endif

		switch (t.character()) {
			case 'c':
			case 'l':
			case 'r': {
				ColInfo ci;
				ci.align = t.character();
				if (colinfo.size() && colinfo.back().rightline > 1) {
					ci.leftline = true;
					--colinfo.back().rightline;
				}
				colinfo.push_back(ci);
				break;
			}
			case 'p':
				colinfo.push_back(ColInfo());
				colinfo.back().align = nextalign;
				colinfo.back().width = p.verbatim_item();
				nextalign = 'b';
				break;
			case '|':
				if (colinfo.empty())
					leftline = true;
				else
					++colinfo.back().rightline;
				break;
			case '>': {
				string s = p.verbatim_item();
				if (s == "\\raggedleft ")
					nextalign = 'l';
				else if (s == "\\raggedright ")
					nextalign = 'r';
				else
					cerr << "unknown '>' column '" << s << "'\n";
				break;
			}
			default:
				if (special_columns.find(t.character()) != special_columns.end()) {
					ColInfo ci;
					ci.align = 'c';
					ci.special += t.character();
					int const nargs = special_columns[t.character()];
					for (int i = 0; i < nargs; ++i)
						ci.special += "{" + p.verbatim_item() + "}";
					//cerr << "handling special column '" << t << "' " << nargs
					//	<< "  '" << ci.special << "'\n";
					colinfo.push_back(ci);
				} else {
					cerr << "ignoring special separator '" << t << "'\n";
				}
				break;
			}
	}
	if (colinfo.size() && leftline)
		colinfo[0].leftline = true;
}


} // anonymous namespace


void parse_table(Parser & p, ostream & os, unsigned flags)
{
	string hlines;

	while (p.good()) {
		Token const & t = p.get_token();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
#endif

		//
		// cat codes
		//
		if (t.cat() == catMath) {
			// we are inside some text mode thingy, so opening new math is allowed
			Token const & n = p.get_token();
			if (n.cat() == catMath) {
				// TeX's $$...$$ syntax for displayed math
				os << "\\[";
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << "\\]";
				p.get_token(); // skip the second '$' token
			} else {
				// simple $...$  stuff
				p.putback();
				os << '$';
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << '$';
			}
		}

		else if (t.cat() == catSpace || t.cat() == catNewline)
				os << t.cs();

		else if (t.cat() == catLetter ||
			       t.cat() == catSuper ||
			       t.cat() == catSub ||
			       t.cat() == catOther ||
			       t.cat() == catActive ||
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
			p.skip_spaces();
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
			hlines += "\\cline{" + p.verbatim_item() + '}';

		else if (t.cat() == catComment)
			os << t.asInput();

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
			active_environments.push_back(name);
			os << "\\begin{" << name << '}';
			if (is_math_env(name)) {
				parse_math(p, os, FLAG_END, MATH_MODE);
			} else {
				parse_table(p, os, FLAG_END);
			}
			os << "\\end{" << name << '}';
			active_environments.pop_back();
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != active_environment())
					p.error("\\end{" + name + "} does not match \\begin{"
						+ active_environment() + "}");
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else
			os << t.asInput();
	}
}


void handle_hline_above(RowInfo & ri, vector<CellInfo> & ci)
{
	ri.topline = true;
	for (size_t col = 0; col < ci.size(); ++col)
		ci[col].topline = true;
}


void handle_hline_below(RowInfo & ri, vector<CellInfo> & ci)
{
	ri.bottomline = true;
	for (size_t col = 0; col < ci.size(); ++col)
		ci[col].bottomline = true;
}


void handle_tabular(Parser & p, ostream & os,
		    Context & context)
{
	string posopts = p.getOpt();
	if (posopts.size())
		cerr << "vertical tabular positioning '" << posopts << "' ignored\n";

	vector<ColInfo>            colinfo;

	// handle column formatting
	handle_colalign(p, colinfo);

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
		cellinfo[row].resize(colinfo.size());

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
				Token t = p1.get_token();
				//cerr << "read token: " << t << "\n";
				if (t.cs() == "hline") {
					if (i == 0) {
						if (rowinfo[row].topline) {
							if (row > 0) // extra bottomline above
								handle_hline_below(rowinfo[row - 1], cellinfo[row - 1]);
							else
								cerr << "dropping extra hline\n";
							//cerr << "below row: " << row-1 << endl;
						} else {
							handle_hline_above(rowinfo[row], cellinfo[row]);
							//cerr << "above row: " << row << endl;
						}
					} else {
						//cerr << "below row: " << row << endl;
						handle_hline_below(rowinfo[row], cellinfo[row]);
					}
				} else if (t.cs() == "cline") {
					string arg = p1.verbatim_item();
					//cerr << "read cline arg: '" << arg << "'\n";
					vector<string> t;
					split(arg, t, '-');
					t.resize(2);
					size_t from = string2int(t[0]) - 1;
					size_t to = string2int(t[1]);
					for (size_t col = from; col < to; ++col) {
						//cerr << "row: " << row << " col: " << col << " i: " << i << endl;
						if (i == 0) {
							rowinfo[row].topline = true;
							cellinfo[row][col].topline = true;
						} else {
							rowinfo[row].bottomline = true;
							cellinfo[row][col].bottomline = true;
						}
					}
				} else {
					cerr << "unexpected line token: " << t << endl;
				}
			}
		}

		// split into cells
		vector<string> cells;
		split(lines[row], cells, TAB);
		// Has the last multicolumn cell a rightline?
		bool last_rightline = false;
		for (size_t col = 0, cell = 0;
				cell < cells.size() && col < colinfo.size(); ++col, ++cell) {
			//cerr << "cell content: '" << cells[cell] << "'\n";
			Parser p(cells[cell]);
			p.skip_spaces();
			//cells[cell] << "'\n";
			if (p.next_token().cs() == "multicolumn") {
				// how many cells?
				p.get_token();
				size_t const ncells = string2int(p.verbatim_item());

				// special cell properties alignment
				vector<ColInfo> t;
				handle_colalign(p, t);
				cellinfo[row][col].multi     = 1;
				cellinfo[row][col].align     = t.front().align;
				ostringstream os;
				parse_text_in_inset(p, os, FLAG_ITEM, false, context);
				cellinfo[row][col].content   = os.str();

				// multicolumn cells are tricky: This
				// \multicolumn{2}{|c|}{col1-2}&
				// \multicolumn{2}{|c|}{col3-4} "\\"
				// gives | col1-2 | col3-4 | and not
				//       | col1-2 || col3-4 |
				// So:
				if (last_rightline && t.front().leftline) {
					t.front().leftline = false;
				}
				last_rightline = t.front().rightline;

				// multicolumn lines override normal cell lines
				cellinfo[row][col].leftline  = t.front().leftline;
				cellinfo[row][col].rightline = t.front().rightline;

				// add dummy cells for multicol
				for (size_t i = 0; i < ncells - 1 && col < colinfo.size(); ++i) {
					++col;
					cellinfo[row][col].multi = 2;
					cellinfo[row][col].align = 'c';
				}

				// more than one line on the right?
				if (t.front().rightline > 1)
					cellinfo[row][col + 1].leftline = true;

			} else {
				// FLAG_END is a hack, we need to read all of it
				cellinfo[row][col].leftline = colinfo[col].leftline;
				cellinfo[row][col].rightline = colinfo[col].rightline;
				cellinfo[row][col].align = colinfo[col].align;
				ostringstream os;
				parse_text_in_inset(p, os, FLAG_CELL, false, context);
				cellinfo[row][col].content   = os.str();
				last_rightline = false;
			}
		}

		//cerr << "//  handle almost empty last row what we have\n";
		// handle almost empty last row
		if (row && lines[row].empty() && row + 1 == rowinfo.size()) {
			//cerr << "remove empty last line\n";
			if (rowinfo[row].topline)
				rowinfo[row - 1].bottomline = true;
			for (size_t col = 0; col < colinfo.size(); ++col)
				if (cellinfo[row][col].topline)
					cellinfo[row - 1][col].bottomline = true;
			rowinfo.pop_back();
		}

	}

	//cerr << "// output what we have\n";
	// output what we have
	os << "\n<lyxtabular version=\"3\" rows=\"" << rowinfo.size()
		 << "\" columns=\"" << colinfo.size() << "\">\n"
		 << "<features>\n";

	//cerr << "// after header\n";
	for (size_t col = 0; col < colinfo.size(); ++col) {
		os << "<column alignment=\""
		   << verbose_align(colinfo[col].align) << "\"";
		os << " valignment=\"top\"";
		if (colinfo[col].leftline)
			os << " leftline=\"true\"";
		if (colinfo[col].rightline)
			os << " rightline=\"true\"";
		if (colinfo[col].width.size())
			os << " width=\"" << colinfo[col].width << "\"";
		if (colinfo[col].special.size())
			os << " special=\"" << colinfo[col].special << "\"";
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
			os << " alignment=\"" << verbose_align(cell.align)
			   << "\""
			   << " valignment=\"top\"";
			if (cell.topline)
				os << " topline=\"true\"";
			if (cell.bottomline)
				os << " bottomline=\"true\"";
			if (cell.leftline)
				os << " leftline=\"true\"";
			if (cell.rightline)
				os << " rightline=\"true\"";
			//cerr << "\nrow: " << row << " col: " << col;
			//if (cell.topline)
			//	cerr << " topline=\"true\"";
			//if (cell.bottomline)
			//	cerr << " bottomline=\"true\"";
			os << " usebox=\"none\""
			   << ">"
			   << "\n\\begin_inset Text\n"
			   << cell.content
			   << "\n\\end_inset \n"
			   << "</cell>\n";
		}
		os << "</row>\n";
	}

	os << "</lyxtabular>\n";
}




// }])
