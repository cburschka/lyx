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
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_charinset.h"
#include "math_arrayinset.h"
#include "math_deliminset.h"
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "frontends/Alert.h"
#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/filetools.h" // LibFileSearch
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "lyxrc.h"
#include "math_hullinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "textpainter.h"
#include "preview.h"

#include "graphics/GraphicsCacheItem.h"

using std::ostream;
using std::ifstream;
using std::istream;
using std::pair;
using std::endl;
using std::vector;
using std::getline;



InsetFormula::InsetFormula()
	: par_(MathAtom(new MathHullInset))
{}


InsetFormula::InsetFormula(MathInsetTypes t)
	: par_(MathAtom(new MathHullInset(t)))
{}


InsetFormula::InsetFormula(string const & s)
{
	if (s.size()) {
		bool res = mathed_parse_normal(par_, s);

		if (!res)
			res = mathed_parse_normal(par_, "$" + s + "$");

		if (!res) {
			lyxerr << "cannot interpret '" << s << "' as math\n";
			par_ = MathAtom(new MathHullInset(LM_OT_SIMPLE));
		}
	}
	metrics();
}


Inset * InsetFormula::clone(Buffer const &, bool) const
{
	return new InsetFormula(*this);
}


void InsetFormula::write(Buffer const *, ostream & os) const
{
	os << "Formula ";
	WriteStream wi(os, false, false);
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
#if 0
	TextMetricsInfo mi;
	par()->metricsT(mi);
	TextPainter tpain(par()->width(), par()->height());
	par()->drawT(tpain, 0, par()->ascent());
	tpain.show(os);
	// reset metrics cache to "real" values
	metrics();
	return tpain.textheight();
#else
	WriteStream wi(os, false, true);
	par_->write(wi);
	return wi.line();
#endif
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
	metrics(bv, font);

	int const x = int(xx);
	int const w = par_->width();
	int const h = par_->height();
	int const a = par_->ascent();

	MathPainterInfo pi(bv->painter());
	pi.base.style = display() ? LM_ST_DISPLAY : LM_ST_TEXT;
	pi.base.font  = font;
	pi.base.font.setColor(LColor::math);

	if (lcolor.getX11Name(LColor::mathbg)!=lcolor.getX11Name(LColor::background))
		pi.pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

	if (mathcursor &&
			const_cast<InsetFormulaBase const *>(mathcursor->formula()) == this)
	{
		mathcursor->drawSelection(pi);
		pi.pain.rectangle(x, y - a, w, h, LColor::mathframe);
	}

	par_->draw(pi, x, y);

	// preview stuff
	if (lyxrc.preview) {
		ostringstream os;
		WriteStream wi(os, false, false);
		par_->write(wi);
		if (preview(os.str(), preview_)) {
			cerr << "image could be drawn\n";
			pi.pain.image(x + w + 2, y - a + 1, w - 2, h - 2, *(preview_->image()));
		} else {
			pi.pain.fillRectangle(x + w, y - a, w, h, LColor::white);
		}
		pi.pain.rectangle(x + w, y - a, w, h, LColor::mathframe);
		xx += w;
	}

	xx += w;
	xo_ = x;
	yo_ = y;

	setCursorVisible(false);
}


vector<string> const InsetFormula::getLabelList() const
{
	return hull()->getLabelList();
}


UpdatableInset::RESULT
InsetFormula::localDispatch(BufferView * bv, kb_action action,
	 string const & arg)
{
	RESULT result = DISPATCHED;

	switch (action) {

		case LFUN_BREAKLINE:
			bv->lockedInsetStoreUndo(Undo::INSERT);
			mathcursor->breakLine();
			mathcursor->normalize();
			updateLocal(bv, true);
			break;

		case LFUN_MATH_NUMBER:
		{
			//lyxerr << "toggling all numbers\n";
			if (display()) {
				bv->lockedInsetStoreUndo(Undo::INSERT);
				bool old = hull()->numberedType();
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
			bv->lockedInsetStoreUndo(Undo::INSERT);

			MathCursor::row_type row = mathcursor->hullRow();
			string old_label = hull()->label(row);
			string new_label = arg;

			if (new_label.empty()) {
				string const default_label =
					(lyxrc.label_init_length >= 0) ? "eq:" : "";
				pair<bool, string> const res = old_label.empty()
					? Alert::askForText(_("Enter new label to insert:"), default_label)
					: Alert::askForText(_("Enter label:"), old_label);
				if (!res.first)
					break;
				new_label = frontStrip(strip(res.second));
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
			hull()->mutate(arg);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_EXTERN:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			if (mathcursor)
				mathcursor->handleExtern(arg);
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
			if (getType() == LM_OT_SIMPLE)
				hull()->mutate(LM_OT_EQUATION);
			else
				hull()->mutate(LM_OT_SIMPLE);
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
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}

	return result;
}


bool InsetFormula::display() const
{
	return getType() != LM_OT_SIMPLE;
}


MathHullInset const * InsetFormula::hull() const
{
	lyx::Assert(par_->asHullInset());
	return par_->asHullInset();
}


MathHullInset * InsetFormula::hull()
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
	return par_->ascent() + 1;
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	return par_->descent() + 1;
}


int InsetFormula::width(BufferView * bv, LyXFont const & font) const
{
	metrics(bv, font);
	int const w = par_->width();
	// double the space for the preview if needed
	return lyxrc.preview ? 2 * w : w;
}


MathInsetTypes InsetFormula::getType() const
{
	return hull()->getType();
}
