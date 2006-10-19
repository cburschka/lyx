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
#include "cursor.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "metricsinfo.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <sstream>

using lyx::docstring;
using lyx::odocstream;

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
		InsetVSpaceMailer::string2params(lyx::to_utf8(cmd.argument()), space_);
		break;
	}

	case LFUN_MOUSE_RELEASE:
		InsetVSpaceMailer(*this).showDialog(&cur.bv());
		break;

	default:
		InsetBase::doDispatch(cur, cmd);
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


string const InsetVSpace::label() const
{
	// FIXME UNICODE
	static string const label = lyx::to_utf8(_("Vertical Space"));
	return label + " (" + space_.asGUIName() + ')';
}


namespace {
int const arrow_size = 4;
}


void InsetVSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	int height = 3 * arrow_size;
	if (space_.length().len().value() >= 0.0)
		height = max(height, space_.inPixels(*mi.base.bv));

	LyXFont font;
	font.decSize();
	font.decSize();

	int w = 0;
	int a = 0;
	int d = 0;
	string lab = label();
	docstring dlab(lab.begin(), lab.end());
	theFontMetrics(font).rectText(dlab, w, a, d);

	height = max(height, a + d);

	dim.asc = height / 2 + (a - d) / 2; // align cursor with the
	dim.des = height - dim.asc;         // label text
	dim.wid = ADD_TO_VSPACE_WIDTH + 2 * arrow_size + 5 + w;
	dim_ = dim;
}


void InsetVSpace::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	x += ADD_TO_VSPACE_WIDTH;

	int const start = y - dim_.asc;
	int const end   = y + dim_.des;

	// y-values for top arrow
	int ty1, ty2;
	// y-values for bottom arrow
	int by1, by2;

	if (space_.kind() == VSpace::VFILL) {
		ty1 = ty2 = start;
		by1 = by2 = end;
	} else {
		// adding or removing space
		bool const added = space_.kind() != VSpace::LENGTH ||
				   space_.length().len().value() >= 0.0;
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
	string lab = label();
	docstring dlab(lab.begin(), lab.end());
	theFontMetrics(font).rectText(dlab, w, a, d);

	pi.pain.rectText(x + 2 * arrow_size + 5,
			 start + (end - start) / 2 + (a - d) / 2,
			 dlab, font, LColor::none, LColor::none);

	// top arrow
	pi.pain.line(x, ty1, midx, ty2, LColor::added_space);
	pi.pain.line(midx, ty2, rightx, ty1, LColor::added_space);

	// bottom arrow
	pi.pain.line(x, by1, midx, by2, LColor::added_space);
	pi.pain.line(midx, by2, rightx, by1, LColor::added_space);

	// joining line
	pi.pain.line(midx, ty2, midx, by2, LColor::added_space);
}


int InsetVSpace::latex(Buffer const & buf, odocstream & os,
			  OutputParams const &) const
{
	os << lyx::from_ascii(space_.asLatexCommand(buf.params())) << '\n';
	return 1;
}


int InsetVSpace::plaintext(Buffer const &, odocstream & os,
			   OutputParams const &) const
{
	os << "\n\n";
	return 2;
}


int InsetVSpace::docbook(Buffer const &, odocstream & os,
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
