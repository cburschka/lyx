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

using namespace std;

namespace lyx {


InsetHFill::InsetHFill()
	: InsetCommand(InsetCommandParams(HFILL_CODE), string())
{}


ParamInfo const & InsetHFill::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	return param_info_;
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
	int const x0 = x + 1;
	int const x1 = x + dim.wid - 2;
	int const y0 = y + dim.des - 1;
	int const y1 = y - dim.asc + 1;

	pi.pain.line(x0, y1, x0, y0, Color_added_space);
	pi.pain.line(x0, y, x1, y, Color_added_space,
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


void InsetHFill::write(Buffer const &, ostream & os) const
{
	os << "\n\\hfill\n";
}


bool InsetHFill::isSpace() const
{
	return true;
}


} // namespace lyx
