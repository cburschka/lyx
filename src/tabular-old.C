/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 *           @author: Jürgen Vigna
 *
 * ======================================================
 */

#include <config.h>

#include "tabular.h"
#include "buffer.h"
#include "debug.h"

#include "support/lstrings.h"
#include "support/textutils.h"

using std::istream;
using std::getline;
using std::endl;

#ifndef CXX_GLOBAL_CSTD
using std::strlen;
#endif

namespace {

bool getTokenValue(string const & str, char const * token, string & ret)
{
    size_t token_length = strlen(token);
    string::size_type pos = str.find(token);

    if (pos == string::npos || pos + token_length + 1 >= str.length()
	|| str[pos + token_length] != '=')
	return false;
    ret.erase();
    pos += token_length + 1;
    char ch = str[pos];
    if ((ch != '"') && (ch != '\'')) { // only read till next space
	ret += ch;
	ch = ' ';
    }
    while ((pos < str.length() - 1) && (str[++pos] != ch))
	ret += str[pos];

    return true;
}


bool getTokenValue(string const & str, char const * token, int & num)
{
    string::size_type pos = str.find(token);
    char ch = str[pos + strlen(token)];

    if ((pos == string::npos) || (ch != '='))
	return false;
    string ret;
    pos += strlen(token) + 1;
    ch = str[pos];
    if ((ch != '"') && (ch != '\'')) { // only read till next space
	if (!IsDigit(ch))
	    return false;
	ret += ch;
    }
    ++pos;
    while ((pos < str.length() - 1) && IsDigit(str[pos]))
	ret += str[pos++];

    num = strToInt(ret);
    return true;
}


bool getTokenValue(string const & str, char const * token, LyXAlignment & num)
{
    int tmp;
    bool const ret = getTokenValue(str, token, tmp);
    num = static_cast<LyXAlignment>(tmp);
    return ret;
}


bool getTokenValue(string const & str, char const * token,
		   LyXTabular::VAlignment & num)
{
    int tmp;
    bool const ret = getTokenValue(str, token, tmp);
    num = static_cast<LyXTabular::VAlignment>(tmp);
    return ret;
}


bool getTokenValue(string const & str, char const * token,
		   LyXTabular::BoxType & num)
{
    int tmp;
    bool ret = getTokenValue(str, token, tmp);
    num = static_cast<LyXTabular::BoxType>(tmp);
    return ret;
}


bool getTokenValue(string const & str, char const * token, bool & flag)
{
    string::size_type pos = str.find(token);
    char ch = str[pos + strlen(token)];

    if ((pos == string::npos) || (ch != '='))
	return false;
    string ret;
    pos += strlen(token) + 1;
    ch = str[pos];
    if ((ch != '"') && (ch != '\'')) { // only read till next space
	if (!IsDigit(ch))
	    return false;
	ret += ch;
    }
    ++pos;
    while ((pos < str.length() - 1) && IsDigit(str[pos]))
	ret += str[pos++];

    flag = strToInt(ret);
    return true;
}


bool getTokenValue(string const & str, const char * token, LyXLength & len)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return isValidLength(tmp, &len);
}


inline
void l_getline(istream & is, string & str)
{
#ifdef WITH_WARNINGS
//#warning old l_getline
#endif
    getline(is, str);
    while (str.empty())
	getline(is, str);
}

} // namespace anon


void LyXTabular::ReadOld(Buffer const * buf, istream & is,
			 LyXLex & lex, string const & l)
{
    string line(l);
    int rows_arg;
    int columns_arg;
    if (!getTokenValue(line, "rows", rows_arg))
	return;
    if (!getTokenValue(line, "columns", columns_arg))
	return;
    Init(buf->params, rows_arg, columns_arg);
    l_getline(is, line);
    if (!prefixIs(line, "<Features ")) {
	lyxerr << "Wrong tabular format (expected <Feture ...> got" <<
	    line << ")" << endl;
	return;
    }
    getTokenValue(line, "islongtable", is_long_tabular);
	int hrow;
	int fhrow;
	int frow;
	int lfrow;

	getTokenValue(line, "endhead", hrow);
	getTokenValue(line, "endfirsthead", fhrow);
	getTokenValue(line, "endfoot", frow);
	getTokenValue(line, "endlastfoot", lfrow);
	setHeaderFooterRows(abs(hrow), abs(fhrow), abs(frow), abs(lfrow));

    for (int i = 0; i < rows_; ++i) {
	l_getline(is, line);
	if (!prefixIs(line, "<Row ")) {
	    lyxerr << "Wrong tabular format (expected <Row ...> got" <<
		line << ")" << endl;
	    return;
	}
	getTokenValue(line, "topline", row_info[i].top_line);
	getTokenValue(line, "bottomline", row_info[i].bottom_line);
	getTokenValue(line, "newpage", row_info[i].newpage);
	for (int j = 0; j < columns_; ++j) {
	    l_getline(is,line);
	    if (!prefixIs(line,"<Column")) {
		lyxerr << "Wrong tabular format (expected <Column ...> got" <<
		    line << ")" << endl;
		return;
	    }
	    if (!i) {
		getTokenValue(line, "alignment", column_info[j].alignment);
		getTokenValue(line, "valignment", column_info[j].valignment);
		getTokenValue(line, "leftline", column_info[j].left_line);
		getTokenValue(line, "rightline", column_info[j].right_line);
		getTokenValue(line, "width", column_info[j].p_width);
		getTokenValue(line, "special", column_info[j].align_special);
	    }
	    l_getline(is, line);
	    if (!prefixIs(line, "<Cell")) {
		lyxerr << "Wrong tabular format (expected <Cell ...> got" <<
		    line << ")" << endl;
		return;
	    }
	    getTokenValue(line, "multicolumn", cell_info[i][j].multicolumn);
	    getTokenValue(line, "alignment", cell_info[i][j].alignment);
	    getTokenValue(line, "valignment", cell_info[i][j].valignment);
	    getTokenValue(line, "topline", cell_info[i][j].top_line);
	    getTokenValue(line, "bottomline", cell_info[i][j].bottom_line);
	    getTokenValue(line, "leftline", cell_info[i][j].left_line);
	    getTokenValue(line, "rightline", cell_info[i][j].right_line);
	    getTokenValue(line, "rotate", cell_info[i][j].rotate);
	    getTokenValue(line, "usebox", cell_info[i][j].usebox);
	    getTokenValue(line, "width", cell_info[i][j].p_width);
	    getTokenValue(line, "special", cell_info[i][j].align_special);
	    l_getline(is, line);
	    if (prefixIs(line, "\\begin_inset")) {
		cell_info[i][j].inset.read(buf, lex);
		l_getline(is, line);
	    }
	    if (line != "</Cell>") {
		lyxerr << "Wrong tabular format (expected </Cell> got" <<
		    line << ")" << endl;
		return;
	    }
	    l_getline(is, line);
	    if (line != "</Column>") {
		lyxerr << "Wrong tabular format (expected </Column> got" <<
		    line << ")" << endl;
		return;
	    }
	}
	l_getline(is, line);
	if (line != "</Row>") {
	    lyxerr << "Wrong tabular format (expected </Row> got" <<
		line << ")" << endl;
	    return;
	}
    }
    while (line != "</LyXTabular>") {
	l_getline(is, line);
    }
    set_row_column_number_info();
}
