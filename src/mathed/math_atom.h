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

Andre'

*/

class MathInset;

class MathAtom {
public: 
	///
	MathAtom();
	///
	MathAtom(MathAtom const &);
	///
	explicit MathAtom(MathInset * p);
	/// 
	virtual ~MathAtom(); 
	///
	void operator=(MathAtom const &);
	///
	void reset(MathInset * p);
	///
	MathInset * nucleus() const;
	///
	MathInset * operator->() const;

private:
	///
	MathInset * nucleus_;

	/// raw copy
	void copy(MathAtom const & p);
	/// raw destruction
	void done();
};

#endif
