/**
 * \file table.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jean-Marc Lasgouttes
 * \author Georg Baum
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


// filled in preamble.C
std::map<char, int> special_columns;


namespace {

class ColInfo {
public:
	ColInfo() : align('c'), rightline(false), leftline(false) {}
	/// column alignment
	char align;
	/// column width
	string width;
	/// special column alignment
	string special;
	/// how many lines on the right?
	int rightline;
	/// a line on the left?
	bool leftline;
};


/// row type for longtables
enum LTRowType
{
	/// normal row
	LT_NORMAL,
	/// part of head
	LT_HEAD,
	/// part of head on first page
	LT_FIRSTHEAD,
	/// part of foot
	LT_FOOT,
	/// part of foot on last page
	LT_LASTFOOT
};


class RowInfo {
public:
	RowInfo() : topline(false), bottomline(false), type(LT_NORMAL),
	            newpage(false) {}
	/// horizontal line above
	bool topline;
	/// horizontal line below
	bool bottomline;
	/// These are for longtabulars only
	/// row type (head, foot, firsthead etc.)
	LTRowType type;
	/// row for a pagebreak
	bool newpage;
};


class CellInfo {
public:
	CellInfo() : multi(0), align('n'), leftline(false), rightline(false),
	             topline(false), bottomline(false) {}
	/// cell content
	string content;
	/// multicolumn flag
	int multi;
	/// cell alignment
	char align;
	/// do we have a line on the left?
	bool leftline;
	/// do we have a line on the right?
	bool rightline;
	/// do we have a line above?
	bool topline;
	/// do we have a line below?
	bool bottomline;
};


/// translate a horizontal alignment (as stored in ColInfo and CellInfo) to LyX
inline char const * verbose_align(char c)
{
	return c == 'c' ? "center" : c == 'r' ? "right" : c == 'l' ? "left" : "none";
}


// stripped down from tabluar.C. We use it currently only for bools and
// strings
string const write_attribute(string const & name, bool const & b)
{
	// we write only true attribute values so we remove a bit of the
	// file format bloat for tabulars.
	return b ? ' ' + name + "=\"true\"" : string();
}


string const write_attribute(string const & name, string const & s)
{
	return s.empty() ? string() : ' ' + name + "=\"" + s + '"';
}


int string2int(string const & s, int deflt = 0)
{
	istringstream is(s);
	int i = deflt;
	is >> i;
	return i;
}


/* rather brutish way to code table structure in a string:

  \begin{tabular}{ccc}
    1 & 2 & 3\\ \hline
    \multicolumn{2}{c}{4} & 5 //
    6 & 7 \\
  \end{tabular}

 gets "translated" to:

         HLINE 1 TAB 2               TAB 3 HLINE HLINE LINE
  \hline HLINE \multicolumn{2}{c}{4} TAB 5 HLINE HLINE LINE
         HLINE 6 TAB 7                     HLINE HLINE LINE
*/

char const TAB   = '\001';
char const LINE  = '\002';
char const HLINE = '\004';


/// handle column specifications for tabulars and multicolumns
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

		// We cannot handle comments here
		if (t.cat() == catComment) {
			if (t.cs().empty()) {
				// "%\n" combination
				p.skip_spaces();
			} else
				cerr << "Ignoring comment: " << t.asInput();
			continue;
		}

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


/*!
 * Parse hlines and similar stuff.
 * \returns wether the token \p t was parsed
 */
bool parse_hlines(Parser & p, Token const & t, string & hlines,
                  bool is_long_tabular)
{
	BOOST_ASSERT(t.cat() == catEscape);

	if (t.cs() == "hline")
		hlines += "\\hline";

	else if (t.cs() == "cline")
		hlines += "\\cline{" + p.verbatim_item() + '}';

	else if (is_long_tabular && t.cs() == "newpage")
		hlines += "\\newpage";

	else
		return false;

	return true;
}


/// Position in a row
enum RowPosition {
	/// At the very beginning, before the first token
	ROW_START,
	/// After the first token and before any column token
	IN_HLINES_START,
	/// After the first column token. Comments and whitespace are only
	/// treated as tokens in this position
	IN_COLUMNS,
	/// After the first non-column token at the end
	IN_HLINES_END
};


/*!
 * Parse table structure.
 * We parse tables in a two-pass process: This function extracts the table
 * structure (rows, columns, hlines etc.), but does not change the cell
 * content. The cell content is parsed in a second step in handle_tabular().
 */
void parse_table(Parser & p, ostream & os, bool is_long_tabular,
                 RowPosition & pos, unsigned flags)
{
	// table structure commands such as \hline
	string hlines;

	// comments that occur at places where we can't handle them
	string comments;

	while (p.good()) {
		Token const & t = p.get_token();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
#endif

		// comments and whitespace in hlines
		switch (pos) {
		case ROW_START:
		case IN_HLINES_START:
		case IN_HLINES_END:
			if (t.cat() == catComment) {
				if (t.cs().empty())
					// line continuation
					p.skip_spaces();
				else
					// We can't handle comments here,
					// store them for later use
					comments += t.asInput();
				continue;
			} else if (t.cat() == catSpace ||
			           t.cat() == catNewline) {
				// whitespace is irrelevant here, we
				// need to recognize hline stuff
				p.skip_spaces();
				continue;
			}
			break;
		case IN_COLUMNS:
			break;
		}

		// We need to handle structure stuff first in order to
		// determine wether we need to output a HLINE separator
		// before the row or not.
		if (t.cat() == catEscape) {
			if (parse_hlines(p, t, hlines, is_long_tabular)) {
				switch (pos) {
				case ROW_START:
					pos = IN_HLINES_START;
					break;
				case IN_COLUMNS:
					pos = IN_HLINES_END;
					break;
				case IN_HLINES_START:
				case IN_HLINES_END:
					break;
				}
				continue;
			}

			else if (t.cs() == "tabularnewline" ||
			         t.cs() == "\\" ||
			         t.cs() == "cr") {
				if (t.cs() == "cr")
					cerr << "Warning: Converting TeX "
					        "'\\cr' to LaTeX '\\\\'."
					     << endl;
				// stuff before the line break
				os << comments << HLINE << hlines << HLINE
				   << LINE;
				//cerr << "hlines: " << hlines << endl;
				hlines.erase();
				comments.erase();
				pos = ROW_START;
				continue;
			}

			else if (is_long_tabular &&
			         (t.cs() == "endhead" ||
			          t.cs() == "endfirsthead" ||
			          t.cs() == "endfoot" ||
			          t.cs() == "endlastfoot")) {
				hlines += t.asInput();
				switch (pos) {
				case IN_COLUMNS:
				case IN_HLINES_END:
					// these commands are implicit line
					// breaks
					os << comments << HLINE << hlines
					   << HLINE << LINE;
					hlines.erase();
					comments.erase();
					pos = ROW_START;
					break;
				case ROW_START:
					pos = IN_HLINES_START;
					break;
				case IN_HLINES_START:
					break;
				}
				continue;
			}

		}

		// We need a HLINE separator if we either have no hline
		// stuff at all and are just starting a row or if we just
		// got the first non-hline token.
		switch (pos) {
		case ROW_START:
			// no hline tokens exist, first token at row start
		case IN_HLINES_START:
			// hline tokens exist, first non-hline token at row
			// start
			os << hlines << HLINE << comments;
			hlines.erase();
			comments.erase();
			pos = IN_COLUMNS;
			break;
		case IN_HLINES_END:
			// Oops, there is still cell content after hline
			// stuff. This does not work in LaTeX, so we ignore
			// the hlines.
			cerr << "Ignoring '" << hlines << "' in a cell"
			     << endl;
			os << comments;
			hlines.erase();
			comments.erase();
			pos = IN_COLUMNS;
			break;
		case IN_COLUMNS:
			break;
		}

		// If we come here we have normal cell content
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
			parse_table(p, os, is_long_tabular, pos,
			            FLAG_BRACE_LAST);
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
				parse_table(p, os, is_long_tabular, pos,
				            FLAG_END);
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

	// We can have comments if the last line is incomplete
	os << comments;

	// We can have hline stuff if the last line is incomplete
	if (!hlines.empty()) {
		// this does not work in LaTeX, so we ignore it
		cerr << "Ignoring '" << hlines << "' at end of tabular"
		     << endl;
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


} // anonymous namespace


void handle_tabular(Parser & p, ostream & os, bool is_long_tabular,
		    Context & context)
{
	string posopts = p.getOpt();
	if (!posopts.empty()) {
		if (is_long_tabular)
			cerr << "horizontal longtable";
		else
			cerr << "vertical tabular";
		cerr << " positioning '" << posopts << "' ignored\n";
	}

	vector<ColInfo>            colinfo;

	// handle column formatting
	handle_colalign(p, colinfo);

	// first scan of cells
	// use table mode to keep it minimal-invasive
	// not exactly what's TeX doing...
	vector<string> lines;
	ostringstream ss;
	RowPosition rowpos = ROW_START;
	parse_table(p, ss, is_long_tabular, rowpos, FLAG_END);
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
		// we do only expect this for a last line without '\\'
		if (dummy.size() != 3) {
			if ((dummy.size() != 1 && dummy.size() != 2) ||
			    row != rowinfo.size() - 1)
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
					if (from >= colinfo.size()) {
						cerr << "cline starts at non "
						        "existing column "
						     << (from + 1) << endl;
						from = colinfo.size() - 1;
					}
					size_t to = string2int(t[1]) - 1;
					if (to >= colinfo.size()) {
						cerr << "cline ends at non "
						        "existing column "
						     << (to + 1) << endl;
						to = colinfo.size() - 1;
					}
					for (size_t col = from; col <= to; ++col) {
						//cerr << "row: " << row << " col: " << col << " i: " << i << endl;
						if (i == 0) {
							rowinfo[row].topline = true;
							cellinfo[row][col].topline = true;
						} else {
							rowinfo[row].bottomline = true;
							cellinfo[row][col].bottomline = true;
						}
					}
				} else if (t.cs() == "endhead") {
					if (i > 0)
						rowinfo[row].type = LT_HEAD;
					for (int r = row - 1; r >= 0; --r) {
						if (rowinfo[r].type != LT_NORMAL)
							break;
						rowinfo[r].type = LT_HEAD;
					}
				} else if (t.cs() == "endfirsthead") {
					if (i > 0)
						rowinfo[row].type = LT_FIRSTHEAD;
					for (int r = row - 1; r >= 0; --r) {
						if (rowinfo[r].type != LT_NORMAL)
							break;
						rowinfo[r].type = LT_FIRSTHEAD;
					}
				} else if (t.cs() == "endfoot") {
					if (i > 0)
						rowinfo[row].type = LT_FOOT;
					for (int r = row - 1; r >= 0; --r) {
						if (rowinfo[r].type != LT_NORMAL)
							break;
						rowinfo[r].type = LT_FOOT;
					}
				} else if (t.cs() == "endlastfoot") {
					if (i > 0)
						rowinfo[row].type = LT_LASTFOOT;
					for (int r = row - 1; r >= 0; --r) {
						if (rowinfo[r].type != LT_NORMAL)
							break;
						rowinfo[r].type = LT_LASTFOOT;
					}
				} else if (t.cs() == "newpage") {
					if (i == 0) {
						if (row > 0)
							rowinfo[row - 1].newpage = true;
						else
							cerr << "Ignoring "
							        "'\\newpage' "
							        "before rows."
							     << endl;
					} else
						rowinfo[row].newpage = true;
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
	   << "\" columns=\"" << colinfo.size() << "\">\n";
	os << "<features"
	   << write_attribute("islongtable", is_long_tabular)
	   << ">\n";

	//cerr << "// after header\n";
	for (size_t col = 0; col < colinfo.size(); ++col) {
		os << "<column alignment=\""
		   << verbose_align(colinfo[col].align) << "\""
		   << " valignment=\"top\""
		   << write_attribute("leftline", colinfo[col].leftline)
		   << write_attribute("rightline", colinfo[col].rightline)
		   << write_attribute("width", colinfo[col].width)
		   << write_attribute("special", colinfo[col].special)
		   << ">\n";
	}
	//cerr << "// after cols\n";

	for (size_t row = 0; row < rowinfo.size(); ++row) {
		os << "<row"
		   << write_attribute("topline", rowinfo[row].topline)
		   << write_attribute("bottomline", rowinfo[row].bottomline)
		   << write_attribute("endhead",
		                      rowinfo[row].type == LT_HEAD)
		   << write_attribute("endfirsthead",
		                      rowinfo[row].type == LT_FIRSTHEAD)
		   << write_attribute("endfoot",
		                      rowinfo[row].type == LT_FOOT)
		   << write_attribute("endlastfoot",
		                      rowinfo[row].type == LT_LASTFOOT)
		   << write_attribute("newpage", rowinfo[row].newpage)
		   << ">\n";
		for (size_t col = 0; col < colinfo.size(); ++col) {
			CellInfo const & cell = cellinfo[row][col];
			os << "<cell";
			if (cell.multi)
				os << " multicolumn=\"" << cell.multi << "\"";
			os << " alignment=\"" << verbose_align(cell.align)
			   << "\""
			   << " valignment=\"top\""
			   << write_attribute("topline", cell.topline)
			   << write_attribute("bottomline", cell.bottomline)
			   << write_attribute("leftline", cell.leftline)
			   << write_attribute("rightline", cell.rightline);
			//cerr << "\nrow: " << row << " col: " << col;
			//if (cell.topline)
			//	cerr << " topline=\"true\"";
			//if (cell.bottomline)
			//	cerr << " bottomline=\"true\"";
			os << " usebox=\"none\""
			   << ">"
			   << "\n\\begin_inset Text\n"
			   << cell.content
			   << "\n\\end_inset\n"
			   << "</cell>\n";
		}
		os << "</row>\n";
	}

	os << "</lyxtabular>\n";
}




// }])
