/**
 * \file insetvspace.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetvspace.h"

#include "buffer.h"
#include "BufferView.h"
#include "cursor.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

#include <sstream>

using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;
using std::max;


namespace {

int const ADD_TO_VSPACE_WIDTH = 5;

} // namespace anon


InsetVSpace::InsetVSpace(VSpace const & space)
	: space_(space)
{}


InsetVSpace::~InsetVSpace()
{
	InsetVSpaceMailer(*this).hideDialog();
}


std::auto_ptr<InsetBase> InsetVSpace::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetVSpace(*this));
}


void InsetVSpace::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetVSpaceMailer::string2params(cmd.argument, space_);
		break;
	}

	case LFUN_MOUSE_PRESS:
		InsetVSpaceMailer(*this).showDialog(&cur.bv());
		break;

	default:
		InsetOld::doDispatch(cur, cmd);
		break;
	}
}


void InsetVSpace::read(Buffer const &, LyXLex & lex)
{
	BOOST_ASSERT(lex.isOK());
	string vsp;
	lex >> vsp;
	if (lex)
		space_ = VSpace(vsp);

	string end_token;
	lex >> end_token;
	if (end_token != "\\end_inset")
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
}


void InsetVSpace::write(Buffer const &, ostream & os) const
{
	os << "VSpace " << space_.asLyXCommand();
}


void InsetVSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	int size = 10;
	int const arrow_size = 4;
	int const space_size = space_.inPixels(*mi.base.bv);

	LyXFont font;
	font.decSize();
	int const min_size = max(3 * arrow_size, font_metrics::maxHeight(font));

	if (space_.length().len().value() < 0.0)
		size = min_size;
	else
		size = max(min_size, space_size);

	dim.asc = size / 2;
	dim.des = size / 2;
	dim.wid = 10 + 2 * ADD_TO_VSPACE_WIDTH;

	dim_ = dim;
}


void InsetVSpace::draw(PainterInfo & pi, int x, int y) const
{
	static std::string const label = _("Vertical Space");

	setPosCache(pi, x, y);

	x += ADD_TO_VSPACE_WIDTH;

	int const arrow_size = 4;
	int const start = y - dim_.asc;
	int const end   = y + dim_.des;

	// the label to display (if any)
	string str;
	// y-values for top arrow
	int ty1, ty2;
	// y-values for bottom arrow
	int by1, by2;

	str = label + " (" + space_.asLyXCommand() + ")";

	if (space_.kind() == VSpace::VFILL) {
		ty1 = ty2 = start;
		by1 = by2 = end;
	} else {
		// adding or removing space
		bool const added = space_.kind() != VSpace::LENGTH ||
				   space_.length().len().value() > 0.0;
		ty1 = added ? (start + arrow_size) : start;
		ty2 = added ? start : (start + arrow_size);
		by1 = added ? (end - arrow_size) : end;
		by2 = added ? end : (end - arrow_size);
	}

	int const midx = x + arrow_size;
	int const rightx = midx + arrow_size;

	// first the string
	int w = 0;
	int a = 0;
	int d = 0;

	LyXFont font;
	font.setColor(LColor::added_space);
	font.decSize();
	font.decSize();
	font_metrics::rectText(str, font, w, a, d);

	pi.pain.rectText(x + 2 * arrow_size + 5, y + d,
		       str, font, LColor::none, LColor::none);

	// top arrow
	pi.pain.line(x, ty1, midx, ty2, LColor::added_space);
	pi.pain.line(midx, ty2, rightx, ty1, LColor::added_space);

	// bottom arrow
	pi.pain.line(x, by1, midx, by2, LColor::added_space);
	pi.pain.line(midx, by2, rightx, by1, LColor::added_space);

	// joining line
	pi.pain.line(midx, ty2, midx, by2, LColor::added_space);
}


int InsetVSpace::latex(Buffer const & buf, ostream & os,
			  OutputParams const &) const
{
	os << space_.asLatexCommand(buf.params()) << '\n';
	return 1;
}


int InsetVSpace::plaintext(Buffer const &, ostream & os,
			   OutputParams const &) const
{
	os << "\n\n";
	return 2;
}


int InsetVSpace::linuxdoc(Buffer const &, std::ostream & os,
			  OutputParams const &) const
{
	os << '\n';
	return 1;
}


int InsetVSpace::docbook(Buffer const &, std::ostream & os,
			 OutputParams const &) const
{
	os << '\n';
	return 1;
}


string const InsetVSpaceMailer::name_ = "vspace";


InsetVSpaceMailer::InsetVSpaceMailer(InsetVSpace & inset)
	: inset_(inset)
{}


string const InsetVSpaceMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.space());
}


void InsetVSpaceMailer::string2params(string const & in, VSpace & vspace)
{
	vspace = VSpace();
	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetVSpaceMailer", in, 1, name_);

	string vsp;
	lex >> vsp;
	if (lex)
		vspace = VSpace(vsp);
}


string const InsetVSpaceMailer::params2string(VSpace const & vspace)
{
	ostringstream data;
	data << name_ << ' ' << vspace.asLyXCommand();
	return data.str();
}
