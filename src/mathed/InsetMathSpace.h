// -*- C++ -*-
/**
 * \file InsetMathSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "InsetMath.h"
#include "Length.h"


namespace lyx {


/// Smart spaces
class InsetMathSpace : public InsetMath {
public:
	///
	explicit InsetMathSpace();
	///
	explicit InsetMathSpace(std::string const & name, std::string const & length);
	///
	explicit InsetMathSpace(Length const & length);
	///
	~InsetMathSpace();
	///
	InsetMathSpace const * asSpaceInset() const { return this; }
	///
	InsetMathSpace * asSpaceInset() { return this; }
	///
	void incSpace();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
	/// generate something that will be understood by the Dialogs.
	std::string const createDialogStr() const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	docstring contextMenu(BufferView const &, int, int) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	virtual Inset * clone() const;
	///
	bool isNegative() const;
	///
	int space_;
	/// amount of space for \\hspace
	Length length_;
};


} // namespace lyx
#endif
