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
#include <iosfwd>


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
	/// for changing requests a bit
	FuncRequest(FuncRequest const & cmd, std::string const & arg);

	/// access to button
	mouse_button::state button() const;

	/// argument parsing, extract argument i as std::string
	std::string getArg(unsigned int i) const;

public:  // should be private
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


bool operator==(FuncRequest const & lhs, FuncRequest const & rhs);

std::ostream & operator<<(std::ostream &, FuncRequest const &);

#endif // FUNCREQUEST_H
