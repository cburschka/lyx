/**
 * \file funcrequest.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author André Pönitz
 */

#ifndef FUNCREQUEST_H
#define FUNCREQUEST_H

#include "commandtags.h"
#include "LString.h"

/** 
 * This class encapsulates a LyX action and its argument
 * in order to pass it around easily.
 */
struct FuncRequest {

	FuncRequest()
		: action(LFUN_UNKNOWN_ACTION)
	{}

	FuncRequest(kb_action act)
		: action(act)
	{}

	FuncRequest(kb_action act, string const & arg)
		: action(act), argument(arg)
	{}

	/// for mouse events
	FuncRequest(kb_action act, int ax, int ay, int aextra)
		: action(act), argument(), x(ax), y(ay), extra(aextra)
	{}

	/// the action
	kb_action action;
	/// the action's string argument
	string argument;
	/// the x coordinate of a mouse press
	int x;
	/// the y coordinate of a mouse press
	int y;
	/// some extra information (like button number)
	int extra;
};

#endif // FUNCREQUEST_H
