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

extern char const * latex_mathenv[];
extern MathCursor * mathcursor;


// quite a hack i know. Should be done with return values...
int number_of_newlines = 0;


InsetFormula::InsetFormula()
	: InsetFormulaBase(new MathMatrixInset)
{}


InsetFormula::InsetFormula(MathInsetTypes t)
	: InsetFormulaBase(new MathMatrixInset(t))
{}


InsetFormula::InsetFormula(string const & s)
	: InsetFormulaBase(mathed_parse(s))
{
	Metrics();
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
	par()->Write(os, fragile);
	return 1;
}


int InsetFormula::ascii(ostream & os, int) const
{
	par()->Write(os, false);
	return 1;
}


int InsetFormula::linuxdoc(ostream & os) const
{
	return ascii(os, 0);
}


int InsetFormula::docBook(ostream & os) const
{
	return ascii(os, 0);
}


void InsetFormula::read(LyXLex & lex)
{
	par(mathed_parse(lex));
	Metrics();
}


void InsetFormula::draw(BufferView * bv, LyXFont const &,
			int y, float & xx, bool) const
{
	int x = int(xx) - 1;
	y -= 2;

	MathInset::workwidth = bv->workWidth();
	Painter & pain = bv->painter();

	Metrics();
	int w = par()->width();
	int h = par()->height();
	int a = par()->ascent();
	pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

	if (mathcursor && mathcursor->formula() == this) {
		mathcursor->drawSelection(pain);
		pain.rectangle(x, y - a, w, h, LColor::mathframe);
	}

	par()->draw(pain, x, y);
	xx += par()->width();

	setCursorVisible(false);
}


void InsetFormula::Metrics() const 
{
	const_cast<MathInset *>(par_)->Metrics(display() ? LM_ST_DISPLAY : LM_ST_TEXT);
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
			int x;
			int y;
			mathcursor->GetPos(x, y);
			mathcursor->breakLine();
			mathcursor->normalize();
			updateLocal(bv, true);
			break;

		case LFUN_MATH_NUMBER:
		{
			//lyxerr << "toggling all numbers\n";
			if (display()) {
				bv->lockedInsetStoreUndo(Undo::INSERT);
				bool old = par()->numberedType();
				for (int row = 0; row < par()->nrows(); ++row)
					par()->numbered(row, !old);
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
				bool old = par()->numbered(row);
				bv->owner()->message(old ? _("No number") : _("Number"));
				par()->numbered(row, !old);
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_INSERT_LABEL:
		{
			bv->lockedInsetStoreUndo(Undo::INSERT);

			int row = mathcursor->row();
			string old_label = par()->label(row);
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
				par()->numbered(row, true);
			}

			if (!new_label.empty() && bv->ChangeRefsIfUnique(old_label, new_label))
				bv->redraw();

			par()->label(row, new_label);

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
			mathcursor->GetPos(x, y);
			par()->mutate(arg);
			mathcursor->SetPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_DISPLAY:
		{
			int x;
			int y;
			mathcursor->GetPos(x, y);
			if (par()->GetType() == LM_OT_SIMPLE)
				par()->mutate(LM_OT_EQUATION);
			else
				par()->mutate(LM_OT_SIMPLE);
			mathcursor->SetPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}
		
		case LFUN_PASTESELECTION:
		{
			string const clip = bv->getClipboard();
  		if (!clip.empty())
				par(mathed_parse(clip));
			break;
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
	par()->WriteNormal(os); 
	string code = os.str().c_str();
	string script = "lyx2" + arg + " '" + code + "' " + outfile;
	lyxerr << "calling: " << script << endl;
	Systemcalls cmd(Systemcalls::System, script, 0);

	ifstream is(outfile.c_str());
	par(mathed_parse(is));
	Metrics();
}

bool InsetFormula::display() const
{
	return par_->GetType() != LM_OT_SIMPLE;
}


MathMatrixInset * InsetFormula::par() const
{
	return static_cast<MathMatrixInset *>(par_);
}

void InsetFormula::par(MathInset * p)
{ 
	delete par_;
	par_ = p ? p : new MathMatrixInset;
}


Inset::Code InsetFormula::lyxCode() const
{
	return Inset::MATH_CODE;
}


void InsetFormula::validate(LaTeXFeatures & features) const
{
	par()->Validate(features);
}

bool InsetFormula::insetAllowed(Inset::Code code) const
{
	return code == Inset::LABEL_CODE; 
}


int InsetFormula::ascent(BufferView *, LyXFont const &) const
{
	return par()->ascent() + 1;
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	return par()->descent() + 1;
}


int InsetFormula::width(BufferView *, LyXFont const &) const
{
	Metrics();
	return par()->width();
}
