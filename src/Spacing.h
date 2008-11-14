// -*- C++ -*-
/**
 * \file src/Spacing.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SPACING_H
#define SPACING_H

#ifdef TEX2LYX
#include "tex2lyx/Spacing.h"
#else

#include "support/strfwd.h"

#include <string>

namespace lyx {

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
	Spacing() : space(Default), value("1.0") {}
	///
	Spacing(Spacing::Space sp, double val = 1.0) { set(sp, val); }
	///
	Spacing(Spacing::Space sp, std::string const & val) { set(sp, val); }
	///
	bool isDefault() const { return space == Default; }
	///
	std::string const getValueAsString() const;
	///
	double getValue() const;
	///
	Spacing::Space getSpace() const { return space; }
	///
	void set(Spacing::Space sp, double val = 1.0);
	///
	void set(Spacing::Space sp, std::string const & val);
	///
	void writeFile(std::ostream &, bool para = false) const;
	/// useSetSpace is true when using the variant supported by
	/// the memoir class.
	std::string const writeEnvirBegin(bool useSetSpace) const;
	/// useSetSpace is true when using the variant supported by
	/// the memoir class.
	std::string const writeEnvirEnd(bool useSetSpace) const;
	/// useSetSpace is true when using the variant supported by
	/// the memoir class.
	std::string const writePreamble(bool useSetSpace) const;

private:
	///
	Space space;
	///
	std::string value;
	/// names of line spacing
	static std::string const spacing_string[];
};


///
inline
bool operator==(Spacing const & a, Spacing const & b)
{
	return a.getSpace() == b.getSpace()
		&& a.getValueAsString() == b.getValueAsString();
}

///
inline
bool operator!=(Spacing const & a, Spacing const & b)
{
	return !(a == b);
}

} // namespace lyx

#endif // TEX2LYX
#endif // SPACING_H
