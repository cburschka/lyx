/**
 * \file funcrequest.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "funcrequest.h"

#include "support/std_sstream.h"

#include <iostream>
#include <vector>

using std::getline;

using std::istringstream;
using std::vector;
using std::string;


FuncRequest::FuncRequest()
	: action(LFUN_NOACTION), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(kb_action act)
	: action(act), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(kb_action act, string const & arg)
	: action(act), argument(arg), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(kb_action act, int ax, int ay, mouse_button::state but)
	: action(act), x(ax), y(ay), button_(but)
{}


FuncRequest::FuncRequest(FuncRequest const & cmd, string const & arg)
	: action(cmd.action), argument(arg),
	  x(cmd.x), y(cmd.y), button_(cmd.button_)
{}


mouse_button::state FuncRequest::button() const
{
	return button_;
}


void split(vector<string> & args, string str)
{
	istringstream is(str);
	while (is) {
		char c;
		string s;
		is >> c;
		if (c == '"')
			getline(is, s, '"');
		else {
			is.putback(c);
			is >> s;
		}
		args.push_back(s);
	}
}


string FuncRequest::getArg(unsigned int i) const
{
	vector<string> args;
	split(args, argument);
	return i < args.size() ? args[i] : string();
}


bool operator==(FuncRequest const & lhs, FuncRequest const & rhs)
{
	return lhs.action == rhs.action && lhs.argument == rhs.argument;
}


std::ostream & operator<<(std::ostream & os, FuncRequest const & cmd)
{
	return os
		<< " action: " << cmd.action 
		<< " arg: '" << cmd.argument << "'"
		<< " x: " << cmd.x 
		<< " y: " << cmd.y; 
}
