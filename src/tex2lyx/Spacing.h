// -*- C++ -*-
/**
 *  \file Spacing.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 *  Full author contact details are available in file CREDITS
 */

#ifndef SPACING_H
#define SPACING_H

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

	void set(Spacing::Space, float = 1.0) {}
};

#endif // NOT SPACING_H
