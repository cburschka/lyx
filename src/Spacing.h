// -*- C++ -*-
/**
 * \file Spacing.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SPACING_H
#define SPACING_H

#include <iosfwd>

#include "support/std_string.h"

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
