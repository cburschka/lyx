/**
 * \file mouse_state.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * GUII representation of mouse presses and
 * mouse button states
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef MOUSE_STATE_H
#define MOUSE_STATE_H

/// used both for presses and held during motion
namespace mouse_button {

	enum state {
		none = 0, //< no buttons held
		button1	= 1, //< mouse button 1 pressed
		button2 = 2,
		button3 = 4,
		button4 = 8,
		button5 = 16
	};

inline void operator|=(state & s1, state s2)
{
	s1 = static_cast<state>(s1 | s2);
}
 
} // namespace mouse_button

#endif // MOUSE_STATE_H
