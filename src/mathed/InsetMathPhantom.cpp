/**
 * \file InsetMathPhantom.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathPhantom.h"

#include "LaTeXFeatures.h"
#include "MathStream.h"

#include "MetricsInfo.h"

#include "frontends/Painter.h"

#include <algorithm>
#include <ostream>

namespace lyx {


InsetMathPhantom::InsetMathPhantom(Buffer * buf, Kind k)
	: InsetMathNest(buf, 1), kind_(k)
{}


Inset * InsetMathPhantom::clone() const
{
	return new InsetMathPhantom(*this);
}


void InsetMathPhantom::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathPhantom::draw(PainterInfo & pi, int x, int y) const
{
	static int const arrow_size = 4;

	// We first draw the text and then an arrow
	ColorCode const origcol = pi.base.font.color();
	if (visibleContents())
		pi.base.font.setColor(Color_special);
	cell(0).draw(pi, x + 1, y);
	if (visibleContents())
		pi.base.font.setColor(origcol);
	Dimension const dim = dimension(*pi.base.bv);

	if (kind_ == phantom || kind_ == vphantom) {
		// y1---------
		//           / \.
		// y2-----  / | \.
		//            |
		//            |
		// y3-----  \ | /
		//           \ /
		// y4---------
		//          | | |
		//         /  |  \.
		//        x1  x2 x3

		int const x2 = x + dim.wid / 2;
		int const x1 = x2 - arrow_size;
		int const x3 = x2 + arrow_size;

		int const y1 = y - dim.asc;
		int const y2 = y1 + arrow_size;
		int const y4 = y + dim.des;
		int const y3 = y4 - arrow_size;

		// top arrow
		pi.pain.line(x2, y1, x1, y2, Color_added_space);
		pi.pain.line(x2, y1, x3, y2, Color_added_space);

		// bottom arrow
		pi.pain.line(x2, y4, x1, y3, Color_added_space);
		pi.pain.line(x2, y4, x3, y3, Color_added_space);

		// joining line
		pi.pain.line(x2, y1, x2, y4, Color_added_space);
	}

	if (kind_ == phantom || kind_ == hphantom) {
		// y1----  /            \.
		//        /              \.
		// y2--- <---------------->
		//        \              /
		// y3----  \            /
		//       |  |          |  |
		//      x1 x2         x3 x4

		int const x1 = x;
		int const x2 = x + arrow_size;
		int const x4 = x + dim.wid;
		int const x3 = x4 - arrow_size;

		int const y2 = y + (dim.des - dim.asc) / 2;
		int const y1 = y2 - arrow_size;
		int const y3 = y2 + arrow_size;

		// left arrow
		pi.pain.line(x1, y2, x2, y3, Color_added_space);
		pi.pain.line(x1, y2, x2, y1, Color_added_space);

		// right arrow
		pi.pain.line(x4, y2, x3, y3, Color_added_space);
		pi.pain.line(x4, y2, x3, y1, Color_added_space);

		// joining line
		pi.pain.line(x1, y2, x4, y2, Color_added_space);
	}

	else if (kind_ == mathclap) {
		// y1----      \     /
		//              \   /
		// y2--- -------->-<--------
		//              /   \.
		// y3----      /     \.
		//       |    |   |   |    |
		//      x1   x2  x3  x4   x5

		int const x1 = x;
		int const x5 = x + dim.wid;
		int const x3 = x + dim.wid / 2;
		int const x2 = std::max(x1, x3 - arrow_size);
		int const x4 = std::min(x5, x3 + arrow_size);

		int const y2 = y + (dim.des - dim.asc) / 2;
		int const y1 = y2 - arrow_size;
		int const y3 = y2 + arrow_size;

		// left arrow
		pi.pain.line(x2, y3, x3, y2, Color_added_space);
		pi.pain.line(x2, y1, x3, y2, Color_added_space);

		// right arrow
		pi.pain.line(x4, y3, x3, y2, Color_added_space);
		pi.pain.line(x4, y1, x3, y2, Color_added_space);

		// joining line
		pi.pain.line(x1, y2, x5, y2, Color_added_space);
	}

	else if (kind_ == mathllap) {
		// y1----                \.
		//                        \.
		// y2--- ------------------>
		//                        /
		// y3----                /
		//       |              |  |
		//      x1             x2 x3

		int const x1 = x;
		int const x3 = x + dim.wid;
		int const x2 = std::max(x1, x3 - arrow_size);

		int const y2 = y + (dim.des - dim.asc) / 2;
		int const y1 = y2 - arrow_size;
		int const y3 = y2 + arrow_size;

		// right arrow
		pi.pain.line(x3, y2, x2, y3, Color_added_space);
		pi.pain.line(x3, y2, x2, y1, Color_added_space);

		// joining line
		pi.pain.line(x1, y2, x3, y2, Color_added_space);
	}

	else if (kind_ == mathrlap) {
		// y1----  /
		//        /
		// y2--- <------------------
		//        \.
		// y3----  \.
		//       |  |              |
		//      x1 x2             x3

		int const x1 = x;
		int const x3 = x + dim.wid;
		int const x2 = std::min(x3, x + arrow_size);

		int const y2 = y + (dim.des - dim.asc) / 2;
		int const y1 = y2 - arrow_size;
		int const y3 = y2 + arrow_size;

		// left arrow
		pi.pain.line(x1, y2, x2, y3, Color_added_space);
		pi.pain.line(x1, y2, x2, y1, Color_added_space);

		// joining line
		pi.pain.line(x1, y2, x3, y2, Color_added_space);
	}

	else if (kind_ == smash || kind_ == smasht || kind_ == smashb) {
		// y1---------
		//            |
		// y2-----  \ | /
		//           \ /
		// y3-------- |
		//           / \.
		// y4-----  / | \.
		//            |
		// y5---------
		//          | | |
		//         /  |  \.
		//        x1  x2 x3

		int const x2 = x + dim.wid / 2;
		int const x1 = x2 - arrow_size;
		int const x3 = x2 + arrow_size;

		int const y1 = y - dim.asc;
		int const y5 = y + dim.des;
		int const y3 = y;
		int const y2 = std::max(y1, y3 - arrow_size);
		int const y4 = std::min(y5, y3 + arrow_size);

		// top arrow
		if (kind_ != smashb) {
			pi.pain.line(x1, y2, x2, y3, Color_added_space);
			pi.pain.line(x3, y2, x2, y3, Color_added_space);
		}

		// bottom arrow
		if (kind_ != smasht) {
			pi.pain.line(x1, y4, x2, y3, Color_added_space);
			pi.pain.line(x3, y4, x2, y3, Color_added_space);
		}

		// joining line
		if (kind_ == smasht)
			pi.pain.line(x2, y1, x2, y3, Color_added_space);
		else if (kind_ == smashb)
			pi.pain.line(x2, y3, x2, y5, Color_added_space);
		else
			pi.pain.line(x2, y1, x2, y5, Color_added_space);
	}

	drawMarkers(pi, x, y);
}


void InsetMathPhantom::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	if (os.fragile())
		os << "\\protect";
	switch (kind_) {
	case phantom:
		os << "\\phantom{";
		break;
	case vphantom:
		os << "\\vphantom{";
		break;
	case hphantom:
		os << "\\hphantom{";
		break;
	case smash:
		os << "\\smash{";
		break;
	case smasht:
		os << "\\smash[t]{";
		break;
	case smashb:
		os << "\\smash[b]{";
		break;
	case mathclap:
		os << "\\mathclap{";
		break;
	case mathllap:
		os << "\\mathllap{";
		break;
	case mathrlap:
		os << "\\mathrlap{";
		break;
	}
	os << cell(0) << '}';
}


void InsetMathPhantom::normalize(NormalStream & os) const
{
	switch (kind_) {
	case phantom:
		os << "[phantom ";
		break;
	case vphantom:
		os << "[vphantom ";
		break;
	case hphantom:
		os << "[hphantom ";
		break;
	case smash:
		os << "[smash ";
		break;
	case smasht:
		os << "[smasht ";
		break;
	case smashb:
		os << "[smashb ";
		break;
	case mathclap:
		os << "[mathclap ";
		break;
	case mathllap:
		os << "[mathllap ";
		break;
	case mathrlap:
		os << "[mathrlap ";
		break;
	}
	os << cell(0) << ']';
}


void InsetMathPhantom::infoize(odocstream & os) const
{
	switch (kind_) {
	case phantom:
		os << "Phantom";
		break;
	case vphantom:
		os << "Vphantom";
		break;
	case hphantom:
		os << "Hphantom";
		break;
	case smash:
		os << "Smash";
		break;
	case smasht:
		os << "Smashtop";
		break;
	case smashb:
		os << "Smashbottom";
		break;
	case mathllap:
		os << "Mathllap";
		break;
	case mathclap:
		os << "Mathclap";
		break;
	case mathrlap:
		os << "Mathrlap";
		break;
	}
}


void InsetMathPhantom::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	switch (kind_) {
	case phantom:
	case vphantom:
	case hphantom:
	case smash:
		break;
	case smasht:
	case smashb:
		features.require("amsmath");
		break;
	case mathclap:
	case mathllap:
	case mathrlap:
		features.require("mathtools");
		break;
	}
}


bool InsetMathPhantom::visibleContents() const 
{ 
	return kind_ == phantom || kind_ == vphantom || kind_ == hphantom;
}


} // namespace lyx
