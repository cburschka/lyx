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
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	InsetMathDiagram const * asDiagramInset() const { return this; }
	///
	virtual int colsep() const;
	///
	virtual int rowsep() const;

	///
	void write(WriteStream & os) const;
	///
	void infoize(odocstream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_DIAGRAM_CODE; }

private:
	///
	virtual Inset * clone() const;

};



} // namespace lyx
#endif
