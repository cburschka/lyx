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

#ifdef __GNUG__
#pragma implementation
#endif

#include "tabular_funcs.h"

#include "support/lstrings.h"
#include "support/LIstream.h"

using std::istream;

// Perfect case for a template... (Lgb)
// or perhaps not...

template <>
string const write_attribute(string const & name, bool const & b)
{
	return write_attribute(name, int(b));
}

template <>
string const write_attribute(string const & name, LyXLength const & value)
{
	return write_attribute(name, value.asString());
}

string const tostr(LyXAlignment const & num)
{
	switch(num) {
	case LYX_ALIGN_NONE:
		return "none";
	case LYX_ALIGN_BLOCK:
		return "block";
	case LYX_ALIGN_LEFT:
		return "left";
	case LYX_ALIGN_CENTER:
		return "center";
	case LYX_ALIGN_RIGHT:
		return "right";
	case LYX_ALIGN_LAYOUT:
		return "layout";
	case LYX_ALIGN_SPECIAL:
		return "special";
	}
	return string();
}


string const tostr(LyXTabular::VAlignment const & num)
{
	switch(num) {
	case LyXTabular::LYX_VALIGN_TOP:
		return "top";
	case LyXTabular::LYX_VALIGN_CENTER:
		return "center";
	case LyXTabular::LYX_VALIGN_BOTTOM:
		return "bottom";
	}
	return string();
}


string const tostr(LyXTabular::BoxType const & num)
{
	switch(num) {
	case LyXTabular::BOX_NONE:
		return "none";
	case LyXTabular::BOX_PARBOX:
		return "parbox";
	case LyXTabular::BOX_MINIPAGE:
		return "minipage";
	}
	return string();
}

// I would have liked a fromstr template a lot better. (Lgb)
bool string2type(string const str, LyXAlignment & num)
{
	if (str == "none")
		num = LYX_ALIGN_NONE;
	else if (str == "block")
		num = LYX_ALIGN_BLOCK;
	else if (str == "left")
		num = LYX_ALIGN_LEFT;
	else if (str == "center")
		num = LYX_ALIGN_CENTER;
	else if (str == "right")
		num = LYX_ALIGN_RIGHT;
	else
		return false;
	return true;
}


bool string2type(string const str, LyXTabular::VAlignment & num)
{
	if (str == "top")
		num = LyXTabular::LYX_VALIGN_TOP;
	else if (str == "center")
		num = LyXTabular::LYX_VALIGN_CENTER;
	else if (str == "bottom")
		num = LyXTabular::LYX_VALIGN_BOTTOM;
	else
		return false;
	return true;
}


bool string2type(string const str, LyXTabular::BoxType & num)
{
	if (str == "none")
		num = LyXTabular::BOX_NONE;
	else if (str == "parbox")
		num = LyXTabular::BOX_PARBOX;
	else if (str == "minipage")
		num = LyXTabular::BOX_MINIPAGE;
	else
		return false;
	return true;
}


bool string2type(string const str, bool & num)
{
	if (str == "true")
		num = true;
	else if (str == "false")
		num = false;
	else
		return false;
	return true;
}


bool getTokenValue(string const & str, const char * token, string & ret)
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


bool getTokenValue(string const & str, const char * token, int & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	num = strToInt(tmp);
	return true;
}


bool getTokenValue(string const & str, const char * token, LyXAlignment & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, num);
}


bool getTokenValue(string const & str, const char * token,
				   LyXTabular::VAlignment & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, num);
}


bool getTokenValue(string const & str, const char * token,
				   LyXTabular::BoxType & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, num);
}


bool getTokenValue(string const & str, const char * token, bool & flag)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, flag);
}    


bool getTokenValue(string const & str, const char * token, LyXLength & len)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return isValidLength(tmp, &len);
}    


void l_getline(istream & is, string & str)
{
	str.erase();
	while (str.empty()) {
		std::getline(is, str);
		if (!str.empty() && str[str.length() - 1] == '\r')
			str.erase(str.length() - 1);
	}
}
