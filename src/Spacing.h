// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
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
	Spacing() : space(Default), value(1.0) {}
	///
	Spacing(Spacing::Space sp, float val = 1.0) {
		set(sp, val);
	}
	Spacing(Spacing::Space sp, string const & val) {
		set(sp, val);
	}
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
	/// names of line spacing
	static string const spacing_string[];
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
