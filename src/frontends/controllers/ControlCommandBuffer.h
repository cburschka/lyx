// -*- C++ -*-
/**
 * \file ControlCommandBuffer.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 * \author John Levon <levon@movementarian.org>
 */

#ifndef CONTROLCOMMANDBUFFER_H
#define CONTROLCOMMANDBUFFER_H

#include <config.h>
 
#include "LString.h"

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

class LyXFunc;

/**
 * ControlCommandBuffer
 *
 * This provides methods for the use of a toolkit's
 * minibuffer/command buffer 
 */
class ControlCommandBuffer {
public:
	ControlCommandBuffer(LyXFunc & lf);
	
	/// return the previous history entry if any
	string const historyUp();

	/// return the next history entry if any
	string const historyDown();

	/// return the possible completions
	std::vector<string> const completions(string const & prefix, string & new_prefix);
 
	/// dispatch a command
	void dispatch(string const & str);
 
private:
	/// controlling lyxfunc
	LyXFunc & lyxfunc_;
	
	/// available command names
	std::vector<string> commands_;
 
	/// command history
	std::vector<string> history_;

	/// current position in command history
	std::vector<string>::const_iterator history_pos_;
};
 
#endif // CONTROLCOMMANDBUFFER_H
