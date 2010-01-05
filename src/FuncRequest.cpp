/**
 * \file FuncRequest.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FuncRequest.h"
#include "LyXAction.h"

#include "support/lstrings.h"

#include <climits>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

FuncRequest const FuncRequest::unknown(LFUN_UNKNOWN_ACTION);
FuncRequest const FuncRequest::noaction(LFUN_NOACTION);

FuncRequest::FuncRequest(Origin o)
	: action(LFUN_NOACTION), origin(o), x(0), y(0),
	  button_(mouse_button::none)
{}


FuncRequest::FuncRequest(FuncCode act, Origin o)
	: action(act), origin(o), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(FuncCode act, docstring const & arg, Origin o)
	: action(act), argument_(arg), origin(o), x(0), y(0),
	  button_(mouse_button::none)
{}


FuncRequest::FuncRequest(FuncCode act, string const & arg, Origin o)
	: action(act), argument_(from_utf8(arg)), origin(o), x(0), y(0),
	  button_(mouse_button::none)
{}


FuncRequest::FuncRequest(FuncCode act, int ax, int ay,
			 mouse_button::state but, Origin o)
	: action(act), origin(o), x(ax), y(ay), button_(but)
{}


FuncRequest::FuncRequest(FuncRequest const & cmd, docstring const & arg, Origin o)
	: action(cmd.action), argument_(arg), origin(o),
	  x(cmd.x), y(cmd.y), button_(cmd.button_)
{}


FuncRequest::FuncRequest(FuncRequest const & cmd, string const & arg, Origin o)
	: action(cmd.action), argument_(from_utf8(arg)), origin(o),
	  x(cmd.x), y(cmd.y), button_(cmd.button_)
{}


mouse_button::state FuncRequest::button() const
{
	return button_;
}


namespace {

// Extracts arguments from str into args. Arguments are delimted by
// whitespace or by double quotes.
// We extract at most max + 1 arguments, treating args[max] as 
// continuing to eol.
void splitArg(vector<string> & args, string const & str, 
		unsigned int max = UINT_MAX)
{
	istringstream is(str);
	while (is) {
		if (args.size() == max) {
			string s;
			getline(is, s);
			args.push_back(trim(s));
			return;
		}		

		char c;
		string s;
		is >> c;
		if (is) {
			if (c == '"')
				// get quote delimited argument
				getline(is, s, '"');
			else {
				// get whitespace delimited argument
				is.putback(c);
				is >> s;
			}
			args.push_back(s);
		}
	}
}

}

string FuncRequest::getArg(unsigned int i) const
{
	vector<string> args;
	splitArg(args, to_utf8(argument_));
	return i < args.size() ? args[i] : string();
}


string FuncRequest::getLongArg(unsigned int i) const
{
	vector<string> args;
	splitArg(args, to_utf8(argument_), i);
	return i < args.size() ? args[i] : string();
}


bool operator==(FuncRequest const & lhs, FuncRequest const & rhs)
{
	return lhs.action == rhs.action && lhs.argument() == rhs.argument();
}


ostream & operator<<(ostream & os, FuncRequest const & cmd)
{
	return os
		<< " action: " << cmd.action 
		<< " [" << lyxaction.getActionName(cmd.action) << "] " 
		<< " arg: '" << to_utf8(cmd.argument()) << "'"
		<< " x: " << cmd.x
		<< " y: " << cmd.y;
}


} // namespace lyx
