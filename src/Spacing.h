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

#include <iosfwd>

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
		Other
	};
	///
	Spacing() {
		space = Single;
		value = getValue();
	}
	///
	float getValue() const;
	///
	Spacing::Space getSpace() const { return space; }
	///
	void set(Spacing::Space sp, float val = 1.0);
	///
	void set(Spacing::Space sp, char const * val) ;
	///
	void writeFile(std::ostream &) const;
	///
	friend bool operator!=(Spacing const & a, Spacing const & b) {
		if (a.space == b.space && a.getValue() == b.getValue())
			return false;
		return true;
	}
private:
	///
	Space space;
	///
	float value;
};
#endif
