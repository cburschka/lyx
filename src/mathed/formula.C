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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "formula.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_charinset.h"
#include "math_arrayinset.h"
#include "math_metricsinfo.h"
#include "math_deliminset.h"
#include "math_hullinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "textpainter.h"

#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"
#include "funcrequest.h"

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

#include <fstream>


using std::ostream;
using std::ifstream;
using std::istream;
using std::pair;
using std::endl;
using std::vector;
using std::getline;


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



InsetFormula::InsetFormula()
	: par_(MathAtom(new MathHullInset)),
	  preview_(new PreviewImpl(*this))
{}


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
	os << par_->fileInsetLabel() << " ";
	par_->write(wi);
}


int InsetFormula::latex(Buffer const *, ostream & os, bool fragile, bool) const
{
	WriteStream wi(os, fragile, true);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::ascii(Buffer const *, ostream & os, int) const
{
	if (display()) {
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
	ms <<    par_.nucleus();
	ms <<   ETag("math");
	ms << ETag("equation");
	return ms.line() + res;
}


void InsetFormula::read(Buffer const *, LyXLex & lex)
{
	mathed_parse_normal(par_, lex);
	metrics();
}


//ostream & operator<<(ostream & os, LyXCursor const & c)
//{
//	os << '[' << c.x() << ' ' << c.y() << ' ' << c.pos() << ']';
//	return os;
//}


void InsetFormula::draw(BufferView * bv, LyXFont const & font,
			int y, float & xx, bool) const
{
	// This initiates the loading of the preview, so should come
	// before the metrics are computed.
	preview_->setView(bv);
	bool const use_preview = preview_->previewReady();

	int const x = int(xx);
	int const w = width(bv, font);
	int const d = descent(bv, font);
	int const a = ascent(bv, font);
	int const h = a + d;

	MathPainterInfo pi(bv->painter());

	if (use_preview) {
		pi.pain.image(x, y - a, w, h,
			      *(preview_->pimage()->image(*this, *bv)));
	} else {
		//pi.base.style = display() ? LM_ST_DISPLAY : LM_ST_TEXT;
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

		par_->draw(pi, x + 1, y);
	}

	xx += w;
	xo_ = x;
	yo_ = y;

	setCursorVisible(false);
}


vector<string> const InsetFormula::getLabelList() const
{
	vector<string> res;
	par()->getLabelList(res);
	return res;
}


UpdatableInset::RESULT
InsetFormula::localDispatch(BufferView * bv, FuncRequest const & ev)
{
	RESULT result = DISPATCHED;

	switch (ev.action) {

		case LFUN_BREAKLINE:
			bv->lockedInsetStoreUndo(Undo::INSERT);
			mathcursor->breakLine();
			mathcursor->normalize();
			updateLocal(bv, true);
			break;

		case LFUN_MATH_NUMBER:
		{
			if (!hull())
				break;
			//lyxerr << "toggling all numbers\n";
			if (display()) {
				bv->lockedInsetStoreUndo(Undo::INSERT);
				bool old = par()->numberedType();
				for (MathInset::row_type row = 0; row < par_->nrows(); ++row)
					hull()->numbered(row, !old);
				bv->owner()->message(old ? _("No number") : _("Number"));
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_MATH_NONUMBER:
		{
			//lyxerr << "toggling line number\n";
			if (display()) {
				bv->lockedInsetStoreUndo(Undo::INSERT);
				MathCursor::row_type row = mathcursor->hullRow();
				bool old = hull()->numbered(row);
				bv->owner()->message(old ? _("No number") : _("Number"));
				hull()->numbered(row, !old);
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_INSERT_LABEL:
		{
			if (!hull())
				break;

			bv->lockedInsetStoreUndo(Undo::INSERT);

			MathCursor::row_type row = mathcursor->hullRow();
			string old_label = hull()->label(row);
			string new_label = ev.argument;

			if (new_label.empty()) {
				string const default_label =
					(lyxrc.label_init_length >= 0) ? "eq:" : "";
				pair<bool, string> const res = old_label.empty()
					? Alert::askForText(_("Enter new label to insert:"), default_label)
					: Alert::askForText(_("Enter label:"), old_label);
				if (!res.first)
					break;
				new_label = trim(res.second);
			}

			//if (new_label == old_label)
			//	break;  // Nothing to do

			if (!new_label.empty()) {
				lyxerr << "setting label to '" << new_label << "'\n";
				hull()->numbered(row, true);
			}

#warning FIXME: please check you really mean repaint() ... is it needed,
#warning and if so, should it be update() instead ?
			if (!new_label.empty() && bv->ChangeRefsIfUnique(old_label, new_label))
				bv->repaint();

			hull()->label(row, new_label);

			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_MUTATE:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			int x;
			int y;
			mathcursor->getPos(x, y);
			mutate(ev.argument);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_EXTERN:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			if (mathcursor)
				mathcursor->handleExtern(ev.argument);
			// re-compute inset dimension
			metrics(bv);
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_DISPLAY:
		{
			int x = 0;
			int y = 0;
			mathcursor->getPos(x, y);
			if (hullType() == "simple")
				mutate("equation");
			else
				mutate("simple");
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_PASTESELECTION:
		{
			string const clip = bv->getClipboard();
			if (!clip.empty())
				mathed_parse_normal(par_, clip);
			break;
		}

		default:
			result = InsetFormulaBase::localDispatch(bv, ev);
	}

	return result;
}


bool InsetFormula::display() const
{
	return hullType() != "simple" && hullType() != "none";
}


MathHullInset * InsetFormula::hull() const
{
	lyx::Assert(par_->asHullInset());
	return par_->asHullInset();
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
		(code == Inset::LABEL_CODE && display())
		|| code == Inset::REF_CODE
		|| code == Inset::ERT_CODE;
}


int InsetFormula::ascent(BufferView *, LyXFont const &) const
{
	return preview_->previewReady() ?
		preview_->pimage()->ascent() : 1 + par_->ascent();
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	if (!preview_->previewReady())
		return 1 + par_->descent();

	int const descent = preview_->pimage()->descent();
	return display() ? descent + 12 : descent;
}


int InsetFormula::width(BufferView * bv, LyXFont const & font) const
{
	metrics(bv, font);
	return preview_->previewReady() ?
		preview_->pimage()->width() : par_->width();
}


string InsetFormula::hullType() const
{
	return par()->getType();
}


void InsetFormula::mutate(string const & type)
{
	par()->mutate(type);
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
	return ls.str().c_str();
}
