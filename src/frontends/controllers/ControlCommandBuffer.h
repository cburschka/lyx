// -*- C++ -*-
/**
 * \file ControlCommandBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars
 * \author Asger and Juergen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLCOMMANDBUFFER_H
#define CONTROLCOMMANDBUFFER_H

#include "support/std_string.h"

#include <vector>


class LyXView;

/**
 * ControlCommandBuffer
 *
 * This provides methods for the use of a toolkit's
 * minibuffer/command buffer
 */
class ControlCommandBuffer {
public:
	ControlCommandBuffer(LyXView & lv);

	/// return the previous history entry if any
	string const historyUp();

	/// return the next history entry if any
	string const historyDown();

	/// return the font and depth in the active BufferView as a message.
	string const getCurrentState() const;

	/// return the possible completions
	std::vector<string> const completions(string const & prefix,
					      string & new_prefix);

	/// dispatch a command
	void dispatch(string const & str);
private:
	/// controlling LyXView
	LyXView & lv_;

	/// available command names
	std::vector<string> commands_;

	/// command history
	std::vector<string> history_;

	/// current position in command history
	std::vector<string>::const_iterator history_pos_;
};

#endif // CONTROLCOMMANDBUFFER_H
