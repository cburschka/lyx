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

namespace lyx {

class Inset;
class InsetMath;

class MathAtom {
public:
	/// default constructor, object is useless, but we need it to put it into
	// std::containers
	MathAtom();
	/// the "real constructor"
	explicit MathAtom(Inset * p);
	/// copy constructor, invokes nucleus_->clone()
	MathAtom(MathAtom const &);
	/// we really need to clean up
	~MathAtom();
	/// assignment invokes nucleus_->clone()
	MathAtom & operator=(MathAtom const &);
	/// access to the inset (checked with gprof)
	InsetMath       * nucleus()       { return nucleus_; }
	InsetMath const * nucleus() const { return nucleus_; }
	/// access to the inset
	InsetMath const * operator->() const { return nucleus_; }

private:
	///
	InsetMath * nucleus_;
};


} // namespace lyx

#endif
