// -*- C++ -*-

#ifndef MATH_ATOM_H
#define MATH_ATOM_H

#ifdef __GNUG__
#pragma interface
#endif

/**
The 'atom' is the major blob in math typesetting.  And 'atom' consists
of a nucleus, an optional superscript, and an optional subscript.

Exactly where the subscript and superscript are drawn depends on the
size, and type, of the nucleus they are attached to.

Jules

--

Ok: Implementing it thusly is not feasible since cursor movement gets
hackish. We use MathAtom only as a wrapper around MathInset * with value
semantics.

The MathAtom owns the MathInset * and is responsible for proper cloning and
destruction. Every MathInset * should be put into a MathAtom after its
creation as soon as possible.

Andre'

*/

class MathInset;

class MathAtom {
public:
	/// default constructor, object is useless, but we need it to put it into
	// std::containers
	MathAtom();
	/// the "real constructor"
	explicit MathAtom(MathInset * p);
	/// copy constructor, invokes nucleus_->clone()
	MathAtom(MathAtom const &);
	/// we really need to clean up
	~MathAtom();
	/// assignment invokes nucleus_->clone()
	void operator=(MathAtom const &);
	/// change inset under the hood
	void reset(MathInset * p);
	/// access to the inset (checked with gprof)
	MathInset * nucleus() const { return nucleus_; }
	/// access to the inset
	MathInset * operator->() const { return nucleus_; }

private:
	///
	MathInset * nucleus_;
};

#endif
