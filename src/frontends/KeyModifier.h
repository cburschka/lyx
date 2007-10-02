// -*- C++ -*-
/**
 * \file key_state.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Keyboard modifier state representation.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KEYMODIFIER_H
#define KEYMODIFIER_H

namespace lyx {

/// modifier key states

enum KeyModifier {
	NoModifier  = 0, //< no modifiers held
	ControlModifier  = 1, //< control button held
	AltModifier   = 2, //< alt/meta key held
	ShiftModifier = 4  //< shift key held
};


inline KeyModifier operator|(KeyModifier s1, KeyModifier s2)
{
	int const i1 = static_cast<int>(s1);
	int const i2 = static_cast<int>(s2);
	return static_cast<KeyModifier>(i1 | i2);
}


inline void operator|=(KeyModifier & s1, KeyModifier s2)
{
	s1 = static_cast<KeyModifier>(s1 | s2);
}


} // namespace lyx

#endif // KEY_STATE_H
