// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 * ======================================================
 */

#ifndef InsetFloat_H
#define InsetFloat_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

class Painter;

/** The float inset
  
*/
class InsetFloat : public InsetCollapsable {
public:
	///
	InsetFloat(string const &);
	///
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	void Read(Buffer const * buf, LyXLex & lex);
	///
	void Validate(LaTeXFeatures & features) const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::FLOAT_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	const char * EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
	///
	void InsetButtonRelease(BufferView * bv, int x, int y, int button);
	///
	void wide(bool w);
	///
	bool wide() const;
private:
	///
	string floatType;
	///
	string floatPlacement;
	///
	bool wide_;
};

#endif
