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
	InsetFloat(InsetFloat const &, bool same_id = false);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::FLOAT_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	int docBook(Buffer const *, std::ostream &) const;
	///
	string const editMessage() const;
	///
	bool insetAllowed(Inset::Code) const;
	///
	void insetButtonRelease(BufferView * bv, int x, int y, int button);
	///
	string const & type() const;
	///
	void placement(string const & p);
	///
	string const & placement() const;
	///
	void wide(bool w);
	///
	bool wide() const;
private:
	///
	string floatType_;
	///
	string floatPlacement_;
	///
	bool wide_;
};

#endif
