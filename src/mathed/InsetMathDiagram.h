// -*- C++ -*-
/**
 * \file InsetMathDiagram.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
*  \author Ronen Abravanel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DIAGRAM_H
#define MATH_DIAGRAM_H

#include "InsetMathGrid.h"


namespace lyx {


class InsetMathDiagram : public InsetMathGrid {
public:
	///
	explicit InsetMathDiagram(Buffer * buf);
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	InsetMathDiagram const * asDiagramInset() const { return this; }
	///
	int colsep() const override;
	///
	int rowsep() const override;

	///
	void write(TeXMathStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_DIAGRAM_CODE; }

private:
	///
	Inset * clone() const override;

};



} // namespace lyx
#endif
