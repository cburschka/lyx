// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef SPACING_H
#define SPACING_H

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>

#include "LString.h"

///
class Spacing {
public:
	///
	enum Space {
		///
		Single,
		///
		Onehalf,
		///
		Double,
		///
		Other,
		///
		Default
	};
	///
	Spacing() : space(Single), value(1.0) {}
	///
	bool isDefault() const {
		return space == Default;
	}
	///
	float getValue() const;
	///
	Spacing::Space getSpace() const { return space; }
	///
	void set(Spacing::Space sp, float val = 1.0);
	///
	void set(Spacing::Space sp, string const & val) ;
	///
	void writeFile(std::ostream &, bool para = false) const;
	///
	string const writeEnvirBegin() const;
	///
	string const writeEnvirEnd() const;
private:
	///
	Space space;
	///
	float value;
};


///
inline
bool operator==(Spacing const & a, Spacing const & b)
{
	return a.getSpace() == b.getSpace()
		&& a.getValue() == b.getValue();
}

///
inline
bool operator!=(Spacing const & a, Spacing const & b)
{
	return !(a == b);
}
#endif
