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

#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "latexrunparams.h"
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


class InsetFormula::PreviewImpl : public lyx::graphics::PreviewedInset {
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


InsetFormula::InsetFormula(BufferView *)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new PreviewImpl(*this))
{
	//view_ = bv->owner()->view();
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


Inset * InsetFormula::clone() const
{
	return new InsetFormula(*this);
}


void InsetFormula::write(Buffer const *, ostream & os) const
{
	WriteStream wi(os, false, false);
	os << par_->fileInsetLabel() << ' ';
	par_->write(wi);
}


int InsetFormula::latex(Buffer const *, ostream & os,
			LatexRunParams const & runparams) const
{
	WriteStream wi(os, runparams.moving_arg, true);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::ascii(Buffer const *, ostream & os, int) const
{
	if (0 && display()) {
		Dimension dim;
		TextMetricsInfo mi;
		par()->metricsT(mi, dim);
		TextPainter tpain(dim.width(), dim.height());
		par()->drawT(tpain, 0, dim.ascent());
		tpain.show(os, 3);
		// reset metrics cache to "real" values
		//metrics();
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
	//metrics();
}


//ostream & operator<<(ostream & os, LyXCursor const & c)
//{
//	os << '[' << c.x() << ' ' << c.y() << ' ' << c.pos() << ']';
//	return os;
//}


void InsetFormula::draw(PainterInfo & pi, int x, int y) const
{
	cache(pi.base.bv);
	// This initiates the loading of the preview, so should come
	// before the metrics are computed.
	bool const use_preview = preview_->previewReady();

	int const w = dim_.wid;
	int const d = dim_.des;
	int const a = dim_.asc;
	int const h = a + d;

	if (use_preview) {
		pi.pain.image(x + 1, y - a, w, h,   // one pixel gap in front
			      *(preview_->pimage()->image()));
	} else {
		PainterInfo p(pi.base.bv);
		p.base.style = LM_ST_TEXT;
		p.base.font  = pi.base.font;
		p.base.font.setColor(LColor::math);
		if (lcolor.getX11Name(LColor::mathbg)
			    != lcolor.getX11Name(LColor::background))
			p.pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

		if (mathcursor &&
				const_cast<InsetFormulaBase const *>(mathcursor->formula()) == this)
		{
			mathcursor->drawSelection(pi);
			//p.pain.rectangle(x, y - a, w, h, LColor::mathframe);
		}

		par_->draw(p, x, y);
	}

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


void InsetFormula::metrics(MetricsInfo & m, Dimension & dim) const
{
	view_ = m.base.bv;
	if (preview_->previewReady()) {
		dim.asc = preview_->pimage()->ascent();
		dim.des = preview_->pimage()->descent();
		// insert a one pixel gap in front of the formula
		dim.wid = 1 + preview_->pimage()->width();
		if (display())
			dim.des += 12;
	} else {
		MetricsInfo mi = m;
		mi.base.style = LM_ST_TEXT;
		mi.base.font.setColor(LColor::math);
		par()->metrics(mi, dim_);
		dim.asc += 1;
		dim.des += 1;
	}
	dim_ = dim;
}


void InsetFormula::mutate(string const & type)
{
	par_.nucleus()->mutate(type);
}


//
// preview stuff
//

void InsetFormula::addPreview(lyx::graphics::PreviewLoader & ploader) const
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
