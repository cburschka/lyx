/**
 * \file InsetMathDiagram.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Ronen Abravanel
*
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathDiagram.h"

#include "MathStream.h"

#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include <ostream>

namespace lyx {


InsetMathDiagram::InsetMathDiagram(Buffer * buf) : InsetMathGrid(buf, 1, 1)
{
}


Inset * InsetMathDiagram::clone() const
{
	return new InsetMathDiagram(*this);
}


int InsetMathDiagram::colsep() const
{
	return 10;
}


int InsetMathDiagram::rowsep() const
{
	return 10;
}


void InsetMathDiagram::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (mi.base.style == LM_ST_DISPLAY)
		mi.base.style = LM_ST_TEXT;
	InsetMathGrid::metrics(mi, dim);
}


void InsetMathDiagram::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\Diagram";
	bool open = os.startOuterRow();
	os << '{';
	InsetMathGrid::write(os);
	os << "}\n";
	if (open)
		os.startOuterRow();
}


void InsetMathDiagram::infoize(odocstream & os) const
{
	os << "Diagram ";
	InsetMathGrid::infoize(os);
}


void InsetMathDiagram::normalize(NormalStream & os) const
{
	os << "[Diagram ";
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathDiagram::maple(MapleStream & os) const
{
	os << "Diagram(";
	InsetMathGrid::maple(os);
	os << ')';
}


void InsetMathDiagram::validate(LaTeXFeatures & features) const
{
	features.require("feyn");
	InsetMathGrid::validate(features);
}


} // namespace lyx
