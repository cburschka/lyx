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

#include "support/Length.h"


namespace lyx {

struct InsetSpaceParams;

/// Smart spaces
class InsetMathSpace : public InsetMath {
public:
	///
	explicit InsetMathSpace();
	///
	explicit InsetMathSpace(std::string const & name, std::string const & length);
	///
	explicit InsetMathSpace(Length const & length, bool const prot = false);
	///
	InsetMathSpace const * asSpaceInset() const override { return this; }
	///
	InsetMathSpace * asSpaceInset() override { return this; }
	///
	void incSpace();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;

	///
	void normalize(NormalStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void mathmlize(MathMLStream & ms) const override;
	///
	void htmlize(HtmlStream & ms) const override;
	///
	void write(TeXMathStream & os) const override;
	/// generate something that will be understood by the Dialogs.
	InsetSpaceParams params() const;
	///
	bool hasSettings() const override { return true; }
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	///
	std::string contextMenuName() const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	InsetCode lyxCode() const override { return MATH_SPACE_CODE; }

protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
private:
	Inset * clone() const override;
	///
	bool isNegative() const;
	///
	int space_;
	/// amount of space for \\hspace
	Length length_;
};


} // namespace lyx
#endif
