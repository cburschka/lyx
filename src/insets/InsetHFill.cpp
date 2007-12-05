/**
 * \file InsetHFill.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetHFill.h"

#include "MetricsInfo.h"

#include "frontends/Painter.h"

#include "support/gettext.h"

#include <ostream>


namespace lyx {


InsetHFill::InsetHFill()
	: InsetCommand(InsetCommandParams(HFILL_CODE), std::string())
{}


CommandInfo const * InsetHFill::findInfo(std::string const & /* cmdName */)
{
	static const char * const paramnames[] = {""};
	static const CommandInfo info = {0, paramnames, 0};
	return &info;
}


Inset * InsetHFill::clone() const
{
	return new InsetHFill;
}


void InsetHFill::metrics(MetricsInfo &, Dimension & dim) const
{
	// The metrics for this inset are calculated externally in
	// \c TextMetrics::computeRowMetrics. Those are dummy value:
	dim = Dimension(10, 10, 10);
}


void InsetHFill::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = Inset::dimension(*pi.base.bv);
	x += 1;

	int const y0 = y + dim.des;
	int const y1 = y - dim.asc;

	pi.pain.line(x, y1, x, y0, Color_added_space);
	if (dim.wid == 0)
		// The HFill is not expanded.
		return;

	int const x1 = x + dim.wid - 2;

	pi.pain.line(x, y, x1, y, Color_added_space,
		frontend::Painter::line_onoffdash);
	pi.pain.line(x1, y1, x1, y0, Color_added_space);
}


docstring const InsetHFill::getScreenLabel(Buffer const &) const
{
	return _("Horizontal Fill");
}


int InsetHFill::plaintext(Buffer const &, odocstream & os,
			  OutputParams const &) const
{
	os << "     ";
	return 5;
}


int InsetHFill::docbook(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	os << '\n';
	return 0;
}


void InsetHFill::write(Buffer const &, std::ostream & os) const
{
	os << "\n\\hfill\n";
}


bool InsetHFill::isSpace() const
{
	return true;
}


} // namespace lyx
