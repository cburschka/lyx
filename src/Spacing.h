// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef SPACING_H
#define SPACING_H

#include <cstdio>

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
	Spacing()
	{
		space = Single;
		value = getValue();
	}
	///
	float getValue() const
	{
		switch(space) {
		case Single: return 1.0;
		case Onehalf: return 1.25;
		case Double: return 1.667;
		case Other: return value;
		}
		return 1.0;
	}
	///
	Spacing::Space getSpace() const
	{
		return space;
	}
	///
	void set(Spacing::Space sp, float val = 1.0)
	{
		space = sp;
		if (sp == Other) {
			switch(int(val * 1000 + 0.5)) {
			case 1000: space = Single; break;
			case 1250: space = Onehalf; break;
			case 1667: space = Double; break;
			default: value = val; break;
			}
		}
	}
	///
	void set(Spacing::Space sp, char const * val)
	{
		float fval;
		sscanf(val, "%f", &fval);
		set(sp, fval);
	}
	///
	void writeFile(FILE * file);
	///
	friend bool operator!=(Spacing const & a, Spacing const & b)
	{
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
