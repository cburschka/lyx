/**
 * \file key_state.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * Keyboard modifier state representation.
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef KEY_STATE_H
#define KEY_STATE_H

/// modifier key states
namespace key_modifier { 
	enum state {
		none	= 0, //< no modifiers held
		ctrl 	= 1, //< control button held
		alt	= 2, //< alt/meta key held
		shift	= 4  //< shift key held
	};

 
inline state operator|(state const & s1, state const & s2)
{
	int const i1(static_cast<int>(s1));
	int const i2(static_cast<int>(s2));
	return static_cast<state>(i1 | i2);
}

 
inline void operator|=(state & s1, state s2)
{
        s1 = static_cast<state>(s1 | s2);
}
 
} // namespace key_modifier

#endif // KEY_STATE_H
