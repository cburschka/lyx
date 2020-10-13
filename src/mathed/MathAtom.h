// -*- C++ -*-
/**
 * \file MathAtom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ATOM_H
#define MATH_ATOM_H


/**
Wrapper for InsetMath * with copy-semantics

--

The 'atom' is the major blob in math typesetting.  And 'atom' consists
of a nucleus, an optional superscript, and an optional subscript.

Exactly where the subscript and superscript are drawn depends on the
size, and type, of the nucleus they are attached to.

Jules

--

Ok: Implementing it thusly is not feasible since cursor movement gets
hackish. We use MathAtom only as a wrapper around InsetMath * with value
semantics.

The MathAtom owns the InsetMath * and is responsible for proper cloning and
destruction. Every InsetMath * should be put into a MathAtom after its
creation as soon as possible.

Andre'

*/

#include "support/unique_ptr.h"

namespace lyx {

class InsetMath;

class MathAtom {
public:
	MathAtom() = default;
	MathAtom(MathAtom &&) = default;
	MathAtom & operator=(MathAtom &&) = default;
	/// the "real constructor"
	explicit MathAtom(InsetMath * p);
	/// copy constructor, invokes clone()
	MathAtom(MathAtom const &);
	MathAtom & operator=(MathAtom const &);
	/// access to the inset
	InsetMath * nucleus() { return nucleus_.get(); }
	InsetMath const * nucleus() const { return nucleus_.get(); }
	InsetMath * operator->() { return nucleus_.get(); }
	InsetMath const * operator->() const { return nucleus_.get(); }
private:
	std::unique_ptr<InsetMath> nucleus_;
};


} // namespace lyx

#endif
