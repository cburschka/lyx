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
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "support/filetools.h" // LibFileSearch
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


namespace {

	void stripFromLastEqualSign(MathArray & ar)
	{
		// find position of last '=' in the array
		MathArray::size_type pos = ar.size();
		for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
			if ((*it)->getChar() == '=')
				pos = it - ar.begin();

		// delete everything behind this position
		ar.erase(pos, ar.size());
	}


	MathArray pipeThroughExtern(string const & arg, MathArray const & ar)
	{
		string lang;
		string extra;
		istringstream iss(arg.c_str());
		iss >> lang >> extra;
		if (extra.empty())
			extra = "noextra";	

		// create normalized expression
		string outfile = lyx::tempName(string(), "mathextern");
		ostringstream os;
		os << "[" << extra << ' ';
		ar.writeNormal(os); 
		os << "]";
		string code = os.str().c_str();

		// run external sript
		string file = LibFileSearch("mathed", "extern_" + lang);
		if (file.empty()) {
			lyxerr << "converter to '" << lang << "' not found\n";
			return MathArray();
		}
		string script = file + " '" + code + "' " + outfile;
		lyxerr << "calling: " << script << endl;
		Systemcalls cmd(Systemcalls::System, script, 0);

		// append result
		MathArray res;
		mathed_parse_cell(res, GetFileContents(outfile));
		return res;
	}

}


InsetFormula::InsetFormula()
	: par_(MathAtom(new MathMatrixInset))
{}


InsetFormula::InsetFormula(MathInsetTypes t)
	: par_(MathAtom(new MathMatrixInset(t)))
{}


InsetFormula::InsetFormula(string const & s) 
{
	if (s.size()) {
		bool res = mathed_parse_normal(par_, s);

		if (!res)
			res = mathed_parse_normal(par_, "$" + s + "$");

		if (!res) {
			lyxerr << "cannot interpret '" << s << "' as math\n";
			par_ = MathAtom(new MathMatrixInset(LM_OT_SIMPLE));
		}
	}
	metrics();
}


Inset * InsetFormula::clone(Buffer const &, bool) const
{
	return new InsetFormula(*this);
}


void InsetFormula::write(Buffer const * buf, ostream & os) const
{
	os << "Formula ";
	latex(buf, os, false, false);
}


int InsetFormula::latex(Buffer const * buf, ostream & os, bool fragil, bool)
	const
{
	MathWriteInfo wi(buf, os, fragil);
	par_->write(wi);
	return 1;
}


int InsetFormula::ascii(Buffer const * buf, ostream & os, int) const
{
	MathWriteInfo wi(buf, os, false);
	par_->write(wi);
	return 1;
}


int InsetFormula::linuxdoc(Buffer const * buf, ostream & os) const
{
	return ascii(buf, os, 0);
}


int InsetFormula::docbook(Buffer const * buf, ostream & os) const
{
	return ascii(buf, os, 0);
}


void InsetFormula::read(Buffer const *, LyXLex & lex)
{
	mathed_parse_normal(par_, lex);
	metrics();
}


void InsetFormula::draw(BufferView * bv, LyXFont const & font,
			int y, float & xx, bool) const
{
	int x = int(xx) - 1;
	y -= 2;

	Painter & pain = bv->painter();

	metrics(bv, font);
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
	xo_ = x;
	yo_ = y;

	setCursorVisible(false);
}


vector<string> const InsetFormula::getLabelList() const
{
	return mat()->getLabelList();
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
				bool old = mat()->numberedType();
				for (MathInset::row_type row = 0; row < par_->nrows(); ++row)
					mat()->numbered(row, !old);
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
				MathCursor::row_type row = mathcursor->row();
				bool old = mat()->numbered(row);
				bv->owner()->message(old ? _("No number") : _("Number"));
				mat()->numbered(row, !old);
				updateLocal(bv, true);
			}
			break;
		}

		case LFUN_INSERT_LABEL:
		{
			bv->lockedInsetStoreUndo(Undo::INSERT);

			MathCursor::row_type row = mathcursor->row();
			string old_label = mat()->label(row);
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
				mat()->numbered(row, true);
			}

			if (!new_label.empty() && bv->ChangeRefsIfUnique(old_label, new_label))
				bv->redraw();

			mat()->label(row, new_label);

			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_MUTATE:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			int x;
			int y;
			mathcursor->getPos(x, y);
			mat()->mutate(arg);
			mathcursor->setPos(x, y);
			mathcursor->normalize();
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_EXTERN:
		{
			bv->lockedInsetStoreUndo(Undo::EDIT);
			handleExtern(arg);
			// re-compute inset dimension
			metrics(bv);
			updateLocal(bv, true);
			break;
		}

		case LFUN_MATH_DISPLAY:
		{
			int x;
			int y;
			mathcursor->getPos(x, y);
			if (mat()->getType() == LM_OT_SIMPLE)
				mat()->mutate(LM_OT_EQUATION);
			else
				mat()->mutate(LM_OT_SIMPLE);
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

		case LFUN_MATH_COLUMN_INSERT:
		{
			if (mat()->getType() == LM_OT_ALIGN)
				mat()->mutate(LM_OT_ALIGNAT);
			mat()->addCol(mat()->ncols());
			mathcursor->normalize();
			updateLocal(bv, true);
		}

		default:
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}

	return result;
}


void InsetFormula::handleExtern(const string & arg)
{
	// where are we?
	if (!mathcursor)
		return; 

	bool selected = mathcursor->selection();

	MathArray ar;
	if (selected) {
		mathcursor->selGet(ar);
		lyxerr << "use selection: " << ar << "\n";
	} else {
		mathcursor->end();
		ar = mathcursor->cursor().cell();
		stripFromLastEqualSign(ar);
		mathcursor->insert(MathAtom(new MathCharInset('=', LM_TC_VAR)));
		lyxerr << "use whole cell: " << ar << "\n";
	}

	mathcursor->insert(pipeThroughExtern(arg, ar));
}


bool InsetFormula::display() const
{
	return mat()->getType() != LM_OT_SIMPLE;
}


MathMatrixInset const * InsetFormula::mat() const
{
	lyx::Assert(par_->asMatrixInset());
	return par_->asMatrixInset();
}


MathMatrixInset * InsetFormula::mat()
{
	lyx::Assert(par_->asMatrixInset());
	return par_->asMatrixInset();
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
	return code == Inset::LABEL_CODE && display(); 
}


int InsetFormula::ascent(BufferView *, LyXFont const &) const
{
	return par_->ascent() + 2;
}


int InsetFormula::descent(BufferView *, LyXFont const &) const
{
	return par_->descent() - 2;
}


int InsetFormula::width(BufferView * bv, LyXFont const & font) const
{
	metrics(bv, font);
	return par_->width();
}


MathInsetTypes InsetFormula::getType() const
{
	return mat()->getType();
}
