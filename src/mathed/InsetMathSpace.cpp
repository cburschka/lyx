/**
 * \file InsetMathSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSpace.h"
#include "MathData.h"
#include "MathFactory.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"

#include "insets/InsetSpace.h"

#include "frontends/Application.h"
#include "frontends/Painter.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

namespace {

struct SpaceInfo {
	string name;
	int width;
	InsetSpaceParams::Kind kind;
	bool negative;
	bool visible;
	bool custom;
	bool escape; ///< whether a backslash needs to be added for writing
};

SpaceInfo space_info[] = {
	// name           width kind                            negative visible custom escape
	{"!",                6, InsetSpaceParams::NEGTHIN,         true,  true,  false, true},
	{"negthinspace",     6, InsetSpaceParams::NEGTHIN,         true,  true,  false, true},
	{"negmedspace",      8, InsetSpaceParams::NEGMEDIUM,       true,  true,  false, true},
	{"negthickspace",   10, InsetSpaceParams::NEGTHICK,        true,  true,  false, true},
	{",",                6, InsetSpaceParams::THIN,            false, true,  false, true},
	{"thinspace",        6, InsetSpaceParams::THIN,            false, true,  false, true},
	{":",                8, InsetSpaceParams::MEDIUM,          false, true,  false, true},
	{"medspace",         8, InsetSpaceParams::MEDIUM,          false, true,  false, true},
	{";",               10, InsetSpaceParams::THICK,           false, true,  false, true},
	{"thickspace",      10, InsetSpaceParams::THICK,           false, true,  false, true},
	{"enskip",          10, InsetSpaceParams::ENSKIP,          false, true,  false, true},
	{"enspace",         10, InsetSpaceParams::ENSPACE,         false, true,  false, true},
	{"quad",            20, InsetSpaceParams::QUAD,            false, true,  false, true},
	{"qquad",           40, InsetSpaceParams::QQUAD,           false, true,  false, true},
	{"lyxnegspace",     -2, InsetSpaceParams::NEGTHIN,         true,  false, false, true},
	{"lyxposspace",      2, InsetSpaceParams::THIN,            false, false, false, true},
	{"hfill",           80, InsetSpaceParams::HFILL,           false, true,  false, true},
	{"hspace*{\\fill}", 80, InsetSpaceParams::HFILL_PROTECTED, false, true,  false, true},
	{"hspace*",          0, InsetSpaceParams::CUSTOM_PROTECTED,false, true,  true,  true},
	{"hspace",           0, InsetSpaceParams::CUSTOM,          false, true,  true,  true},
	{" ",               10, InsetSpaceParams::NORMAL,          false, true,  false, true},
	{"~",               10, InsetSpaceParams::PROTECTED,       false, true,  false, false},
};

int const nSpace = sizeof(space_info)/sizeof(SpaceInfo);
int const defaultSpace = 4;

} // anon namespace

InsetMathSpace::InsetMathSpace()
	: space_(defaultSpace)
{
}


InsetMathSpace::InsetMathSpace(string const & name, string const & length)
	: space_(defaultSpace)
{
	for (int i = 0; i < nSpace; ++i)
		if (space_info[i].name == name) {
			space_ = i;
			break;
		}
	if (space_info[space_].custom) {
		length_ = Length(length);
		if (length_.zero() || length_.empty()) {
			length_.value(1.0);
			length_.unit(Length::EM);
		}
	}
}


InsetMathSpace::InsetMathSpace(Length const & length, bool const prot)
	: space_(defaultSpace), length_(length)
{
	for (int i = 0; i < nSpace; ++i)
		if ((prot && space_info[i].name == "hspace*")
			|| (!prot && space_info[i].name == "hspace")) {
			space_ = i;
			break;
		}
}


Inset * InsetMathSpace::clone() const
{
	return new InsetMathSpace(*this);
}


void InsetMathSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = 4;
	dim.des = 0;
	if (space_info[space_].custom)
		dim.wid = abs(length_.inPixels(
				mi.base.textwidth,
				mathed_char_width(mi.base.font, 'M')));
	else
		dim.wid = space_info[space_].width;
}


void InsetMathSpace::draw(PainterInfo & pi, int x, int y) const
{
	// Sadly, HP-UX CC can't handle that kind of initialization.
	// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};
	if (!space_info[space_].visible)
		return;

	Dimension const dim = dimension(*pi.base.bv);
	int xp[4];
	int yp[4];
	int w = dim.wid;

	xp[0] = ++x;        yp[0] = y - 3;
	xp[1] = x;          yp[1] = y;
	xp[2] = x + w - 2;  yp[2] = y;
	xp[3] = x + w - 2;  yp[3] = y - 3;

	pi.pain.lines(xp, yp, 4,
			space_info[space_].custom ?
			Color_special :
			(isNegative() ? Color_latex : Color_math));
}


void InsetMathSpace::incSpace()
{
	int const oldwidth = space_info[space_].width;
	do
		space_ = (space_ + 1) % nSpace;
	while ((space_info[space_].width == oldwidth && !space_info[space_].custom) ||
	       !space_info[space_].visible);
	if (space_info[space_].custom && (length_.zero() || length_.empty())) {
		length_.value(1.0);
		length_.unit(Length::EM);
	}
}


void InsetMathSpace::validate(LaTeXFeatures & features) const
{
	if (space_info[space_].name == "negmedspace" ||
	    space_info[space_].name == "negthickspace")
		features.require("amsmath");
}


void InsetMathSpace::maple(MapleStream & os) const
{
	os << ' ';
}

void InsetMathSpace::mathematica(MathematicaStream & os) const
{
	os << ' ';
}


void InsetMathSpace::octave(OctaveStream & os) const
{
	os << ' ';
}


void InsetMathSpace::mathmlize(MathStream & ms) const
{
	SpaceInfo const & si = space_info[space_];
	if (si.negative || !si.visible)
		return;
	string l;
	if (si.custom)
		l = length_.asHTMLString();
	else if (si.kind != InsetSpaceParams::MEDIUM) {
		stringstream ss;
		ss << si.width;
		l = ss.str() + "px";
	}
	
	ms << "<mspace";
	if (!l.empty())
		ms << " width=\"" << from_ascii(l) << "\"";
	ms << " />";
}

	
void InsetMathSpace::htmlize(HtmlStream & ms) const
{
	SpaceInfo const & si = space_info[space_];
	switch (si.kind) {
	case InsetSpaceParams::THIN:
		ms << from_ascii("&thinsp;");
		break;
	case InsetSpaceParams::MEDIUM:
		ms << from_ascii("&nbsp;");
		break;
	case InsetSpaceParams::THICK:
		ms << from_ascii("&emsp;");
		break;
	case InsetSpaceParams::ENSKIP:
	case InsetSpaceParams::ENSPACE:
		ms << from_ascii("&ensp;");
		break;
	case InsetSpaceParams::QUAD:
		ms << from_ascii("&emsp;");
		break;
	case InsetSpaceParams::QQUAD:
		ms << from_ascii("&emsp;&emsp;");
		break;
	case InsetSpaceParams::HFILL:
	case InsetSpaceParams::HFILL_PROTECTED:
		// FIXME: is there a useful HTML entity?
		break;
	case InsetSpaceParams::CUSTOM:
	case InsetSpaceParams::CUSTOM_PROTECTED: {
		string l = length_.asHTMLString();
		ms << MTag("span", "width='" + l + "'") 
		   << from_ascii("&nbsp;") << ETag("span");
		break;
	}
	case InsetSpaceParams::NORMAL:
	case InsetSpaceParams::PROTECTED:
		ms << from_ascii("&nbsp;");
		break;
	default:
		break;
	}
}

	
void InsetMathSpace::normalize(NormalStream & os) const
{
	os << "[space " << int(space_) << "] ";
}


void InsetMathSpace::write(WriteStream & os) const
{
	// no MathEnsurer - all kinds work in text and math mode
	if (space_info[space_].escape)
		os << '\\';
	os << space_info[space_].name.c_str();
	if (space_info[space_].custom)
		os << '{' << length_.asLatexString().c_str() << '}';
	else if (space_info[space_].escape && space_info[space_].name.length() > 1)
		os.pendingSpace(true);
}


InsetSpaceParams InsetMathSpace::params() const
{
	InsetSpaceParams isp(true);
	LASSERT(space_info[space_].visible, return isp);
	isp.kind = space_info[space_].kind;
	isp.length = GlueLength(length_);
	return isp;
}


string InsetMathSpace::contextMenuName() const
{
	return "context-mathspace";
}


bool InsetMathSpace::getStatus(Cursor & cur, FuncRequest const & cmd,
                               FuncStatus & status) const
{
	switch (cmd.action()) {
	// we handle these
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
		status.setEnabled(true);
		return true;
	default:
		bool retval = InsetMath::getStatus(cur, cmd, status);
		return retval;
	}
}


void InsetMathSpace::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "mathspace") {
			MathData ar;
			if (createInsetMath_fromDialogStr(cmd.argument(), ar)) {
				cur.recordUndo();
				*this = *ar[0].nucleus()->asSpaceInset();
				break;
			}
		}
		cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button1) {
			showInsetDialog(&cur.bv());
			break;
		}
		cur.undispatched();
		break;

	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
		// eat other mouse commands
		break;

	default:
		InsetMath::doDispatch(cur, cmd);
		break;
	}
}


bool InsetMathSpace::isNegative() const
{
	if (space_info[space_].custom)
		return length_.value() < 0;
	return space_info[space_].negative;
}

} // namespace lyx
