/**
 * \file funcrequest.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author André Pönitz
 */

#include "funcrequest.h"
#include "BufferView.h"
#include "lyxfunc.h" // only for setMessage()
#include "frontends/LyXView.h"
#include "debug.h"


FuncRequest::FuncRequest()
	: view_(0), action(LFUN_UNKNOWN_ACTION)
{}


FuncRequest::FuncRequest(kb_action act)
	: view_(0), action(act)
{}


FuncRequest::FuncRequest(kb_action act, string const & arg)
	: view_(0), action(act), argument(arg)
{}


FuncRequest::FuncRequest
		(kb_action act, int ax, int ay, mouse_button::state button)
	: view_(0), action(act), argument(), x(ax), y(ay), button_(button)
{}


FuncRequest::FuncRequest(BufferView * view, kb_action act)
	: view_(view), action(act)
{}


FuncRequest::FuncRequest(BufferView * view, kb_action act, string const & arg)
	: view_(view), action(act), argument(arg)
{}


FuncRequest::FuncRequest
		(BufferView * view, kb_action act, int ax, int ay, mouse_button::state but)
	: view_(view), action(act), argument(), x(ax), y(ay), button_(but)
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
		lyxerr  << "Dropping message '" << msg << "'\n";
}


void FuncRequest::errorMessage(string const & msg) const
{
	if (view_)
		view_->owner()->getLyXFunc().setErrorMessage(msg);
	else
		lyxerr  << "Dropping error message '" << msg << "'\n";
}
