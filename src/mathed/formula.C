/*
*  File:        formula.C
*  Purpose:     Implementation of formula inset
*  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
*  Created:     January 1996
*  Description: Allows the edition of math paragraphs inside Lyx.
*
*  Copyright: 1996-1998 Alejandro Aguilar Sierra
*
*  Version: 0.4, Lyx project.
*
*   You are free to use and modify this code under the terms of
*   the GNU General Public Licence version 2 or later.
*/


#include <config.h>

#include "formula.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_charinset.h"
#include "math_arrayinset.h"
#include "metricsinfo.h"
#include "math_deliminset.h"
#include "math_hullinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "textpainter.h"
#include "Lsstream.h"

#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"
#include "funcrequest.h"
#include "Lsstream.h"

#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/lyxlib.h"
#include "support/systemcall.h"
#include "support/filetools.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "graphics/PreviewedInset.h"
#include "graphics/PreviewImage.h"


using std::ostream;
using std::vector;


class InsetFormula::PreviewImpl : public grfx::PreviewedInset {
public:
	///
	PreviewImpl(InsetFormula & p) : PreviewedInset(p) {}

private:
	///
	bool previewWanted() const;
	///
	string const latexString() const;
	///
	InsetFormula & parent() const
	{
		return *static_cast<InsetFormula*>(inset());
	}
};



InsetFormula::InsetFormula(bool chemistry)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new PreviewImpl(*this))
{
	if (chemistry)
		mutate("chemistry");
}


InsetFormula::InsetFormula(InsetFormula const & other)
	: InsetFormulaBase(other),
	  par_(other.par_),
	  preview_(new PreviewImpl(*this))
{}


InsetFormula::InsetFormula(BufferView * bv)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new PreviewImpl(*this))
{
	view_ = bv->owner()->view();
}


InsetFormula::InsetFormula(string const & data)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new PreviewImpl(*this))
{
	if (!data.size())
		return;
	if (!mathed_parse_normal(par_, data))
		lyxerr << "cannot interpret '" << data << "' as math\n";
}



InsetFormula::~InsetFormula()
{}


Inset * InsetFormula::clone(Buffer const &, bool) const
{
	return new InsetFormula(*this);
}


void InsetFormula::write(Buffer const *, ostream & os) const
{
	WriteStream wi(os, false, false);
	os << par_->fileInsetLabel() << ' ';
	par_->write(wi);
}


int InsetFormula::latex(Buffer const *, ostream & os, LatexRunParams const &,
			bool fragile, bool) const
{
	WriteStream wi(os, fragile, true);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::ascii(Buffer const *, ostream & os, int) const
{
	if (0 && display()) {
		TextMetricsInfo mi;
		par()->metricsT(mi);
		TextPainter tpain(par()->width(), par()->height());
		par()->drawT(tpain, 0, par()->ascent());
		tpain.show(os, 3);
		// reset metrics cache to "real" values
		metrics();
		return tpain.textheight();
	} else {
		WriteStream wi(os, false, true);
		wi << ' ' << (par_->asNestInset()->cell(0)) << ' ';
		return wi.line();
	}
}


int InsetFormula::linuxdoc(Buffer const * buf, ostream & os) const
{
	return docbook(buf, os, false);
}


int InsetFormula::docbook(Buffer const * buf, ostream & os, bool) const
{
	MathMLStream ms(os);
	ms << MTag("equation");
	ms <<   MTag("alt");
	ms <<    "<[CDATA[";
	int res = ascii(buf, ms.os(), 0);
	ms <<    "]]>";
	ms <<   ETag("alt");
	ms <<   MTag("math");
	ms <<    par_;
	ms <<   ETag("math");
	ms << ETag("equation");
	return ms.line() + res;
}


void InsetFormula::read(Buffer const *, LyXLex & lex)
{
	mathed_parse_normal(par_, lex);
	// remove extra 'mathrm' for chemistry stuff.
	// will be re-added on write
	if (par_->asHullInset()->getType() =="chemistry")  {
		lyxerr << "this is chemistry\n";
		if (par_->cell(0).size() == 1) {
			lyxerr << "this is size 1\n";
	    if (par_->cell(0)[0]->asFontInset()) {
				lyxerr << "this is a font inset \n";
				lyxerr << "replacing " << par_.nucleus()->cell(0) << 
					" with " << par_->cell(0)[0]->cell(0) << "\n";
			}
		}
	}
	metrics();
}


//ostream & operator<<(ostream & os, LyXCursor const & c)
//{
//	os << '[' << c.x() << ' ' << c.y() << ' ' << c.pos() << ']';
//	return os;
//}


void InsetFormula::draw(BufferView * bv, LyXFont const & font,
			int y, float & xx) const
{
	cache(bv);
	// This initiates the loading of the preview, so should come
	// before the metrics are computed.
	bool const use_preview = preview_->previewReady();

	int const x = int(xx);
	int const w = width(bv, font);
	int const d = descent(bv, font);
	int const a = ascent(bv, font);
	int const h = a + d;

	PainterInfo pi(bv->painter());

	if (use_preview) {
		pi.pain.image(x + 1, y - a, w, h,   // one pixel gap in front
			      *(preview_->pimage()->image()));
	} else {
		pi.base.style = LM_ST_TEXT;
		pi.base.font  = font;
		pi.base.font.setColor(LColor::math);
		if (lcolor.getX11Name(LColor::mathbg)
			    != lcolor.getX11Name(LColor::background))
			pi.pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

		if (mathcursor &&
				const_cast<InsetFormulaBase const *>(mathcursor->formula()) == this)
		{
			mathcursor->drawSelection(pi);
			//pi.pain.rectangle(x, y - a, w, h, LColor::mathframe);
		}

		par_->draw(pi, x, y);
	}

	xx += w;
	xo_ = x;
	yo_ = y;
}


vector<string> const InsetFormula::getLabelList() const
{
	vector<string> res;
	par()->getLabelList(res);
	return res;
}


Inset::Code InsetFormula::lyxCode() const
{
	return Inset::MATH_CODE;
}


void InsetFormula::validate(LaTeXFeatures & features) const
{
	par_->validate(features);
}


bool InsetFormula::insetAllowed(Inset::Code code) const
{
	return
		   code == Inset::LABEL_CODE
		|| code == Inset::REF_CODE
		|| code == Inset::ERT_CODE;
}


void InsetFormula::dimension(BufferView * bv, LyXFont const & font,
	Dimension & dim) const
{
	metrics(bv, font);
	if (preview_->previewReady()) {
		dim.a = preview_->pimage()->ascent();
		int const descent = preview_->pimage()->descent();
		dim.d = display() ? descent + 12 : descent;
		// insert a one pixel gap in front of the formula
		dim.w = 1 + preview_->pimage()->width();
	} else {
		dim = par_->dimensions();
		dim.a += 1;
		dim.d += 1;
	}
}


void InsetFormula::mutate(string const & type)
{
	par_.nucleus()->mutate(type);
}


//
// preview stuff
//

void InsetFormula::addPreview(grfx::PreviewLoader & ploader) const
{
	preview_->addPreview(ploader);
}


void InsetFormula::generatePreview() const
{
	preview_->generatePreview();
}


bool InsetFormula::PreviewImpl::previewWanted() const
{
	return !parent().par_->asNestInset()->editing();
}


string const InsetFormula::PreviewImpl::latexString() const
{
	ostringstream ls;
	WriteStream wi(ls, false, false);
	parent().par_->write(wi);
	return STRCONV(ls.str());
}

