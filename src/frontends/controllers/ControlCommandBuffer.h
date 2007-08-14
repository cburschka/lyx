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

#include "support/docstring.h"

#include <vector>


namespace lyx {
namespace frontend {

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
	docstring const getCurrentState() const;

	/// hide the command buffer.
	void hide() const;

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

} // namespace frontend
} // namespace lyx

#endif // CONTROLCOMMANDBUFFER_H
