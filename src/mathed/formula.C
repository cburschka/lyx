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
#include "math_hullinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "textpainter.h"

#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"

#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/lyxlib.h"
#include "support/systemcall.h"
#include "support/filetools.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "graphics/GraphicsImage.h"
#include "graphics/PreviewLoader.h"
#include "graphics/PreviewImage.h"
#include "graphics/Previews.h"

#include <fstream>
#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signals/connection.hpp>
#include <boost/utility.hpp>

using std::ostream;
using std::ifstream;
using std::istream;
using std::pair;
using std::endl;
using std::vector;
using std::getline;


struct InsetFormula::PreviewImpl : public boost::signals::trackable {
	///
	PreviewImpl(InsetFormula & p) : parent_(p), pimage_(0) {}

	///
	void generatePreview(grfx::PreviewLoader & previewer);

	/** This method is connected to the grfx::PreviewLoader::imageReady
	 *  signal.
	 */
	void previewReady(grfx::PreviewImage const &);

	/// A helper method.
	string const latexString() const;

	///
	bool usePreview() const;

	///
	InsetFormula & parent_;
	///
	mutable grfx::PreviewImage const * pimage_;
	///
	boost::signals::connection connection_;
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
	view_ = bv;
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
	updatePreview();
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
	bool const use_preview = preview_->usePreview();

	int const x = int(xx);
	int const w = width(bv, font);
	int const d = descent(bv, font);
	int const a = ascent(bv, font);
	int const h = a + d;

	MathPainterInfo pi(bv->painter());

	if (use_preview) {
		pi.pain.image(x, y - a, w, h,
			      *(preview_->pimage_->image(*this, *bv)));
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
	return par()->getLabelList();
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
			string new_label = arg;

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
			mutate(arg);
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
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}

	//updatePreview();

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
	return preview_->usePreview() ?
		preview_->pimage_->ascent() : 1 + par_->ascent();
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	if (!preview_->usePreview())
		return 1 + par_->descent();

	int const descent = preview_->pimage_->descent();
	return display() ? descent + 12 : descent;
}


int InsetFormula::width(BufferView * bv, LyXFont const & font) const
{
	metrics(bv, font);
	return preview_->usePreview() ?
		preview_->pimage_->width() : par_->width();
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

void InsetFormula::generatePreview(grfx::PreviewLoader & ploader) const
{
	// Do nothing if no preview is desired.
	if (!grfx::Previews::activated())
		return;

	preview_->generatePreview(ploader);
}


void InsetFormula::PreviewImpl::generatePreview(grfx::PreviewLoader & ploader)
{
	// Generate the LaTeX snippet.
	string const snippet = latexString();

	pimage_ = ploader.preview(snippet);
	if (pimage_)
		return;

	// If this is the first time of calling, connect to the
	// grfx::PreviewLoader signal that'll inform us when the preview image
	// is ready for loading.
	if (!connection_.connected()) {
		connection_ = ploader.connect(
			boost::bind(&PreviewImpl::previewReady, this, _1));
	}

	ploader.add(snippet);
}


bool InsetFormula::PreviewImpl::usePreview() const
{
	BufferView * view = parent_.view();

	if (!grfx::Previews::activated() ||
	    parent_.par_->asNestInset()->editing() ||
	    !view || !view->buffer())
		return false;

	// If the cached grfx::PreviewImage is invalid, update it.
	string const snippet = latexString();
	if (!pimage_ || snippet != pimage_->snippet()) {
		grfx::PreviewLoader & ploader =
			grfx::Previews::get().loader(view->buffer());
		pimage_ = ploader.preview(snippet);
	}

	if (!pimage_)
		return false;

	return pimage_->image(parent_, *view);
}


string const InsetFormula::PreviewImpl::latexString() const
{
	ostringstream ls;
	WriteStream wi(ls, false, false);
	parent_.par_->write(wi);
	return ls.str().c_str();
}


void InsetFormula::PreviewImpl::previewReady(grfx::PreviewImage const & pimage)
{
	// Check snippet against the Inset's current contents
	if (latexString() != pimage.snippet())
		return;

	pimage_ = &pimage;
	BufferView * view = parent_.view();
	if (view)
		view->updateInset(&parent_, false);
}
