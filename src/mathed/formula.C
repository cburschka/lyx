/**
 * \file formula.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "formula.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_hullinset.h"
#include "math_mathmlstream.h"
#include "textpainter.h"

#include "debug.h"
#include "latexrunparams.h"
#include "support/std_sstream.h"


#include "frontends/Painter.h"

#include "graphics/PreviewedInset.h"
#include "graphics/PreviewImage.h"


using std::ostream;
using std::ostringstream;
using std::vector;
using std::auto_ptr;
using std::endl;


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
		lyxerr << "cannot interpret '" << data << "' as math" << endl;
}



InsetFormula::~InsetFormula()
{}


auto_ptr<InsetBase> InsetFormula::clone() const
{
	return auto_ptr<InsetBase>(new InsetFormula(*this));
}


void InsetFormula::write(Buffer const &, ostream & os) const
{
	WriteStream wi(os, false, false);
	os << par_->fileInsetLabel() << ' ';
	par_->write(wi);
}


int InsetFormula::latex(Buffer const &, ostream & os,
			LatexRunParams const & runparams) const
{
	WriteStream wi(os, runparams.moving_arg, true);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::ascii(Buffer const &, ostream & os, int) const
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


int InsetFormula::linuxdoc(Buffer const & buf, ostream & os) const
{
	return docbook(buf, os, false);
}


int InsetFormula::docbook(Buffer const & buf, ostream & os, bool) const
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


void InsetFormula::read(Buffer const &, LyXLex & lex)
{
	mathed_parse_normal(par_, lex);
	// remove extra 'mathrm' for chemistry stuff.
	// will be re-added on write
	if (par_->asHullInset()->getType() =="chemistry")  {
		lyxerr << "this is chemistry" << endl;
		if (par_->cell(0).size() == 1) {
			lyxerr << "this is size 1" << endl;
			if (par_->cell(0)[0]->asFontInset()) {
				lyxerr << "this is a font inset "
				       << "replacing " << par_.nucleus()->cell(0) <<
					" with " << par_->cell(0)[0]->cell(0) << endl;
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

		par_->draw(p, x + offset_, y);
	}

	xo_ = x;
	yo_ = y;
}


void InsetFormula::getLabelList(vector<string> & res) const
{
	par()->getLabelList(res);
}


InsetOld::Code InsetFormula::lyxCode() const
{
	return InsetOld::MATH_CODE;
}


void InsetFormula::validate(LaTeXFeatures & features) const
{
	par_->validate(features);
}


bool InsetFormula::insetAllowed(InsetOld::Code code) const
{
	return
		   code == InsetOld::LABEL_CODE
		|| code == InsetOld::REF_CODE
		|| code == InsetOld::ERT_CODE;
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
		par()->metrics(mi, dim);
		dim.asc += 1;
		dim.des += 1;
	}

	if (display()) {
		offset_ = (m.base.textwidth - dim.wid) / 2;
		dim.wid = m.base.textwidth;
	} else {
		offset_ = 0;
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
