// -*- C++ -*-
/**
 * \file funcrequest.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FUNCREQUEST_H
#define FUNCREQUEST_H

#include "lfuns.h"
#include "frontends/mouse_state.h"

#include <string>

class BufferView;

/**
 * This class encapsulates a LyX action and its argument
 * in order to pass it around easily.
 */
class FuncRequest {
public:
	/// just for putting thes things in std::container
	FuncRequest();
	/// actions without extra argument
	explicit FuncRequest(kb_action act);
	/// actions without extra argument
	FuncRequest(kb_action act, int x, int y, mouse_button::state button);
	/// actions with extra argument
	FuncRequest(kb_action act, std::string const & arg);
	/// actions without extra argument
	FuncRequest(BufferView * bv, kb_action act);
	/// actions with extra argument
	FuncRequest(BufferView * bv, kb_action act, std::string const & arg);
	/// for mouse events
	FuncRequest(BufferView * bv, kb_action act,
		int x, int y, mouse_button::state button);
	/// for changing requests a bit
	FuncRequest(FuncRequest const & cmd, std::string const & arg);
	/// for changing requests a bit
	FuncRequest(FuncRequest const & cmd, BufferView * bv);

	/// access to the view
	BufferView * view() const;
	/// access to the view
	void setView(BufferView * bv);
	/// access to button
	mouse_button::state button() const;

	/// output a message
	void message(std::string const & msg) const;
	/// output an error message
	void errorMessage(std::string const & msg) const;

	/// argument parsing, extract argument i as std::string
	std::string getArg(unsigned int i) const;

private:
	/// the BufferView we are talking to
	BufferView * view_;
public:  // should be private, too...
	/// the action
	kb_action action;
	/// the action's std::string argument
	std::string argument;
	/// the x coordinate of a mouse press
	int x;
	/// the y coordinate of a mouse press
	int y;
	/// some extra information (like button number)
	mouse_button::state button_;
};


inline
bool operator==(FuncRequest const & lhs, FuncRequest const & rhs)
{
	return lhs.action == rhs.action && lhs.argument == rhs.argument;
}

#endif // FUNCREQUEST_H
