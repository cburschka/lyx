// -*- C++ -*-
/**
 * \file tex2lyx/Spacing.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * This class is just a dummy version of that in the main LyX source tree
 * to enable tex2lyx to use LyX's textclass classes and not have to
 * re-invent the wheel.
 */

#ifndef TEX2LYX_SPACING_H
#define TEX2LYX_SPACING_H

#include <string>


namespace lyx {

class Spacing {
public:
	///
	enum Space {
		Single,
		Onehalf,
		Double,
		Other,
		Default
	};
	///
	void set(Spacing::Space, double = 1.0) {}
	///
	void set(Spacing::Space, std::string const &) {}
};


} // namespace lyx

#endif // TEX2LYX_SPACING_H
