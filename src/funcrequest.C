/**
 * \file funcrequest.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author André Pönitz
 */

#include "funcrequest.h"

FuncRequest::FuncRequest()
	: view_(0), action(LFUN_UNKNOWN_ACTION)
{}


FuncRequest::FuncRequest(kb_action act)
	: view_(0), action(act)
{}


FuncRequest::FuncRequest(kb_action act, string const & arg)
	: view_(0), action(act), argument(arg)
{}


FuncRequest::FuncRequest(BufferView * view, kb_action act)
	: view_(view), action(act)
{}


FuncRequest::FuncRequest(BufferView * view, kb_action act, string const & arg)
	: view_(view), action(act), argument(arg)
{}


FuncRequest::FuncRequest
		(BufferView * view, kb_action act, int ax, int ay, int aextra)
	: view_(view), action(act), argument(), x(ax), y(ay), extra(aextra)
{}


BufferView * FuncRequest::view() const
{
	return view_;
}


void FuncRequest::setView(BufferView * view)
{
	view_ = view;
}
