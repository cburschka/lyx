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
#include "support/lyxlib.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "support/filetools.h" // LibFileSearch
#include "LyXView.h"
#include "Painter.h"
#include "lyxrc.h"
#include "math_hullinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"

using std::ostream;
using std::ifstream;
using std::istream;
using std::pair;
using std::endl;
using std::vector;
using std::getline;


namespace {

	string captureOutput(string const & cmd, string const & data)
	{
		string outfile = lyx::tempName(string(), "mathextern");
		string full =  "echo '" + data + "' | (" + cmd + ") > " + outfile;
		lyxerr << "calling: " << full << "\n";
		Systemcalls dummy(Systemcalls::System, full, 0);
		string out = GetFileContents(outfile);
		lyx::unlink(outfile);
		lyxerr << "result: '" << out << "'\n";
		return out;
	}


	MathArray pipeThroughMaple(string const & extra, MathArray const & ar)
	{
		string header = "readlib(latex):\n";

		// remove the \\it for variable names
		//"#`latex/csname_font` := `\\it `:"
		header +=
			"`latex/csname_font` := ``:\n";

		// export matrices in (...) instead of [...]
		header +=
			"`latex/latex/matrix` := "
				"subs(`[`=`(`, `]`=`)`,"
					"eval(`latex/latex/matrix`)):\n";

		// replace \\cdots with proper '*'
		header +=
			"`latex/latex/*` := "
				"subs(`\\,`=`\\cdot `,"
					"eval(`latex/latex/*`)):\n";

		// remove spurious \\noalign{\\medskip} in matrix output
		header += 
			"`latex/latex/matrix`:= "
				"subs(`\\\\\\\\\\\\noalign{\\\\medskip}` = `\\\\\\\\`,"
					"eval(`latex/latex/matrix`)):\n";

		//"#`latex/latex/symbol` "
		//	" := subs((\\'_\\' = \\'`\\_`\\',eval(`latex/latex/symbol`)): ";

		string trailer = "quit;";
		ostringstream os;
		MapleStream ms(os);
		ms << ar;
		string expr = os.str().c_str();

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			// try to fix missing '*' the hard way by using mint
			//
			// ... > echo "1A;" | mint -i 1 -S -s -q
			// on line     1: 1A;
			//                 ^ syntax error - 
			//                   Probably missing an operator such as * p
			//
			lyxerr << "checking expr: '" << expr << "'\n";
			string out = captureOutput("mint -i 1 -S -s -q -q", expr + ";");
			if (out.empty())
				break; // expression syntax is ok
			istringstream is(out.c_str());
			string line;
			getline(is, line);
			if (line.find("on line") != 0)
				break; // error message not identified
			getline(is, line);
			string::size_type pos = line.find('^');
			if (pos == string::npos || pos < 15)
				break; // caret position not found
			pos -= 15; // skip the "on line ..." part
			if (expr[pos] == '*' || (pos > 0 && expr[pos - 1] == '*'))
				break; // two '*' in a row are definitely bad
			expr.insert(pos,  "*");
		}

		string full = "latex(" +  extra + '(' + expr + "));";
		string out = captureOutput("maple -q", header + full + trailer);

		// change \_ into _

		//
		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}
		
	
	MathArray pipeThroughOctave(string const &, MathArray const & ar)
	{
		ostringstream os;
		OctaveStream vs(os);
		vs << ar;
		string expr = os.str().c_str();
		string out;

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			//
			// try to fix missing '*' the hard way 
			// parse error:
			// >>> ([[1 2 3 ];[2 3 1 ];[3 1 2 ]])([[1 2 3 ];[2 3 1 ];[3 1 2 ]])
			//                                   ^
			//
			lyxerr << "checking expr: '" << expr << "'\n";
 			out = captureOutput("octave -q 2>&1", expr);
			lyxerr << "checking out: '" << out << "'\n";

			// leave loop if expression syntax is probably ok
			if (out.find("parse error:") == string::npos)
				break;

			// search line with single caret
			istringstream is(out.c_str());
			string line;
			while (is) {
				getline(is, line);
				lyxerr << "skipping line: '" << line << "'\n";
				if (line.find(">>> ") != string::npos)
					break;
			}

			// found line with error, next line is the one with caret
			getline(is, line);
			string::size_type pos = line.find('^');
			lyxerr << "caret line: '" << line << "'\n";
			lyxerr << "found caret at pos: '" << pos << "'\n";
			if (pos == string::npos || pos < 4)
				break; // caret position not found
			pos -= 4; // skip the ">>> " part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos,  "*");
		}

		if (out.size() < 6)
			return MathArray();

		// remove 'ans = '
		out = out.substr(6);

		// parse output as matrix or single number
		MathAtom at(new MathArrayInset("array", out));
		MathArrayInset const * mat = at.nucleus()->asArrayInset();
		MathArray res;
		if (mat->ncols() == 1 && mat->nrows() == 1)
			res.push_back(mat->cell(0));
		else {
			res.push_back(MathAtom(new MathDelimInset("(", ")")));
			res.back()->cell(0).push_back(at);
		}
		return res;
	}


	MathArray pipeThroughExtern(string const & lang, string const & extra,
		MathArray const & ar)
	{
		if (lang == "octave")
			return pipeThroughOctave(extra, ar);

		if (lang == "maple")
			return pipeThroughMaple(extra, ar);

		// create normalized expression
		ostringstream os;
		NormalStream ns(os);
		os << "[" << extra << ' ';
		ns << ar;
		os << "]";
		string data = os.str().c_str();

		// search external script
		string file = LibFileSearch("mathed", "extern_" + lang);
		if (file.empty()) {
			lyxerr << "converter to '" << lang << "' not found\n";
			return MathArray();
		}
		
		// run external sript
		string out = captureOutput(file, data);
		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}

}


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


void InsetFormula::write(Buffer const * buf, ostream & os) const
{
	os << "Formula ";
	latex(buf, os, false, false);
}


int InsetFormula::latex(Buffer const *, ostream & os, bool fragil, bool) const
{
	WriteStream wi(os, fragil);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::ascii(Buffer const *, ostream & os, int) const
{
	WriteStream wi(os, false);
	par_->write(wi);
	return wi.line();
}


int InsetFormula::linuxdoc(Buffer const * buf, ostream & os) const
{
	return docbook(buf, os);
}


int InsetFormula::docbook(Buffer const * buf, ostream & os) const
{
	MathMLStream ms(os);
	ms << MTag("equation") << MTag("alt");
	int res = ascii(buf, ms.os(), 0);
	ms << ETag("alt") << MTag("math");
	ms << par_.nucleus();
	ms << ETag("math") << ETag("equation");
	return ms.line() + res;
}


void InsetFormula::read(Buffer const *, LyXLex & lex)
{
	mathed_parse_normal(par_, lex);
	metrics();
}


//std::ostream & operator<<(std::ostream & os, LyXCursor const & c)
//{
//	os << '[' << c.x() << ' ' << c.y() << ' ' << c.pos() << ']';
//	return os;
//}


void InsetFormula::draw(BufferView * bv, LyXFont const & font,
			int y, float & xx, bool) const
{
	int x = int(xx);

	Painter & pain = bv->painter();

	metrics(bv, font);
	int w = par_->width();
	int h = par_->height();
	int a = par_->ascent();

	if (lcolor.getX11Name(LColor::mathbg)!=lcolor.getX11Name(LColor::background))
		pain.fillRectangle(x, y - a, w, h, LColor::mathbg);

	if (mathcursor &&
			const_cast<InsetFormulaBase const *>(mathcursor->formula()) == this)
	{
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

			if (!new_label.empty() && bv->ChangeRefsIfUnique(old_label, new_label))
				bv->redraw();

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
			handleExtern(arg);
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
			if (hull()->getType() == LM_OT_SIMPLE)
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


bool needEqnArray(string const & extra)
{
	return extra == "dsolve";
}


void InsetFormula::handleExtern(const string & arg)
{
	// where are we?
	if (!mathcursor)
		return; 

	string lang;
	string extra;
	istringstream iss(arg.c_str());
	iss >> lang >> extra;
	if (extra.empty())
		extra = "noextra";	

	bool selected = mathcursor->selection();

	MathArray ar;
	if (needEqnArray(extra)) {
		mathcursor->last();
		//mathcursor->readLine(ar);
		mathcursor->breakLine();
	} else if (selected) {
		mathcursor->selGet(ar);
		//lyxerr << "use selection: " << ar << "\n";
	} else {
		mathcursor->last();
		mathcursor->stripFromLastEqualSign();
		ar = mathcursor->cursor().cell();
		mathcursor->insert(MathAtom(new MathCharInset('=', LM_TC_VAR)));
		//lyxerr << "use whole cell: " << ar << "\n";
	}

	mathcursor->insert(pipeThroughExtern(lang, extra, ar));
}


bool InsetFormula::display() const
{
	return hull()->getType() != LM_OT_SIMPLE;
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
	return par_->width();
}


MathInsetTypes InsetFormula::getType() const
{
	return hull()->getType();
}
