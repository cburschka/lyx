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
#include "BufferView.h"
#include "lyxfunc.h" // only for setMessage()
#include "frontends/LyXView.h"
#include "debug.h"
#include "support/std_sstream.h"

using std::endl;
using std::getline;

using std::istringstream;
using std::vector;


FuncRequest::FuncRequest()
	: view_(0), action(LFUN_UNKNOWN_ACTION), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(kb_action act)
	: view_(0), action(act), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(kb_action act, string const & arg)
	: view_(0), action(act), argument(arg), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest
		(kb_action act, int ax, int ay, mouse_button::state button)
	: view_(0), action(act), x(ax), y(ay), button_(button)
{}


FuncRequest::FuncRequest(BufferView * view, kb_action act)
	: view_(view), action(act), x(0), y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest(BufferView * view, kb_action act, string const & arg)
	: view_(view), action(act), argument(arg), x(0),  y(0), button_(mouse_button::none)
{}


FuncRequest::FuncRequest
		(BufferView * view, kb_action act, int ax, int ay, mouse_button::state but)
	: view_(view), action(act), x(ax), y(ay), button_(but)
{}


FuncRequest::FuncRequest(FuncRequest const & cmd, string const & arg)
	: view_(cmd.view_), action(cmd.action), argument(arg),
	  x(cmd.x), y(cmd.y), button_(cmd.button_)
{}


FuncRequest::FuncRequest(FuncRequest const & cmd, BufferView * view)
	: view_(view), action(cmd.action), argument(cmd.argument),
	  x(cmd.x), y(cmd.y), button_(cmd.button_)
{}


BufferView * FuncRequest::view() const
{
	return view_;
}


void FuncRequest::setView(BufferView * view)
{
	view_ = view;
}


mouse_button::state FuncRequest::button() const
{
	return button_;
}


void FuncRequest::message(string const & msg) const
{
	if (view_)
		view_->owner()->getLyXFunc().setMessage(msg);
	else
		lyxerr  << "Dropping message '" << msg << "'" << endl;
}


void FuncRequest::errorMessage(string const & msg) const
{
	if (view_)
		view_->owner()->getLyXFunc().setErrorMessage(msg);
	else
		lyxerr  << "Dropping error message '" << msg << "'" << endl;
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


string FuncRequest::getArg(int i) const
{
	vector<string> args;
	split(args, argument);
	return i < args.size() ? args[i] : string();
}
