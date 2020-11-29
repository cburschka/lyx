// -*- C++ -*-
/**
 * \file KeyModifier.h
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

/// Set of KeyModifierFlags
typedef unsigned int KeyModifier;

/// modifier key states
enum KeyModifierFlags : unsigned int {
	NoModifier       = 0,      //< no modifiers held
	ControlModifier  = 1 << 0, //< control button held
	AltModifier      = 1 << 1, //< alt key held
	ShiftModifier    = 1 << 2, //< shift key held
	MetaModifier     = 1 << 3  //< meta key held
};

} // namespace lyx

#endif // KEYMODIFIER_H
