// -*- C++ -*-
/**
 * \file ControlCommandBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars
 * \author Asger and Jürgen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLCOMMANDBUFFER_H
#define CONTROLCOMMANDBUFFER_H

#include <string>
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
	std::string const historyUp();

	/// return the next history entry if any
	std::string const historyDown();

	/// return the font and depth in the active BufferView as a message.
	std::string const getCurrentState() const;

	/// return the possible completions
	std::vector<std::string> const completions(std::string const & prefix,
					      std::string & new_prefix);

	/// dispatch a command
	void dispatch(std::string const & str);
private:
	/// controlling LyXView
	LyXView & lv_;

	/// available command names
	std::vector<std::string> commands_;

	/// command history
	std::vector<std::string> history_;

	/// current position in command history
	std::vector<std::string>::const_iterator history_pos_;
};

#endif // CONTROLCOMMANDBUFFER_H
