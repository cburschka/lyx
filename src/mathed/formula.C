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
#include <fstream>

#include "formula.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "support/LOstream.h"
#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "Painter.h"
#include "lyxrc.h"
#include "math_matrixinset.h"
#include "mathed/support.h"

using std::ostream;
using std::ifstream;
using std::istream;
using std::pair;
using std::endl;
using std::vector;


InsetFormula::InsetFormula()
	: par_(new MathMatrixInset)
{}


InsetFormula::InsetFormula(const InsetFormula & f)
	: InsetFormulaBase(f), par_(static_cast<MathMatrixInset *>(f.par_->clone()))
{}


InsetFormula::InsetFormula(MathInsetTypes t)
	: par_(new MathMatrixInset(t))
{}


InsetFormula::InsetFormula(string const & s) 
	: par_(mathed_parse_normal(s))
{
	if (!par_)
		par_ = mathed_parse_normal("$" + s + "$");

	if (!par_) {
		lyxerr << "cannot interpret '" << s << "' as math\n";
		par_ = new MathMatrixInset(LM_OT_SIMPLE);
	}
	metrics();
}


InsetFormula::~InsetFormula()
{
	delete par_;
}


Inset * InsetFormula::clone(Buffer const &, bool) const
{
	return new InsetFormula(*this);
}


void InsetFormula::write(ostream & os) const
{
	os << "Formula ";
	latex(os, false, false);
}


int InsetFormula::latex(ostream & os, bool fragile, bool) const
{
	par_->write(os, fragile);
	return 1;
}


int InsetFormula::ascii(ostream & os, int) const
{
	par_->write(os, false);
	return 1;
}


int InsetFormula::linuxdoc(ostream & os) const
{
	return ascii(os, 0);
}


int InsetFormula::docbook(ostream & os) const
{
	return ascii(os, 0);
}


void InsetFormula::read(LyXLex & lex)
{
	par(mathed_parse_normal(lex));
	metrics();
}


void InsetFormula::draw(BufferView * bv, LyXFont const &,
			int y, float & xx, bool) const
{
	int x = int(xx) - 1;
	y -= 2;

	MathInset::workwidth = bv->workWidth();
	Painter & pain = bv->painter();

	metrics();
	int w = par_->width();
	int h = par_->height();
	int a = par_->ascent();
	pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

	if (mathcursor && mathcursor->formula() == this) {
		mathcursor->drawSelection(pain);
		pain.rectangle(x, y - a, w, h, LColor::mathframe);
	}

	par_->draw(pain, x, y);
	xx += par_->width();

	setCursorVisible(false);
}


void InsetFormula::metrics() const 
{
	par_->metrics(display() ? LM_ST_DISPLAY : LM_ST_TEXT);
}


vector<string> const InsetFormula::getLabelList() const
{
	return par_->getLabelList();
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
				bool old = par_->numberedType();
				for (int row = 0; row < par_->nrows(); ++row)
					par_->numbered(row, !old);
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
				int row = mathcursor->row();
				bool old = par_->numbered(row);
				bv->owner()->message(old ? _("No number") : _("Number"));
				par_->numbered(row, !old);
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_INSERT_LABEL:
		{
			bv->lockedInsetStoreUndo(Undo::INSERT);

			int row = mathcursor->row();
			string old_label = par_->label(row);
			string new_label = arg;

			if (new_label.empty()) {
				string const default_label =
					(lyxrc.label_init_length >= 0) ? "eq:" : "";
				pair<bool, string> const res = old_label.empty()
					? askForText(_("Enter new label to insert:"), default_label)
					: askForText(_("Enter label:"), old_label);
				
				lyxerr << "res: " << res.first << " - '" << res.second << "'\n";
				if (!res.first)
					break;
				new_label = frontStrip(strip(res.second));
			}

			//if (new_label == old_label)
			//	break;  // Nothing to do

			if (!new_label.empty()) {
				lyxerr << "setting label to '" << new_label << "'\n";
				par_->numbered(row, true);
			}

			if (!new_label.empty() && bv->ChangeRefsIfUnique(old_label, new_label))
				bv->redraw();

			par_->label(row, new_label);

			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_EXTERN:
			bv->lockedInsetStoreUndo(Undo::EDIT);
			handleExtern(arg, bv);
			updateLocal(bv, true);
			break;

		case LFUN_MATH_MUTATE:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			int x;
			int y;
			mathcursor->getPos(x, y);
			par_->mutate(arg);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_DISPLAY:
		{
			int x;
			int y;
			mathcursor->getPos(x, y);
			if (par_->getType() == LM_OT_SIMPLE)
				par_->mutate(LM_OT_EQUATION);
			else
				par_->mutate(LM_OT_SIMPLE);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}
		
		case LFUN_PASTESELECTION:
		{
			string const clip = bv->getClipboard();
  		if (!clip.empty())
				par(mathed_parse_normal(clip));
			break;
		}

		case LFUN_MATH_COLUMN_INSERT:
		{
			if (par_->getType() == LM_OT_ALIGN)
				par_->mutate(LM_OT_ALIGNAT);
			par_->addCol(par_->ncols());
			mathcursor->normalize();
			updateLocal(bv, true);
		}

		default:
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}

	return result;
}


void InsetFormula::handleExtern(const string & arg, BufferView *)
{
	//string outfile = lyx::tempName("maple.out");
	string outfile = "/tmp/lyx2" + arg + ".out";
	ostringstream os;
	par_->writeNormal(os); 
	string code = os.str().c_str();
	string script = "lyx2" + arg + " '" + code + "' " + outfile;
	lyxerr << "calling: " << script << endl;
	Systemcalls cmd(Systemcalls::System, script, 0);

	ifstream is(outfile.c_str());
	par(mathed_parse_normal(is));
	metrics();
}

bool InsetFormula::display() const
{
	return par_->getType() != LM_OT_SIMPLE;
}


MathInset const * InsetFormula::par() const
{
	return par_;
}


void InsetFormula::par(MathMatrixInset * p)
{ 
	delete par_;
	par_ = p ? static_cast<MathMatrixInset *>(p) : new MathMatrixInset;
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
	return code == Inset::LABEL_CODE; 
}


int InsetFormula::ascent(BufferView *, LyXFont const &) const
{
	return par_->ascent() + 1;
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	return par_->descent() + 1;
}


int InsetFormula::width(BufferView *, LyXFont const &) const
{
	metrics();
	return par_->width();
}


MathInsetTypes InsetFormula::getType() const
{
	return par_->getType();
}
