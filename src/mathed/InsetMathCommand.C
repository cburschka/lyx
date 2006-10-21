/**
 * \file InsetMathCommand.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathCommand.h"
#include "MathData.h"
#include "MathMLStream.h"
#include "dispatchresult.h"
#include "funcrequest.h"

#include <sstream>


namespace lyx {

using std::string;
using std::auto_ptr;


CommandInset::CommandInset(string const & name)
	: InsetMathNest(2), name_(name), set_label_(false)
{
	lock_ = true;
}


auto_ptr<InsetBase> CommandInset::doClone() const
{
	return auto_ptr<InsetBase>(new CommandInset(*this));
}


void CommandInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (!set_label_) {
		set_label_ = true;
		button_.update(screenLabel(), true);
	}
	button_.metrics(mi, dim);
	dim_ = dim;
}


InsetBase * CommandInset::editXY(LCursor & cur, int /*x*/, int /*y*/)
{
	edit(cur, true);
	return this;
}


void CommandInset::draw(PainterInfo & pi, int x, int y) const
{
	button_.draw(pi, x, y);
	setPosCache(pi, x, y);
}


void CommandInset::write(WriteStream & os) const
{
	os << '\\' << name_.c_str();
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(0) << '}';
}


docstring const CommandInset::screenLabel() const
{
	return from_ascii(name_);
}


string const CommandInset::createDialogStr(string const & name) const
{
	odocstringstream os;
	os << from_ascii(name) << " LatexCommand ";
	WriteStream ws(os);
	write(ws);
	ws << "\n\\end_inset\n\n";
	return to_utf8(os.str());
}


} // namespace lyx
