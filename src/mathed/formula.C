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

#include "BufferView.h"
#include "debug.h"
#include "LColor.h"
#include "lyx_main.h"
#include "outputparams.h"

#include "frontends/Painter.h"

#include "graphics/PreviewLoader.h"

#include "insets/render_preview.h"

#include "support/std_sstream.h"

#include <boost/bind.hpp>

using std::string;
using std::ostream;
using std::ostringstream;
using std::vector;
using std::auto_ptr;
using std::endl;


InsetFormula::InsetFormula(bool chemistry)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new RenderPreview)
{
	preview_->connect(boost::bind(&InsetFormula::statusChanged, this));
	if (chemistry)
		mutate("chemistry");
}


InsetFormula::InsetFormula(InsetFormula const & other)
	: InsetFormulaBase(other),
	  par_(other.par_),
	  preview_(new RenderPreview)
{
	preview_->connect(boost::bind(&InsetFormula::statusChanged, this));
}


InsetFormula::InsetFormula(BufferView *)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new RenderPreview)
{
	preview_->connect(boost::bind(&InsetFormula::statusChanged, this));
}


InsetFormula::InsetFormula(string const & data)
	: par_(MathAtom(new MathHullInset)),
	  preview_(new RenderPreview)
{
	preview_->connect(boost::bind(&InsetFormula::statusChanged, this));
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
			OutputParams const & runparams) const
{
	WriteStream wi(os, runparams.moving_arg, true);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::plaintext(Buffer const &, ostream & os,
			OutputParams const &) const
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


int InsetFormula::linuxdoc(Buffer const & buf, ostream & os,
			   OutputParams const & runparams) const
{
	return docbook(buf, os, runparams);
}


int InsetFormula::docbook(Buffer const & buf, ostream & os,
			  OutputParams const & runparams) const
{
	MathMLStream ms(os);
	ms << MTag("equation");
	ms <<   MTag("alt");
	ms <<    "<[CDATA[";
	int res = plaintext(buf, ms.os(), runparams);
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


namespace {

bool editing_inset(InsetFormula const * inset)
{
	return mathcursor &&
		(const_cast<InsetFormulaBase const *>(mathcursor->formula()) ==
		 inset);
}

} // namespace anon


void InsetFormula::draw(PainterInfo & pi, int x, int y) const
{
	// The previews are drawn only when we're not editing the inset.
	bool const use_preview = !editing_inset(this)
		&& RenderPreview::activated()
		&& preview_->previewReady();

	int const w = dim_.wid;
	int const d = dim_.des;
	int const a = dim_.asc;
	int const h = a + d;

	if (use_preview) {
		// one pixel gap in front
		preview_->draw(pi, x + 1, y);
	} else {
		PainterInfo p(pi.base.bv);
		p.base.style = LM_ST_TEXT;
		p.base.font  = pi.base.font;
		p.base.font.setColor(LColor::math);
		if (lcolor.getX11Name(LColor::mathbg)
			    != lcolor.getX11Name(LColor::background))
			p.pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

		if (editing_inset(this)) {
			mathcursor->drawSelection(pi);
			//p.pain.rectangle(x, y - a, w, h, LColor::mathframe);
		}

		par_->draw(p, x, y);
	}

	xo_ = x;
	yo_ = y;

	top_x = x;
	top_baseline = y;

}


void InsetFormula::getLabelList(Buffer const & buffer,
				vector<string> & res) const
{
	par()->getLabelList(buffer, res);
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
	bool const use_preview = !editing_inset(this)
		&& RenderPreview::activated()
		&& preview_->previewReady();

	if (use_preview) {
		preview_->metrics(m, dim);
		// insert a one pixel gap in front of the formula
		dim.wid += 1;
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

	dim_ = dim;
}


void InsetFormula::mutate(string const & type)
{
	par_.nucleus()->mutate(type);
}


//
// preview stuff
//

void InsetFormula::statusChanged() const
{
	LyX::cref().updateInset(this);
}


namespace {

string const latex_string(InsetFormula const & inset, Buffer const &)
{
	ostringstream os;
	WriteStream wi(os, false, false);
	inset.par()->write(wi);
	return os.str();
}

} // namespace anon


void InsetFormula::addPreview(lyx::graphics::PreviewLoader & ploader) const
{
	string const snippet = latex_string(*this, ploader.buffer());
	preview_->addPreview(snippet, ploader);
}


void InsetFormula::generatePreview(Buffer const & buffer) const
{
	string const snippet = latex_string(*this, buffer);
	preview_->addPreview(snippet, buffer);
	preview_->startLoading(buffer);
}
